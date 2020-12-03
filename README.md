# MQTTDevice 2.0

MQTTDevice2 ermöglicht die ANbindung von Sensoren, Aktoren und einem Induktionskochfeld per WLAN an [CraftBeerPi V3](https://github.com/Manuel83/craftbeerpi3).

MQTTDevice2 ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen dem MQTT Broker mosquitto und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftBeerPi V3 zu steuern.

## Dokumentation

Eine ausführliche Dokumentation liegt auf github pages: <https://innuendopi.github.io/MQTTDevice2/>

## Sketch Information

Bibliotheken: (Stand ab Version 2.197, 12.2020)

- ESP8266 2.7.4 (SPIFFS)
- Arduino IDE 1.8.13
- Visual Code 1.48.2
- PubSubClient 2.8.0
- ArduinoJSON 6.17
- InfluxDB 3.6
- WiFiManager 2.0

Board Konfiguration:
Flash size 4MB (FS:2MB OTA:~1019kB)
SSL support all SSL ciphers (most comp)
Exceptions Lagacy
IwIP variant v2 lower mem

Debug Ausgabe:
Für Debug Ausgaben muss der Debug Port auf Serial eingestellt werden. Für spezielle Debug Ausgaben entsprechend den Debug Level einstellen (default none).
