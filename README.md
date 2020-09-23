# MQTTDevice 2.0

MQTTDevice ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen einem MQTT Broker und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftbeerPi (CBPi) zu steuern.

Dokumentation: <https://innuendopi.github.io/MQTTDevice2/>

## Changelog

Version 2.12

- Update:   ESP8266 2.7.4 (weiterhin SPIFFS)
- Update:   Bibliotheken aktualisiert
- Added:    Support für Piezo Buzzer an D8
- Fixed:    WebUpdate Loop
- Optimize: OLED Darstellung
- Optimize: Heap Speicher
- Update:   max. 5 Aktoren und 4 Sensoren
- Added:    Support für OLED 1.3" SH1106 (siehe Dokumentation)
- Fixed:    Umstellung Sommer/Winterzeit
- Added:    Visualisierung Grafana, Influx Datenbank

Bibliotheken: (Stand ab Version 2.12, 09.2020)

- ESP8266 2.7.4 (SPIFFS)
- Arduino IDE 1.8.13
- Visual Code 1.48.2
- PubSubClient 2.7.0
- ArduinoJSON 6.16
- InfluxDB 3.3.0
- WiFiManager 0.15.0
