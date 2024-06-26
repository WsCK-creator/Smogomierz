#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include "Plantower_PMS7003.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// Setings for WiFi connections
#define WIFI_SSID "SSID siecie" //Do zmiany
#define WIFI_PASSWORD "haslo" //Do zmiany

// Setings for MQTT connections
#define MQTT_HOST IPAddress(192, 168, 0, 107) //Do zmiany
#define MQTT_PORT 1883
#define MQTT_USER "user"
#define MQTT_PASSWORD "3Dprinter"

// Topics for data form sensors
#define MQTT_PUB_TEMP   "smog/temperature"
#define MQTT_PUB_HUM    "smog/humidity"
#define MQTT_PUB_PRES   "smog/pressure"
#define MQTT_PUB_HIGHT  "smog/hight"
#define MQTT_PUB_PM1    "smog/pm1"
#define MQTT_PUB_PM25   "smog/pm25"
#define MQTT_PUB_PM10   "smog/pm10"

// Stores curent data form sensors
float temperature = 0.0;
float humidity;
float pressure;
float hight;
float pm1;
float pm25;
float pm10;

bool mqttStatus = false;

unsigned long previousMillis = 0;   // Stores last time data was published
const long interval = 10000;        // Interval at which to publish sensor readings


// Objects declarations
Adafruit_BME280 bme;
SoftwareSerial PMS_Serial(13, 15);
Plantower_PMS7003 pms7003 = Plantower_PMS7003();

AsyncMqttClient mqttClient;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;

// Connects to WiFi
void connectToWifi() 
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
}

// Connects to MQTT
void connectToMqtt() 
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

// Event, Connects to MQTT on WiFi connection
void onWifiConnect(const WiFiEventStationModeGotIP& event) 
{
  Serial.println("Connected to Wi-Fi.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  connectToMqtt();
}

// Event, Reconnects to WiFi on WiFi disconnections
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) 
{
  Serial.println("Disconnected from Wi-Fi.");
  mqttClient.disconnect();
  mqttReconnectTimer.detach(); // Ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

// Event, Prints MQTT session status on connection
void onMqttConnect(bool sessionPresent) 
{
  mqttStatus = true;
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

// Event, Reconnects to MQTT on MQTT disconnections
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  mqttStatus = false;
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) mqttReconnectTimer.once(2, connectToMqtt);
}

// Event, Prints info packet info when MQTT packet have been successfully sent
void onMqttPublish(uint16_t packetId) 
{
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// Publish an MQTT message
void publishMQTTPacket(const char *topic, float data)
{
  uint16_t packetId = mqttClient.publish(topic, 1, true, String(data).c_str());
  String info = "Publishing on topic" + String(topic) + "at QoS 1, packetId: " + String(packetId)
                + ". Message: " + String(data);
  Serial.println(info);
}

void printData()
{
  String text_to_print;
  text_to_print += "Temperature = " + String(temperature) + "*C\n";
  text_to_print += "Pressure = " + String(pressure) + "hPa\n";
  text_to_print += "Hight = " + String(hight) + "m\n";
  text_to_print += "Humidity = " + String(humidity) + "%\n";
  text_to_print += "PM1.0 = " + String(pm1) + "ug/m3\n";
  text_to_print += "PM2.5 = " + String(pm25) + "ug/m3\n";
  text_to_print += "PM10 = " + String(pm10) + "ug/m3\n";
 Serial.print(text_to_print);
}

void setup() {

  // Serial for debugging
  Serial.begin(9600);

  //Setting serial for sensor
  PMS_Serial.begin(9600);
  pms7003.init(&PMS_Serial);
  
  while (!bme.begin(0x76))
  {
	Serial.println("Sensor disconnected. Check wiring");
	delay(1000);
  }

  // Attaching WiFi events
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  // Attaching MQTT events
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  // Setting MQTT connection info
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);

  connectToWifi();	
}

void loop() {

	pms7003.updateFrame();

	unsigned long currentMillis = millis();

	if(!WiFi.isConnected() || !mqttStatus) previousMillis = currentMillis;

	if (currentMillis - previousMillis >= interval) {

		previousMillis = currentMillis;

		temperature = bme.readTemperature();
		pressure = bme.readPressure() / 100.0F;
		hight = bme.readAltitude(SEALEVELPRESSURE_HPA);
		humidity = bme.readHumidity();

		pm1 = pms7003.getPM_1_0();
		pm25 = pms7003.getPM_2_5();
		pm10 = pms7003.getPM_10_0();

		printData();
		Serial.println();

		// Publish data to MQTT
		publishMQTTPacket(MQTT_PUB_TEMP, temperature);
		publishMQTTPacket(MQTT_PUB_HUM, humidity);
		publishMQTTPacket(MQTT_PUB_PRES, pressure);
		publishMQTTPacket(MQTT_PUB_HIGHT, hight);
		publishMQTTPacket(MQTT_PUB_PM1, pm1);
		publishMQTTPacket(MQTT_PUB_PM25, pm25);
		publishMQTTPacket(MQTT_PUB_PM10, pm10);
		Serial.println();
	}
}
