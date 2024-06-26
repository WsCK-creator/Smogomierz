
# Smogomierz PCKZiU

Smogomierz jest projektem którego celem jest gromadzenie i wyświetlanie danych pogodowych takich jak: temperatura, wilgotność, ciśnienie, PM1.0, PM2.5, PM10.0.  

## Legenda

1. Wstęp
2. Serwer
    1. Instalacja Systemu
    2. Instalacja Oprogramowania
    3. Konfiguracja oprogramowania
3. Klient
    1. Lista części
    2. Program do ESP8266

# Wstęp

System smogomierza składa się z dwóch głównych części z **serwera** (raspberry pi) oraz **klienta** (esp8266). Zadaniem serwera jest :

- Udostępnianie możliwości wysyłania danych (Mosquitto broker)
- Przetwarzanie tych danych (Node-RED)
- Gromadzenie danych (InfluxDB)
- Wyświetlanie danych (Grafana)

Zadania te realizowane przez aplikacje wirtualizowane na serwerze. Wirtualizacją zajmuję sie aplikacja **Docker** i skrypt pomocniczy **Docker-compose**.

Do zadań klienta należą :

- Odczyt danych z czujnika
- Wysyłanie danych do serwera

Schemat komunikacji i działania smogomierza znajduje się w zdjęciu poniżej.

![Schemat działania](/assets/schemat_dzialania.svg "Schemat działania")

# Serwer

Poniżej znajduje się proces konfiguracji serwera.

## 1 - Instalacja Systemu

Aby zainstalowac system na Raspberry pi należy pobrać i zainstalować **Raspbery Pi Imager** ze sotrony [Raspbery Pi](https://www.raspberrypi.com/software/). Po zainstalowaniu i uruchomieniu programu pojawi się taki ekran aplikacji :

![Aplikacja Raspbery Pi Imager](/assets/rpi_imager.jpg "Aplikacja Raspbery Pi Imager")

Należy wtedy wybrać odpowiednie urządzenie z którego korzystamy. Z zakładki system operacyjny wybieramy **Raspberry Pi OS Legacy**, a następnie naszą kartę SD i klikamy dalej. Aplikacja powina nas zapytać o to czy chcemy użyć niestandardowych ustawień systemy.

![Pytanie o niestandardowe ustawienia](/assets/rpi_imager_custom_os.jpg "Pytanie o niestandardowe ustawienia")

Klikamy edytuj ustawienia i uzupełniamy jak na poniższym zdjęciu. Hasło poniżej to **3Dprinter**.

![Ustawienia główne](/assets/rpi_imager_custom_os_settings_general.jpg "Ustawienia główne")

Następnie przechodzimy do zakładki **SERVICES** i załączmy SHH.

![Ustawienia SHH](/assets/rpi_imager_custom_os_settings_ssh.jpg "Ustawienia SHH")

Zapisujemy ustawienia i Rozpoczynamy proces wygrywania systemu na kartę.

## 2 - Instalacja Oprogramowania

### 2.1 Łączenie z serwerem

Aby zainstalować oprogramowanie należy wsadzić kartę do raspberry pi podpiąć zasilanie i podłączyć do internetu kablem lub przez WiFi. W komputerze podpiętym do tego samego internetu należy uruchomić terminal połączyć się z serwerem przez ssh wpisując komende :

> `ssh serverrpi@serverrpi`

Podczas pierwszego łączenia terminal zapyta się czy na pewno chcemy się połączyć zatwierdzamy tą akcję wpisując **yes** do terminala, następnie wpisujemy hasło i zatwierdzamy enterem.

W razie problemów z łaczeniem można też spróbować zamiast nazwy serwera użyć jego ipv4 np. :

> `ssh serverrpi@192.168.0.107`

Może wystąpić też problem o istnieniu już takiego urządzenia w liście kluczy należy wtedy edytować plik z kluczami i usunąć urządzenie.

### 2.2 Aktualizacja

Przed rozpoczęciem jakiejkolwiek instalacji oprogramowania należy zainstalować aktualizacje poprzez wpisanie komendy :

> `sudo apt-get update && sudo apt-get upgrade`

Akcję tą trzeba potwierdzić wpisując **Y** do konsoli. Aktualizacja może zająć trochę czasu w zależności od połączenia internetowego. Może pojawić się też dodatkowa interakcja podczas instalacji należy wtedy postępować według instrukcji. Poniżej znajduje się przykładowe zdjęcie z tej operacji.

![Aktualizacja systemu](/assets/system_update.jpg "Aktualizacja systemu")

Po ukończeniu instalacji należy uruchomić ponownie urządzenie i połączyć się ponownie. Uruchomić ponownie można poprzez wpisanie komendy :

> `sudo reboot`

### 2.3 Instalacja Docker'a

Następnym krokiem jest pobranie skryptu instalacyjnego do Docker'a.

> `curl -fsSL https://get.docker.com -o get-docker.sh`

Po pobraniu skryptu należy go uruchomić komendą

> `sudo sh get-docker.sh`

Poniżej zdjęcie z terminalu procesu części instalacji :

![Instalacja Docker](/assets/docker_install.jpg "Instalacja Docker")

Po zainstalowaniu Docker'a należy dodać uzytkownika do grupy użytkowników Docker

> `sudo usermod -aG docker ${USER}`

Aby aplikacje uruchamiały się wraz z systemem należy dodać Docker'a do serwisów uruchamianych przy starcie :

> `sudo systemctl enable docker`

### 2.4 Instalacja Docker-compose

Aby zainstalowac Docker-compose potrzebny jest moduł pip3:

> `sudo apt-get install libffi-dev libssl-dev`
>
> `sudo apt install python3-dev`
>
> `sudo apt-get install -y python3 python3-pip`

Następnie należy zainstalować Docker-compose wpisując komendę :

> `sudo pip3 install docker-compose`

### 2.5 Instalacja aplikacji

Aby zainstalować aplikacje należy przenieść się od głównego folderu wpisując :

> `cd`

Następnie należy utworzyć folder containers w którym będą przetrzymywane pliki aplikacji :

> `mkdir containers`

Utworzyć plik docker-compose.yml :

> `touch containers/docker-compose.yml`

Następnie należy wkleić zawartość pliku **

**, znajdującego się w folderze **"smog\config filse for server"**, do pliku na serwerze. Plik na serwerze można edytować komendą :

> `nano containers/docker-compose.yml`

Aby zapisać plik na serwerze należy kliknąć ctrl+s i ctrl+x.

Instalowanie aplikacji rozpoczyna się wpisując komendę :

> `sudo docker-compose -f containers/docker-compose.yml up -d`

## 3 - Konfiguracja oprogramowania

### 3.1 Konfiguracja Mosquitto

Do skonfigurowania Mosquitto trzeba utworzyć plik konfiguracyjny

> `sudo touch containers/config/mosquitto.conf`

Następnie należy wkleić zawartość pliku **mosquitto.conf**, znajdującego się w folderze **"smog\config filse for server"**, do pliku na serwerze. Plik na serwerze można edytować komendą :

> `sudo nano containers/config/mosquitto.conf`

Następnym plikiem który musimy utworzyć jest plik z hasłami :

> `sudo touch containers/config/pwfile`

Musimy jeszcze zmienić właściwości dostępu do pliku. Robimy to wpisują komendy :

> `sudo chmod 0700 containers/config/pwfile`
>
> `sudo chown root containers/config/pwfile`
>
> `sudo chgrp root containers/config/pwfile`

Aby dodać użytkownika oraz zapisać zanim wprowadzone wcześnie musimy znać ID kontenera. Takich informacji dostarczy nam komenda :

> `sudo docker ps`

Na początku musimy uruchomić ponownie nasz kontener :

> `sudo docker restart <container-id>`

Podczas podawania ID nie trzeba przepisywać całego ID wystarczy ciąg dzięki któremu można go odróżnić od innych kontenerów (może to być nawet jeden znak, w tym przypadku mogło by to być tylko "a"). Poniżej zdjęcie ze znalezieniem ID oraz restartem kontenera :

![Restart kontenera mosquitto](/assets/mosquitto_restart.jpg "Restart kontenera mosquitto")

Żeby dodać użytkownika musimy połączyć się ze wewnętrznym terminalem aplikacji :

> `sudo docker exec -it <container-id> sh`

W konsoli aplikacji wpisujemy :

> `mosquitto_passwd -c /mosquitto/config/pwfile user`

Następnie podajemy hasło **3Dprinter** i wychodzimy z terminala aplikacji wpisując **exit** i uruchamiamy ponownie kontener :

> `sudo docker restart <container-id>`

### 3.2 Konfiguracja Node-Red

Uruchom dowolną przeglądarkę i połącz się z witryną **serverrpi:1880**. Powinien ci się ukazać interface Node-Red.

![Interface Node-Red](/assets/node_red_interface.jpg "Interface Node-Red")

W razie problemów z połaczeniem możesz spróbować uruchomić ponownie komputer i serwer lub spróbować połączyć się po ip serwera.

W prawy górnym rogu należy kliknąć na ikonkę rozwijania menu i wybrać **Manage palette**.

![Menu Node-Red](/assets/node_red_menu.jpg "Menu Node-Red")

W oknie którym wyskoczyło należy wybrac zakładkę **Install** i zainstalować dwa rozszerzenia:

- node-red-contrib-influxdb
- node-red-dashboard

Po zainstalowaniu rozszerzeń należy zamknąć okno z rozszerzeniami i z menu wybrać zakładkę import i załadować plik **flows.json** znajdujący się w folderze **"smog\config filse for server"**

Następnie wejść do nowej zakładki **Transfer danych** i dwa razy kliknąć lewym przyciskiem myszy na różowy węzeł i kliknąć w ikonę edycji **Servera**. Klikamy w zakładkę **Security** i wpisujemy wczesniej skonfigurowane ustawienia serwera mosquitto czyli :

> Username: User
>
>Password: 3Dprinter

![Ustawienia logowania Mosquitto](/assets/node_red_mosquitto.jpg "Ustawienia logowania Mosquitto")

Następnie aktualizujemy zmiany i klikamy dwa razy w brązowy węzeł. Tam klikamy w ikonke edycji **Servera**. Tam tak ja w przypadku poprzedniego węzła wpisujemy dane :

> Username: User
>
>Password: 3Dprinter

![Ustawienia logowania Influxdb](/assets/node_red_influxdb.jpg "Ustawienia logowania Influxdb")

Tak jak poprzednio aktualizujemy zmiany i na sam koniec klikamy czerwony przycisk **Deploy** w prawym górnym rogu. Po poprawnym skonfigurowaniu pod różowymi węzłami powinien pojawić się zielony kwadrat z napisem connected i konsoli debugowania nie powinien pojawić się żaden błąd (konsola debugowania znajduje się po prawej stronie interface'u i jest oznacza ikonką robaka). Zdjęcie poprawnie skonfigurowanego Node-Red :

![Poprawnie skonfigurowane Node-Red](/assets/node_red_done.jpg "Poprawnie skonfigurowane Node-Red")

### 3.3 Konfiguracja Grafana

 ***TODO***

# Klient

Poniżej znajdują się informacje na konfiguracji esp8266.

## 1 - Lista części

Klient smogomierza składa się z :

- Płytki mikrokontrolera (ESP8266)
- Zasilacza (HLK-5M05)
- Czujnika pyłu (PMS7003)
- Czujnika wilgotności, temperatury oraz ciśnienia (BME280)

Poniżej schemat połączenia części :

![Schemat połaczenia części](/assets/connection_diagram.jpg "Schemat połaczenia części")

**Zasilacza (HLK-5M05)** przekształca napięcie przemienne do stałego i zasila **wszystkie czujniki**. **Czujnik pyłu (PMS7003)** jest połączony z **mikrokontrolerem (ESP8266)** przez interfejs **URAT**, natomiast **czujnik warunków atmosferycznych (BME280)** jest połączony poprzez interfejs **I2C**.

## 2 - Program do ESP8266

### 2.1 Instalacja oprogramowania

Do wgrania i edytowania kodu jest potrzebny program **Visual Studio Code** który można poprać *[tutaj](https://code.visualstudio.com/download)*.

Po pobraniu instalatora należy go uruchomić i podążać za jego krokami.

### 2.2 Konfiguracja środowiska

Aby kompilować kod będzie nam potrzebne **środowisko**, aby je zainstalować należy kliknąć w przycisk **Extensions** i wyszukać w pasku wyszukiwania **PlatformIO IDE**, a następnie kliknąć **Install**, jak pokazano na zdjęciu poniżej :

!["Instalacja PlatformIO IDE"](/assets/installing_platrformIO.jpg "Instalacja PlatformIO IDE")

Następnym krokiem jest dodanie **płytki ESP8266** do środowiska aby to osiągnąć należy na pasku po lewej stronie kliknąć w nowo dodaną ikonkę **PlatformIO**, a następnie w przycisk **Open** aby otworzyć **interfejs Środowiska PlatformIO IDE**. Następnie klikamy w **Platforms**, zakładkę **Embedded**, wpisujemy w wyszukiwarkę **espressif** i klikamy w **Espressif 8266**. Poniżej zdjęcie procesu :

!["Dodawanie płytki ESP8266"](/assets/installing_esp.jpg "Dodawanie płytki ESP8266")

Następnie klikamy install **Install**. Po poprawnym zainstalowaniu powinien wyskoczyć komunikat.

!["Instalowanie płytki ESP8266"](/assets/installing_esp_final.jpg "Instalowanie płytki ESP8266")

### 2.3 Konfiguracja, kompilacja oraz wgrywanie projektu

Na początku musimy otworzyć folder obszaru roboczego robimy to poprzez naciśnięcie przycisku **Pick a folder** i wybieramy folder o nazwie **smog_esp8266**

!["Otwieranie obszaru roboczego"](/assets/opening_folder.jpg "Otwieranie obszaru roboczego")

Następnie w **main.cpp** w **linijce 14** nalezy zmienić **SSID siecie** na **nazwę sieci** do której jest połączony server, w **linijce 15** **haslo** należy zamienić na **hasło sieci** i w **linijce 18** **podane IP** należy zmienić na **IP servera**.

Aby wgrać program do **ESP8266** należy w dolnym pasku kliknąć w ikonkę **Upload**. Poniżej zdjęcie z umiejscowieniem ikony:

!["Wgrywanie programu"](/assets/compile.jpg "Wgrywanie programu")
