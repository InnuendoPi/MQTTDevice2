void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    yield(); // Warte auf seriellen Monitor. Needed for native USB port only
  }
// Debug Ausgaben prüfen
#ifdef DEBUG_ESP_PORT
  Serial.setDebugOutput(true);
#endif

  Serial.println();
  Serial.println();
  // Setze Namen für das MQTTDevice
  snprintf(mqtt_clientid, 16, "ESP8266-%08X", mqtt_chip_key);
  Serial.printf("*** SYSINFO: Starte MQTTDevice %s\n", mqtt_clientid);

  // WiFi Manager
  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter cstm_mqtthost("host", "MQTT Server IP (CBPi)", mqtthost, 16);
  WiFiManagerParameter p_hint("<small>*Sobald das MQTTDevice im WLAN eingebunden ist, öffne im Browser http://mqttdevice (mDNS)</small>");
  wifiManager.addParameter(&cstm_mqtthost);
  wifiManager.addParameter(&p_hint);
  wifiManager.autoConnect(mqtt_clientid);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  if (shouldSaveConfig)
  {
    strcpy(mqtthost, cstm_mqtthost.getValue());
    saveConfig();
  }
  // Lade Dateisystem
  if (SPIFFS.begin())
  {
    // Prüfe WebUpdate
    updateSys();

    // Erstelle Timer Objekte
    setTimer();

    if (SPIFFS.exists("/config.txt")) // Lade Konfiguration
      loadConfig();
    else
      Serial.println("*** SYSINFO: Konfigurationsdatei config.txt nicht vorhanden. Setze Standardwerte ...");
  }
  else
    Serial.println("*** SYSINFO: Fehler - Dateisystem SPIFFS konnte nicht eingebunden werden!");

  // Lege Event Queues an
  gEM.addListener(EventManager::kEventUser0, listenerSystem);
  gEM.addListener(EventManager::kEventUser1, listenerSensors);
  gEM.addListener(EventManager::kEventUser2, listenerActors);
  gEM.addListener(EventManager::kEventUser3, listenerInduction);

  aktIP = WiFi.localIP();

  // Starte Webserver
  setupServer();
  // Starte NTP
  cbpiEventSystem(EM_SETNTP);
  // Pinbelegung
  pins_used[ONE_WIRE_BUS] = true;
  if (useDisplay)
  {
    pins_used[SDA] = true;
    pins_used[SDL] = true;
  }
  // Starte Sensoren
  DS18B20.begin();

  // Starte mDNS
  if (startMDNS)
    cbpiEventSystem(EM_MDNSET);
  else
  {
    Serial.print("*** SYSINFO: ESP8266 IP Addresse: ");
    Serial.println(aktIP.toString());
  }

  // Starte OLED Display
  dispStartScreen();

  // Starte MQTT
  cbpiEventSystem(EM_MQTTCON);
  cbpiEventSystem(EM_MQTTSUB);
  cbpiEventSystem(EM_LOG);

  // Verarbeite alle Events Setup
  gEM.processAllEvents();
  Serial.printf("*** SYSINFO: %s\n", timeClient.getFormattedTime().c_str());
}

void setupServer()
{
  server.on("/", handleRoot);
  server.on("/setupActor", handleSetActor);       // Einstellen der Aktoren
  server.on("/setupSensor", handleSetSensor);     // Einstellen der Sensoren
  server.on("/reqSensors", handleRequestSensors); // Liste der Sensoren ausgeben
  server.on("/reqActors", handleRequestActors);   // Liste der Aktoren ausgeben
  server.on("/reqInduction", handleRequestInduction);
  server.on("/reqSearchSensorAdresses", handleRequestSensorAddresses);
  server.on("/reqPins", handlereqPins);
  server.on("/reqSensor", handleRequestSensor); // Infos der Sensoren für WebConfig
  server.on("/reqActor", handleRequestActor);   // Infos der Aktoren für WebConfig
  server.on("/reqIndu", handleRequestIndu);     // Infos der Indu für WebConfig
  server.on("/setSensor", handleSetSensor);     // Sensor ändern
  server.on("/setActor", handleSetActor);       // Aktor ändern
  server.on("/setIndu", handleSetIndu);         // Indu ändern
  server.on("/delSensor", handleDelSensor);     // Sensor löschen
  server.on("/delActor", handleDelActor);       // Aktor löschen
  server.on("/reboot", rebootDevice);           // reboots the whole Device
  server.on("/reqDisplay", handleRequestDisplay);
  server.on("/reqDisp", handleRequestDisp); // Infos Display für WebConfig
  server.on("/setDisp", handleSetDisp);     // Display ändern
  server.on("/reqMiscSet", handleRequestMiscSet);
  server.on("/reqMisc", handleRequestMisc);       // Misc Infos für WebConfig
  server.on("/setMisc", handleSetMisc);           // Misc ändern
  server.on("/startHTTPUpdate", startHTTPUpdate); // Firmware ebUpdate

  // FSBrowser initialisieren
  server.on("/list", HTTP_GET, handleFileList); // Verzeichnisinhalt
  server.on("/edit", HTTP_GET, []() {           // Lade Editor
    if (!handleFileRead("/edit.htm"))
    {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.on("/edit", HTTP_PUT, handleFileCreate);    // Datei erstellen
  server.on("/edit", HTTP_DELETE, handleFileDelete); // Datei löschen
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  },
            handleFileUpload);

  server.onNotFound(handleWebRequests); // Sonstiges

  httpUpdate.setup(&server);
  server.begin();
}
