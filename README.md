# MQTTDevice 2.0

MQTTDevice ist ein Arduino Sketch für die Module ESP8266 Wemos D1 mini. Damit ist es möglich eine Kommunikation zwischen einem MQTT Broker und einem ESP8266 herzustellen, um Sensoren und Aktoren mit CraftbeerPi (CBPi) zu steuern.

Dokumentation: <https://innuendopi.github.io/MQTTDevice2/>

## Changelog

Version 2.02

- Reworked: Neuer Button Visualisierung Start/Stop
- Added:    Datenbank Tag Sud-ID für Visualisierung hinzugefügt
- Reworked: Updates
- Added:    Visualisierung Grafana, Influx Datenbank
- Fixed:    Typo Display Update
- Removed:  Visualisierung TCPServer
- Add:      Überprüfung der Eingaben im Web Interface (int, float, char)
- Add:      Versionsanzeige im Web Interface mit mDNS Namen
- Fixed:    Fehler Update mDNS behoben
