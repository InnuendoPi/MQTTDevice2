# Changelog

Version 2.65

- Update:   ESP8266 3.0.2 LittleFS
- Removed:  WLAN events (moved to mqtt handling)
- Added:    CBPi3-CBPi4 Switch: Influx/Grafana nur im CBPi3 Modus
- Reverted: ESP8266 2.7.4 LittleFS (Boardbibliotheken 3.0.0 und 3.0.1 fehlerhaft)
- Fixed:    InfluxDB PowerLevel no value
- Update:   ESP8266 3.0.0 LittleFS
- Update:   ArduinoJSON 6.18
- Added:    Visualisierung bleibt nach Neustart aktiv
- Fixed:    MQTT Topic IDS2 wurde nicht angezeigt
- Fixed:    Display Update Intervall wurde fehlerhaft angezeigt
- Fixed:    Sommerzeit wurde nicht geprüft
- Optimize: Speicherverbrauch
- Optimize: Traffic
- Optimize: Web Interface (JSON)
- Added:    ESC schließt modal dialog
- Optimize: Speichern und Lesen der Konfiguration
- Changed:  Dateisystem LittleFS
- Reworked: Handling Aktoren und Sensoren
- Optimize: Web Interface
- Optimize: Speicherverbrauch
- Update:   Bootstrap und JQuery
- Optimize: WebIf
- Fixed:    WLAN reconnect
- Added:    DEBUG_MSG mit Zeitstempel
- Fixed:    Fehler beim Speichern negativer Sensor Offset behoben
- Update:   Bibliotheken aktualisiert
- Update:   ESP8266 2.7.4 (weiterhin SPIFFS)
- Added:    Support für Piezo Buzzer an D8
- Fixed:    WebUpdate Loop
- Optimize: OLED Darstellung
- Optimize: Heap Speicher
- Update:   max. 8 Aktoren und 6 Sensoren
- Added:    Support für OLED 1.3" SH1106 (siehe Dokumentation)
- Fixed:    Umstellung Sommer/Winterzeit
- Added:    Visualisierung Grafana, Influx Datenbank
