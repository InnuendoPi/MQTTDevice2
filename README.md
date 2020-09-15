# MQTTDevice 2.0

MQTTDevice ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen einem MQTT Broker und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftbeerPi (CBPi) zu steuern.

Dokumentation: <https://innuendopi.github.io/MQTTDevice2/>

## Changelog

Version 2.11

- Added:    Support für Piezo Buzzer an D0
- Removed:  mDNS Support
- Removed:  Eingebettete Grafana Charts von Web-Interface
- Fixed:    WebUpdate Loop
- Optimze:  OLED Darstellung
- Optimize: Heap Speicher
- Update:   max. 5 Aktoren und 4 Sensoren
- Added:    Support für OLED 1.3" SH1106 (siehe Dokumentation)
- Remarked: OLED 0.96" SSD1306 (siehe Dokumentation)
- Update:   Bibliotheken aktualisiert
- Fixed:    Umstellung Sommer/Winterzeit
- Update:   Libs InfluxDB, ArduinoJson, Adafruit_SSD1306
- Update:   Influx CheckDBConnect: bei einem Verbindungsfehler wird der Task Vis gestoppt
- Added:    nach einem WebUpdate erscheint eine Info
- Reworked: Neuer Button Visualisierung Start/Stop
- Added:    Datenbank Tag Sud-ID für Visualisierung hinzugefügt
- Reworked: Updates
- Added:    Visualisierung Grafana, Influx Datenbank
- Fixed:    Typo Display Update
- Removed:  Visualisierung TCPServer
- Add:      Überprüfung der Eingaben im Web Interface (int, float, char)
- Add:      Versionsanzeige im Web Interface mit mDNS Namen
- Fixed:    Fehler Update mDNS behoben
