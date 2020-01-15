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

![Platine](img/platine.jpg)

Im Ordner Info befinden sich EasyEDA Dateien, mit deren Hilfe die Platine erstellt werden kann. Ebenfalls im Ordner Info befinden sich STL Dateien für einen 3D Druck MQTTDevice Gehäuse.

## Platine Stückliste

## Platine Hinweise zum Aufbau
