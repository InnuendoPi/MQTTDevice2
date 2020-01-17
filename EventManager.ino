void listenerSystem(int event, int parm) // System event listener
{
  switch (parm)
  {
  case EM_OK: // Normal mode
    break;
  // 1 - 9 Error events
  case EM_WLANER: // WLAN error -> handling
                  //  Error Reihenfolge
                  //  1. WLAN connected?
                  //  2. MQTT connected
                  //  Wenn WiFi.status() != WL_CONNECTED (wlan_state false nach maxRetries und Delay) ist, ist ein check mqtt überflüssig

    oledDisplay.wlanOK = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    if (WiFi.status() == WL_CONNECTED)
    {
      wlan_state = true;
      oledDisplay.wlanOK = true;
      break;
    }
    DEBUG_MSG("%s", "EM WLAN: WLAN Fehler ... versuche neu zu verbinden\n");
    if (millis() > (wlanconnectlasttry + wait_on_error_wlan)) // Wait bevor Event handling
    {
      if (StopOnWLANError && wlan_state)
      {
        DEBUG_MSG("EM WLAN: WLAN Verbindung verloren! StopOnWLANError: %d WLAN state: %d\n", StopOnWLANError, wlan_state);
        wlan_state = false;
        mqtt_state = false; // MQTT in error state - required to restore values
        cbpiEventActors(EM_ACTER);
        cbpiEventInduction(EM_INDER);
      }
    }
    break;
  case EM_MQTTER: // MQTT Error -> handling
    oledDisplay.mqttOK = false;
    if (pubsubClient.connect(mqtt_clientid))
    {
      DEBUG_MSG("%s", "MQTT auto reconnect successful. Subscribing..\n");
      cbpiEventSystem(EM_MQTTSUB); // MQTT subscribe
      cbpiEventSystem(EM_MQTTRES); // MQTT restore
      break;
    }
    if (millis() > (mqttconnectlasttry + wait_on_error_mqtt))
    {
      if (StopOnMQTTError && mqtt_state)
      {
        DEBUG_MSG("EM MQTTER: MQTT Broker %s nicht erreichbar! StopOnMQTTError: %d mqtt_state: %d\n", mqtthost, StopOnMQTTError, mqtt_state);
        cbpiEventActors(EM_ACTER);
        cbpiEventInduction(EM_INDER);
        mqtt_state = false; // MQTT in error state
      }
    }
    break;
  // 10-19 System triggered events
  case EM_MQTTRES: // restore saved values after reconnect MQTT (10)
    if (pubsubClient.connected())
    {
      wlan_state = true;
      mqtt_state = true;
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state)
        {
          DEBUG_MSG("EM MQTTRES: %s isOnBeforeError: %d Powerlevel: %d\n", actors[i].name_actor.c_str(), actors[i].isOnBeforeError, actors[i].power_actor);
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true; // Sensor ok
          actors[i].Update();
        }
      }
      if (!inductionCooker.induction_state)
      {
        DEBUG_MSG("EM MQTTRES: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError, inductionCooker.powerLevelBeforeError);
        inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
        inductionCooker.isInduon = true;
        inductionCooker.induction_state = true; // Induction ok
        inductionCooker.Update();
        DEBUG_MSG("EM MQTTRES: Induction restore old value: %d\n", inductionCooker.newPower);
      }
    }
    break;
  case EM_REBOOT: // Reboot ESP (11) - manual task
    // Stop actors
    cbpiEventActors(EM_ACTOFF);
    // Stop induction
    if (inductionCooker.isInduon)
      cbpiEventInduction(EM_INDOFF);
    server.send(200, "text/plain", "rebooting...");
    SPIFFS.end(); // unmount SPIFFS
    ESP.restart();
    break;
  // System run & set events
  case EM_WLAN: // check WLAN (20) and reconnect on error
    if (WiFi.status() == WL_CONNECTED)
    {
      oledDisplay.wlanOK = true;
      if (TimerWLAN.active())
        TimerWLAN.detach();
    }
    else if (!TimerWLAN.active())
    {
      TimerWLAN.attach(tickerWLAN, tickerWLANER);
      wlanconnectlasttry = millis();
      switch (WiFi.status())
      {
      case 0: // WL_IDLE_STATUS
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_IDLE_STATUS");
        break;
      case 1: // WL_NO_SSID_AVAIL
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_NO_SSID_AVAIL");
        break;
      case 2: // WL_SCAN_COMPLETED
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_SCAN_COMPLETED");
        break;
      case 3: // WL_CONNECTED
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECTED");
        break;
      case 4: // WL_CONNECT_FAILED
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECT_FAILED");
        cbpiEventSystem(EM_WLANER);
        break;
      case 5: // WL_CONNECTION_LOST
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_CONNECTION_LOST");
        cbpiEventSystem(EM_WLANER);
        break;
      case 6: // WL_DISCONNECTED
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_DISCONNECTED");
        cbpiEventSystem(EM_WLANER);
        break;
      case 255: // WL_NO_SHIELD
        DEBUG_MSG("WiFi status: Fehler rc: %d WL_NO_SHIELD");
        break;
      default:
        break;
      }
    }
    break;
  case EM_MQTT: // check MQTT (22)
    if (!WiFi.status() == WL_CONNECTED)
      break;
    if (pubsubClient.connected())
    {
      oledDisplay.mqttOK = true;
      mqtt_state = true;
      pubsubClient.loop();
      if (TimerMQTT.active())
        TimerMQTT.detach();
    }
    else if (!pubsubClient.connected())
    {
      if (!TimerMQTT.active())
      {
        TimerMQTT.attach(tickerMQTT, tickerMQTTER);
        mqttconnectlasttry = millis();
        switch (pubsubClient.state())
        {
        case -4: // MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECTION_TIMEOUT", pubsubClient.state());
          cbpiEventSystem(EM_MQTTER);
          break;
        case -3: // MQTT_CONNECTION_LOST - the network connection was broken
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECTION_LOST", pubsubClient.state());
          cbpiEventSystem(EM_MQTTER);
          break;
        case -2: // MQTT_CONNECT_FAILED - the network connection failed
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_FAILED", pubsubClient.state());
          cbpiEventSystem(EM_MQTTER);
          break;
        case -1: // MQTT_DISCONNECTED - the client is disconnected cleanly
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_DISCONNECTED", pubsubClient.state());
          cbpiEventSystem(EM_MQTTER);
          break;
        case 0: // MQTT_CONNECTED - the client is connected
          pubsubClient.loop();
          break;
        case 1: // MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_PROTOCOL", pubsubClient.state());
          break;
        case 2: // MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_CLIENT_ID", pubsubClient.state());
          break;
        case 3: // MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_UNAVAILABLE", pubsubClient.state());
          break;
        case 4: // MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_BAD_CREDENTIALS", pubsubClient.state());
          break;
        case 5: // MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
          DEBUG_MSG("EM MQTT: Fehler rc=%d MQTT_CONNECT_UNAUTHORIZED", pubsubClient.state());
          break;
        default:
          break;
        }
      }
    }
    break;
  case EM_MQTTCON:                     // MQTT connect (27)
    if (WiFi.status() == WL_CONNECTED) // kein wlan = kein mqtt
    {
      pubsubClient.setServer(mqtthost, 1883);
      pubsubClient.setCallback(mqttcallback);
      pubsubClient.connect(mqtt_clientid);
    }
    break;
  case EM_MQTTSUB: // MQTT subscribe (28)
    if (pubsubClient.connected())
    {
      DEBUG_MSG("%s\n", "MQTT verbunden. Subscribing...");
      for (int i = 0; i < numberOfActors; i++)
      {
        actors[i].mqtt_subscribe();
      }
      if (inductionCooker.isEnabled)
        inductionCooker.mqtt_subscribe();

      if (startTCP)
      {
        for (int i = 1; i < 10; i++)
        {
          if (tcpServer[i].kettle_id != "0")
          {
            tcpServer[i].mqtt_subscribe();
          }
        }
      }
      oledDisplay.mqttOK = true; // Display MQTT
      mqtt_state = true;         // MQTT state ok
      if (TimerMQTT.active())    // wenn das Ticker Objekt aktiv ist muss es detached werden!
        TimerMQTT.detach();
    }
    break;
  case EM_MDNS: // check MDSN (24)
    if (startMDNS)
      mdns.update();
    break;
  case EM_SETNTP: // NTP Update (25)
    timeClient.begin();
    millis2wait(PAUSE1SEC);
    timeClient.update();
    break;
  case EM_NTP: // NTP Update (25) -> In Ticker Objekt ausgelagert!
    timeClient.update();
    break;
  case EM_MDNSET: // MDNS setup (26)
    if (startMDNS && nameMDNS[0] != '\0' && WiFi.status() == WL_CONNECTED)
    {
      if (mdns.begin(nameMDNS))
        Serial.printf("*** SYSINFO: mDNS %s mit IP %s verbunden\n", nameMDNS, WiFi.localIP().toString().c_str());
      else
        Serial.println("*** SYSINFO: mMDNS Fehler beim Start");
    }
    break;
  case EM_DISPUP: // Display screen output update (30)
    if (oledDisplay.dispEnabled)
      oledDisplay.dispUpdate();
    break;
  case EM_TCP: // TCPServer setup
    publishTCP();
    break;
  case EM_LOG:
    if (SPIFFS.exists("/log1.txt"))
    {
      fsUploadFile = SPIFFS.open("/log1.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      DEBUG_MSG("*** SYSINFO: Update Zertifikate Anzahl Versuche %s\n", line.c_str());
      SPIFFS.remove("/log1.txt");
    }
    if (SPIFFS.exists("/log2.txt"))
    {
      fsUploadFile = SPIFFS.open("/log2.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      DEBUG_MSG("*** SYSINFO: Update Index Anzahl Versuche %s\n", line.c_str());
      SPIFFS.remove("/log2.txt");
    }
    if (SPIFFS.exists("/log3.txt"))
    {
      fsUploadFile = SPIFFS.open("/log1.txt", "r");
      String line;
      while (fsUploadFile.available())
      {
        line = char(fsUploadFile.read());
      }
      fsUploadFile.close();
      DEBUG_MSG("*** SYSINFO: Update Firmware Anzahl Versuche %s\n", line.c_str());
      SPIFFS.remove("/log3.txt");
    }
    break;
  default:
    break;
  }
}

void listenerSensors(int event, int parm) // Sensor event listener
{
  // 1:= Sensor on Err
  switch (parm)
  {
  case EM_OK:
    // all sensors ok
    lastSenInd = 0; // Delete induction timestamp after event
    lastSenAct = 0; // Delete actor timestamp after event

    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && wlan_state && mqtt_state)
    {
      for (int i = 0; i < numberOfActors; i++)
      {
        if (actors[i].switchable && !actors[i].actor_state) // Sensor in normal mode: check actor in error state
        {
          DEBUG_MSG("EM SenOK: %s isOnBeforeError: %d power level: %d\n", actors[i].name_actor.c_str(), actors[i].isOnBeforeError, actors[i].power_actor);
          actors[i].isOn = actors[i].isOnBeforeError;
          actors[i].actor_state = true;
          actors[i].Update();
          lastSenAct = 0; // Delete actor timestamp after event
        }
      }

      if (!inductionCooker.induction_state)
      {
        DEBUG_MSG("EM SenOK: Induction power: %d powerLevelOnError: %d powerLevelBeforeError: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError, inductionCooker.powerLevelBeforeError);
        if (!inductionCooker.induction_state)
        {
          inductionCooker.newPower = inductionCooker.powerLevelBeforeError;
          inductionCooker.isInduon = true;
          inductionCooker.induction_state = true;
          inductionCooker.Update();
          DEBUG_MSG("EM SenOK: Induction restore old value: %d\n", inductionCooker.newPower);
          lastSenInd = 0; // Delete induction timestamp after event
        }
      }
    }
    break;
  case EM_CRCER:
    // Sensor CRC ceck failed
  case EM_DEVER:
    // -127°C device error
  case EM_UNPL:
    // sensor unpluged
  case EM_SENER:
    // all other errors
    if (WiFi.status() == WL_CONNECTED && pubsubClient.connected() && wlan_state && mqtt_state)
    {
      for (int i = 0; i < numberOfSensors; i++)
      {
        if (!sensors[i].sens_state)
        {
          switch (parm)
          {
          case EM_CRCER:
            // Sensor CRC ceck failed
            DEBUG_MSG("EM CRCER: Sensor %s crc check failed\n", sensors[i].sens_name.c_str());
            break;
          case EM_DEVER:
            // -127°C device error
            DEBUG_MSG("%s", "EM DEVER: Sensor %s device error\n", sensors[i].sens_name.c_str());
            break;
          case EM_UNPL:
            // sensor unpluged
            DEBUG_MSG("%s", "EM UNPL: Sensor %s unplugged\n", sensors[i].sens_name.c_str());
            break;
          default:
            break;
          }
        }

        if (sensors[i].sens_sw && !sensors[i].sens_state)
        {
          if (lastSenAct == 0)
          {
            lastSenAct = millis(); // Timestamp on error
            DEBUG_MSG("EM SENER: Erstelle Zeitstempel für Aktoren wegen Sensor Fehler: %l Wait on error actors: %d\n", lastSenAct, wait_on_Sensor_error_actor / 1000);
          }
          if (lastSenInd == 0)
          {
            lastSenInd = millis(); // Timestamp on error
            DEBUG_MSG("EM SENER: Erstelle Zeitstempel für Induktion wegen Sensor Fehler: %l Wait on error induction: %d\n", lastSenInd, wait_on_Sensor_error_induction / 1000);
          }
          if (millis() > lastSenAct + wait_on_Sensor_error_actor) // Wait for approx WAIT_ON_ERROR/1000 seconds
            cbpiEventActors(EM_ACTER);
          if (millis() > lastSenInd + wait_on_Sensor_error_induction) // Wait for approx WAIT_ON_ERROR/1000 seconds
          {
            if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state)
              cbpiEventInduction(EM_INDER);
          }
        } // Switchable
      }   // Iterate sensors
    }     // wlan und mqtt state
    break;
  default:
    break;
  }
  handleSensors();
}

void listenerActors(int event, int parm) // Actor event listener
{
  switch (parm)
  {
  case EM_OK:
    break;
  case 1:
    break;
  case 2:
    break;
  case EM_ACTER:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].switchable && actors[i].actor_state && actors[i].isOn)
      {
        actors[i].isOnBeforeError = actors[i].isOn;
        actors[i].isOn = false;
        //actors[i].power_actor = 0;
        actors[i].actor_state = false;
        actors[i].Update();
        DEBUG_MSG("EM ACTER: Aktor: %s : %d isOnBeforeError: %d\n", actors[i].name_actor.c_str(), actors[i].actor_state, actors[i].isOnBeforeError);
      }
    }
    break;
  case EM_ACTOFF:
    for (int i = 0; i < numberOfActors; i++)
    {
      if (actors[i].isOn)
      {
        actors[i].isOn = false;
        actors[i].Update();
        DEBUG_MSG("EM ACTER: Aktor: %s  ausgeschaltet\n", actors[i].name_actor.c_str());
      }
    }
    break;
  default:
    break;
  }
  handleActors();
}
void listenerInduction(int event, int parm) // Induction event listener
{
  switch (parm)
  {
  case EM_OK: // Induction off
    break;
  case 1: // Induction on
    break;
  case 2:
    //DBG_PRINTLN("EM IND2: received induction event"); // bislang keine Verwendung
    break;
  case EM_INDER:
    if (inductionCooker.isInduon && inductionCooker.powerLevelOnError < 100 && inductionCooker.induction_state) // powerlevelonerror == 100 -> kein event handling
    {
      inductionCooker.powerLevelBeforeError = inductionCooker.power;
      DEBUG_MSG("EM INDER: Induktion Leistung: %d Setze Leistung Induktion auf: %d\n", inductionCooker.power, inductionCooker.powerLevelOnError);
      if (inductionCooker.powerLevelOnError == 0)
        inductionCooker.isInduon = false;
      else
        inductionCooker.newPower = inductionCooker.powerLevelOnError;

      inductionCooker.newPower = inductionCooker.powerLevelOnError;
      inductionCooker.induction_state = false;
      inductionCooker.Update();
    }
    break;
  case EM_INDOFF:
    if (inductionCooker.isInduon)
    {
      DEBUG_MSG("%s\n", "EM INDOFF: Induktion ausgeschaltet");
      inductionCooker.newPower = 0;
      inductionCooker.isInduon = false;
      inductionCooker.Update();
    }
    break;
  default:
    break;
  }
  handleInduction();
}

void cbpiEventSystem(int parm) // System events
{
  gEM.queueEvent(EventManager::kEventUser0, parm);
}

void cbpiEventSensors(int parm) // Sensor events
{
  gEM.queueEvent(EventManager::kEventUser1, parm);
}
void cbpiEventActors(int parm) // Actor events
{
  gEM.queueEvent(EventManager::kEventUser2, parm);
}
void cbpiEventInduction(int parm) // Induction events
{
  gEM.queueEvent(EventManager::kEventUser3, parm);
}

// void timerNTPCallback(void *pArg) // Timer Objekt Temperatur mit Pointer
// {
//   tickNTP = true; // Bei true wird im nächsten loop readTemperature ausgeführt
// }
