# MQTTDevice 2.0

MQTTDevice2 ermöglicht die Anbindung von Sensoren, Aktoren und einem Induktionskochfeld per WLAN an [CraftBeerPi V3](https://github.com/Manuel83/craftbeerpi3).

MQTTDevice2 ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen dem MQTT Broker mosquitto und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftBeerPi V3 zu steuern. Zusätzlich unterstützt der Sketch eine Visualisierung von Temperaturen und Leistungen während der Braurasten mit Grafana.

## Update Hinweis Version 2.5

Beim Update auf Version 2.5 oder neuer von 2.20 oder älter muss das Dateisystem ersetzt werden.
Die Firmware und das Filesystem LittleFS müssen neu geflasht werden. Ein WebUpdate von 2.2x auf 2.50 oder neuer ist nicht möglich.

## Dokumentation

Eine ausführliche Dokumentation liegt auf github pages: <https://innuendopi.github.io/MQTTDevice2/>

## Unterstützung

Unterstützung gibt es im Hobbybrauerforum <https://hobbybrauer.de/forum/>

## Sketch Information

Bibliotheken: (Stand ab Version 2.56, 07.2021)

- ESP8266 2.7.4 (LittleFS)
- Arduino IDE 1.8.15
- Visual Code 1.57.1 + modifiziertes ESP8266FS Plugin
- PubSubClient 2.8.0
- ArduinoJSON 6.18
- InfluxDB 3.8
- WiFiManager 2.0

Board Konfiguration:
Flash size 4MB (FS:2MB OTA:~1019kB)
SSL support all SSL ciphers (most comp)
Exceptions Disabled
IwIP variant v2 lower mem

Debug Ausgabe:
Für Debug Ausgaben muss der Debug Port auf Serial eingestellt werden. Für spezielle Debug Ausgaben entsprechend den Debug Level einstellen (default none).
