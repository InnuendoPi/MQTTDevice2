# MQTTDevice 2.0

Version 2.0 **beta** Firmware ist nicht einsatzfähig!

Wichtiger Hinweis 1: Ein Update von Version 1.x auf 2.x ist nicht möglich! Der Wemos D1 mini muss gelöscht werden.
Wichtiger Hinweis 2: Zum Flashen der Firmware die Einstellung Flash size anpassen: 4MB (FS: 2MB OTA:~1019kB)

## Changelog

- Add:      Bibliothek InnuTicker erstellt
- Reworked: NTP, Sensoren, Aktoren, Induktion, TCPServer, Display, MQTTEr und WLANEr als Ticker Objekte eingebunden
- Fixed:    TCPServer URL mit Leerzeichen im Sensornamen wird durch %20 ersetzt
- Fixed:    Typo Log3 WebUpdate behoben
- Add:      WebUpdate - die Firmware kann nun über das WebInterface aktualisiert werden
- Update:   ESP8266 V2.6.3
- Reworked: Debug Ausgaben werden nun über die ESP8266 lib verarbeitet. Die Standard Einstellung für die
            Firmware Dateien (bin Dateien) ist keine Debug Ausgabe.
- Removed:  Simulation
- Removed:  Telnet - noch nicht in neue Debug lib eingebunden
- Reworked: Das WebInterface ist schlanker und übersichtlicher (und in deutsch)
- Fixed:    NTP Zeit
- Fixed:    MQTT reconnect
- Reworked: Aufruf der handles für Sensoren, Aktoren und Induktion auf Timer Objekte umgestellt
- Add:      Ticker Objekte für NTP und Reconnect WLAN/MQTT

Dokumentation: <https://innuendopi.github.io/MQTTDevice2/>
