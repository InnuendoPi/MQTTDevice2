# MQTTDevice Version 2

## Was ist ein MQTTDevice?

MQTTDevice ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen einem MQTT Broker und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftbeerPi zu steuern.

![Startseite](https://innuendopi.github.io/MQTTDevice2/img/startseite.jpg)

## Was bietet diese Firmware?

* Ein Web Interface (WebIf) für die Konfiguration
* Sensoren (max 6)
  * Suche nach angeschlossenen Sensoren basierend auf OneWire Adressen
  * Das Leseintervall der Sensordaten und das Offset sind konfigurierbar (in Sek) 
* Aktoren (max 6)
  * PIN Auswahl (GPIO)
  * PINs in Verwendung werden ausgeblendet
  * Invertierte GPIO
  * Power Percentage: Es werden Werte zwischen 0 und 100% gesendet. Das ESP8266 "pulses" mit einem Zyklus von 1000ms
* Induktionskochfeld
  * das Induktionskochfeld GGM IDS2 kann direkt gesteuert werden
* OLED Display Integration
* WebUpdate Firmware
* Update Firmware und SPIFFS über Dateiupload
* Event handling
* TCP Server Support (Tozzi Server)
* Dateiexplorer

Installation: https://hobbybrauer.de/forum/viewtopic.php?f=58&t=19036&p=309196#p309196

# Die erste Installation

## Installation ohne den Quellcode zu compilieren

Mit Hilfe von esptool.exe (see https://github.com/igrr/esptool-ck/releases ) aus dem Ordner tools kann die Firmware auf das ESP Modul geladen werden. Das ESPTool ist für verschiedene Betriebssysteme verfügbar.
ESPtool-ck Copyright (C) 2014 Christian Klippel ck@atelier-klippel.de. This code is licensed under GPL v2.

Unter Win10 wird der USB Treiber CH341SER benötigt: http://www.wch.cn/download/CH341SER_ZIP.html

Beispiel für ein ESP8266 Modul vom Typ Wemos D1 mini mit 4MB Flash verbunden mit COM3

	* Download von github entpacken (komplett)

    * Im Ordner tools das Archiv tools.zip entpacken. Enthalten sind das esptool und das Skript Flashen.cmd

    * Eingabeaufforderung öffnen

	* in den Order .../MQTTDevice2/tools wechseln und das Skript Flashen.cmd ausführen
    Das Skript löscht alle Daten aus dem Speicher und spielt die Firmware und das Filesystem SPIFFS auf.

    alternativ manuell mit esptool:

		* Wemos D1 mini löschen:
        esptool.exe -cp COM3 -cd nodemcu -ce 
        * Flashen:
        esptool.exe -cp COM3 -cd nodemcu -ca 0x000000 -cf ..\build\MQTTDevice2.ino.bin -ca 0x200000 -cf ..\build\MQTTDevice2.spiffs.bin

	    * Das ESP8266 Modul resetten

	    * Das ESP8266 Modul startet anschließend im Access Point Modus mit der IP Adresse 192.168.4.1

    	* Das ESP8266 Modul über einen Webbrowser mit dem WLAN verbinden


## Installation mit Quellcode

Voraussetzungen: (2020.01)

* Arduino IDE 1.8.10
* Optional Microsoft VSCode + Arduino + ESP8266FS
* ESP8266 by ESP8266 Community version 2.6.3
* Folgende Bibliotheken müssen über die Arduino IDE hinzugefügt werden:
  * Standard Bibliotheken (buildin) von der Arduino IDE
    * ESP8266WiFi
    * ESP8266WebServer
    * DNSServer
    * ESP8266mDNS
    * SPI
    * Wire
  * Zusätzliche Bibliotheken
    * NTPClient by Fabrice Weinberg Version 3.2.0
    * Adafruit GFX Library by Adafruit Version 1.7.3
    * Adafruit SSD1306 by Adafruit Version 2.0.4
    * ArduinoJSON by Benoit Blanchon Version 6.13.0 
    * DallasTemperature by Miles Burton Version 3.8.0
    * OneWire By Jim Studt Version 2.3.5
    * PubSubClient by Nick O'Leary Version 2.7.0
    * WiFiManager by tzapu Version 0.15.0
    * EventManager

    Die Firmware muss mit der Einstellung Flash size 4MB (FS: 2MB OTA:~1019kB) aufgespielt werden.
    Debug Ausgaben werden in der IDE über "Debug Port" aktiviert. In der Standard Einstellung (bin Dateien) hat die Firmware nach dem Start keine Ausgaben auf dem seriellen Monitor. 

## Updates

Die Firmware bietet zwei Möglichkeiten, um Updates sehr einfach einspielen zu können.

### Update durch Dateiupload

Im Webbrowser die URL http://<IP Adresse Wemos>/update aufrufen
Hier kann Firmware und das Filesystem SPIFFS aktualisiert werden. Wenn das Filesystem SPIFFS mit Dateiupload aktualisiert wird, wird die Konfigurationsdatei überschrieben. Siehe hierzu auch Backup und Restore.

### WebUpdate

Im Webbrowser die URL http://<IP Adresse Wemos> aufrufen und die Funktion WebUpdate aufrufen.
WebUpdate aktualisiert  die Firmware, die index Datei und Zertifikate. Durch WebUpdate wird die Konfigurationsdatei nicht überschrieben.

## Backup and Restore der Konfiguration

Der Dateiexplorer ist erreichbar über den Webbrowser http://<IP Adresse Wemos>/edit 

### Backup

Auf die Datei config.txt klicken und aus dem PopUp Download auswählen.

### Restore

Auf Datei auswählen klicken, die config.txt auswählen und Upload auswählen

### config.txt editieren

Auf die Datei config.txt klicken und aus dem PopUp Edit auswählen.
Nun kann im Hauptfenster die Datei editiert werden. Zum Abspeichern CTRL+S verwenden. Vorsicht!!!

# Bedienung der Firmware

Die meisten Funktionen der Firmware sind selbsterklärend. Das Hinzufügen oder das Löschen von Sensoren und Aktoren wird daher hier nicht beschrieben.


Die Hauptfunktionen

* Hinzufügen, editieren und löschen von Sensoren
* Auto reconnect MQTT
* Auto reconnect WLAN
* OLED Display optional konfigurieren
* System Einstellungen vollständig veränderbar
* Firmware und SPIFFS Updates über Dateiupload
* Firmware WebUpdate
* Filebrowser für einefaches Datei-Management (zB backup und restore config.json)
* DS18B20 Temperatur Offset - einfaches kalibrieren der Sensoren

## Das Menü Enstellungen:

### System

IP Adresse MQTT Server (CBPi):

Unter System wird der MQTT Broker eingetragen. In den allermeisten Fällen dürfte dies mosquitto auf dem CBPi sein.
Wichtig ist, dass die Firmware MQTTDevice permanent versucht, mit dem MQTT Broker eine Verbindung aufzubauen. Wenn der MQTT Broker nicht verfügbar ist, beeinträchtigt das Geschwindigkeit vom Wemos. Der Wemos wirkt abhängig von der bereits konfiguraierten Anzahl an Sensoren und Aktoren träge bis zu sehr lahm. Beim Testen sollte daher der MQTT Broker online sein. 

mDNS:

mDNS ist eine gute Möglichkeit, um das MQTTDevice mit einem beliebigen Namen anzusprechen. In der Standardkonfiguration ist das MQTTDevice im Webbrowser über http://mqttdevice erreichbar.
Zu beachten gilt, dass mDNS Namen im Netzwerk eindeutig sein müssen. 

### Intervalle

Unter Intervalle werden die Zeitabstände konfiguriert, mit denen 
    - wie häufig Sensoren abgefragt werden und die Daten zum CBPi gesendet werden
    - wie häufig Befehle für Aktoren / Induktion vom CBPi abgeholt werden
Mit diesen Intervallen kann die Performance vom Wemos verbessert werden. Die Standard Einstellung von 5 Sekunden ist in Umgebungen mit vielen Sensoren und vielen Aktoren zu häufig. Hier wäre eher 10 bis 30 Sekunden für den kleinen Wemos besser geeignet. Dies muss individuell ausprobiert werden.  


### Der Eventmanager

Der Eventmanager behandelt Fehlverhalten. Wichtig zu Beginn: das Event handling ist in der Standard Einstellung deaktiviert!

Was soll der Wemos machen, wenn
    - die WLAN Verbindung zum MQTT Server verloren geht
    - der MQTT Server offline geht
    - ein Temperatursensor keine Daten mehr liefert
Ohne das Event handling macht der Wemos nichts automatisert. Der Zustand verbleibt unverändert.

Es gibt 4 Grundtypen von Ereignissen (Events), die automatisiert behandelt werden können: für Aktoren und für das Induktionkochfeld bei Sensorfehlern, sowie für Aktoren und das Induktionskochfeld bei WLAN und bei MQTT Fehlern. Für diese 4 Typen werden Verzögerungen für das Event handling konfiguriert. Während der Verzögerung verbleibt der Zustand unverändert.

Zusätzlich kann jeder Sensor, jeder Aktor und das Induktionskochfeld separat für das Event handling aktiviert bzw. deaktiviert werden.

Beispiel 1:
Wenn der MQTT Broker unerwartet die Verbindung beendet, dann
- wird automatisch versucht die Verbindung wieder aufzubauen
- die konfigurierte Verzögerung wird abgewartet, bevor ein Aktor automatisch ausgeschaltet wird
- das Induktionsfeld kann auf eine niedrigere Leistung gesetzt werden (von 100% auf 20% -> Temperatur halten)
Beispiel 2:
Wenn ein Temeratursensor beim Brauen -127°C meldet, dann
- kann ein Aktor Rührwerk am Sudkessel weiterlaufen. Dieser Aktor kann für das Event handling deaktiviert werden.
- Das Induktionskochfeld kann automatisch von 100% Leistung auf 20% heruntergeschaltet werden
- ein Aktor Pumpe kann abgeschaltet werden
etc.

Die Reihenfolge beim Event handling ist grundsätzlich
    - WLAN Fehler
    - MQTT Fehler
    - Sensor Fehler

### Restore

Über das Menü Restore kann der Wemos gelöscht werden. Zur Auswahl stehen
    - WLAN Einstellungen löschen
    - Alle Einstellungen löschen (WLAN und Konfiguration)

### Das OLED Display:
Diese Firmware unterstützt OLED Display monochrom OLED 128x64 I2C 0.96".
Das Display kann über das WebIf konfiguriert werden. Wenn das Display aktiviert wird, sind die PINS D1 (SDL) und D2 (SDA) belegt. Auf dem Display werden Sensoren, Aktoren und Induktion mit ihren aktuellen Werten dargestellt. Dabei bedeutet "S1 78 | A2 100 | I off" 
    - Sensor 1 meldet eine Temperatur von 78°C
    - Aktor 2 hat einen Powerlevel von 100%
    - Induktion ist ausgeschaltet (oder nicht konfiguriert)
Mit jeder Aktualisierung Display wandert die Anzeige auf den nächsten Sensor bzw. Aktor. Im Beispiel wäre das S2 und A3.

Anschluss ESP8266 D1 Mini an AZ-Delivery 0.96 i2c 128x64 OLED Display (Verwendung aller Information auf eigene Gefahr!)

 * VCC -> 3.3V
 * GND -> GND
 * SCL -> D1
 * SDA -> D2

# Anbindung an den TCP Server Tozzi

Die Firmware bietet eine Möglichkeit Daten mit dem TCP Server Tozzi auszutauschen, um eine graphische Darstellung von einem Brautag zu erstellen. Zur Konfiguration muss 
    - der TCP Server um eine MQTTDevice Seite erweitert werden
    - CBPi um ein Plugin erweitert werden
    - das MQTTDevice konfiguriert werden 

## Vorbereitung TCP Server

## Installation CBPi Plugin

## Konfiguration am MQTTDevice

# Die MQTTDevice Platine

Wichtiger Hinweis:
Alle Informationen über die Platine sind rein informativ und können falsch sein. 
Verwendung dieser Informationen auf eigene Gefahr. Jegliche Haftung wird ausgeschlossen.

In diesem Projekt wurde eine Platine für das MQTTDevice entwickelt, um mit Klemmschraubblöcken eine einfache Anbindung an Sensoren, Aktoren und an das Induktionskochfeld GGM IDS2 zu bieten. Die Platine ist mit nur wenigen Bauteilen bestückt. Die Platine bietet folgende Vorteile:
    - der Wemos D1 mini steckt auf einem Sockel und kann jederzeit abgenommen werden
    - alle GPIOs werden auf Schraubklemmen geführt
    - ein LevelShifter sorgt für 5V (statt 3V3) als Ausgangsspannung
    - die Stromversorgung vom Wemos kann bei der Verwendung einer GGM IDS2 direkt vom Induktionskochfeld genutzt werden
    - die Temperatursensoren können direkt an die Schraubklemmen angeschlossen werden (R4k7 gegen 3V3 vorhanden)

## Platine Layout

![Platine](https://innuendopi.github.io/MQTTDevice2/img/platine.jpg)

Im Ordner Info befinden sich EasyEDA Dateien, mit deren Hilfe die Platine erstellt werden kann. Ebenfalls im Ordner Info befinden sich STL Dateien für einen 3D Druck MQTTDevice Gehäuse.

## Platine Stückliste

## Platine Hinweise zum Aufbau
