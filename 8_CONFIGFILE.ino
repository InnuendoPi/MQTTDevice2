bool loadConfig()
{
  DEBUG_MSG("%s\n", "------ loadConfig started ------");
  File configFile = SPIFFS.open("/config.txt", "r");
  if (!configFile)
  {
    DEBUG_MSG("%s\n", "Failed to open config file\n");
    DEBUG_MSG("%s\n", "------ loadConfig aborted ------\n");
    return false;
  }

  size_t size = configFile.size();
  if (size > 2048)
  {
    DEBUG_MSG("%s\n", "Config file size is too large");
    DEBUG_MSG("%s\n", "------ loadConfig aborted ------");
    return false;
  }

  //StaticJsonDocument<2048> doc; //1400
  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    DEBUG_MSG("Conf: Error Json %s\n", error.c_str());
    return false;
  }

  JsonArray actorsArray = doc["actors"];
  numberOfActors = actorsArray.size();
  if (numberOfActors > numberOfActorsMax)
    numberOfActors = numberOfActorsMax;
  int i = 0;
  for (JsonObject actorObj : actorsArray)
  {
    if (i < numberOfActors)
    {
      String actorPin = actorObj["PIN"];
      String actorScript = actorObj["SCRIPT"];
      String actorName = actorObj["NAME"];
      String actorInv = actorObj["INV"];
      String actorSwitch = actorObj["SW"];
      String actorKettle_id = "0";
      if (actorObj.containsKey("kettle_id"))
        actorKettle_id = actorObj["kettle_id"].as<String>();

      actors[i].change(actorPin, actorScript, actorName, actorInv, actorSwitch, actorKettle_id);
      DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %s SW: %s ID: %s\n", (i + 1), actorName.c_str(), actorScript.c_str(), actorPin.c_str(), actorInv.c_str(), actorSwitch.c_str(), actorKettle_id.c_str());
      i++;
    }
  }

  if (numberOfActors == 0)
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  DEBUG_MSG("%s\n", "--------------------");

  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > numberOfSensorsMax)
    numberOfSensors = numberOfSensorsMax;
  //  for (int i = 0; i < numberOfSensorsMax; i++)
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      //JsonObject sensorsObj = sensorsArray[i];
      String sensorsAddress = sensorsObj["ADDRESS"];
      String sensorsScript = sensorsObj["SCRIPT"];
      String sensorsName = sensorsObj["NAME"];
      String sensorsSwitch = sensorsObj["SW"];
      String sensorsKettle_id = "0";
      if (sensorsObj.containsKey("kettle_id"))
        sensorsKettle_id = sensorsObj["kettle_id"].as<String>();
      float sensorsOffset = 0.0;
      if (sensorsObj.containsKey("OFFSET"))
        sensorsOffset = sensorsObj["OFFSET"];

      sensors[i].change(sensorsAddress, sensorsScript, sensorsName, sensorsOffset, sensorsSwitch, sensorsKettle_id);
      DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s Offset: %f SW: %s ID: %s \n", (i + 1), sensorsName.c_str(), sensorsAddress.c_str(), sensorsScript.c_str(), sensorsOffset, sensorsSwitch.c_str(), sensorsKettle_id.c_str());
      i++;
    }
    else
      sensors[i].change("", "", "", 0.0, "", "0");
  }

  if (numberOfSensors == 0)
    DEBUG_MSG("Sensors: %ds\n", numberOfSensors);

  DEBUG_MSG("%s\n", "--------------------");

  JsonArray indArray = doc["induction"];
  JsonObject indObj = indArray[0];
  if (indObj.containsKey("ENABLED"))
  {
    inductionStatus = 1;
    String indEnabled = indObj["ENABLED"];
    String indPinWhite = indObj["PINWHITE"];
    String indPinYellow = indObj["PINYELLOW"];
    String indPinBlue = indObj["PINBLUE"];
    String indScript = indObj["TOPIC"];
    String indKettleid = "0";
    if (indObj.containsKey("kettle_id"))
      indKettleid = indObj["kettle_id"].as<String>();

    long indDelayOff = DEF_DELAY_IND; //default delay
    int indPowerLevel = 100;
    if (indObj.containsKey("PL"))
      indPowerLevel = indObj["PL"];

    if (indObj.containsKey("DELAY"))
      indDelayOff = indObj["DELAY"];

    inductionCooker.change(StringToPin(indPinWhite), StringToPin(indPinYellow), StringToPin(indPinBlue), indScript, indDelayOff, indEnabled, indPowerLevel, indKettleid);
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d ID: %s\n", inductionStatus, indScript.c_str(), indPinWhite.c_str(), indPinYellow.c_str(), indPinBlue.c_str(), (indDelayOff / 1000), indPowerLevel, indKettleid.c_str());
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionStatus);
  }
  DEBUG_MSG("%s\n", "--------------------");
  JsonArray displayArray = doc["display"];
  JsonObject displayObj = displayArray[0];
  if (displayObj["ENABLED"] == "1")
    useDisplay = true;
  else
    useDisplay = false;

  if (useDisplay)
  {
    String dispAddress = displayObj["ADDRESS"];
    dispAddress.remove(0, 2);
    char copy[4];
    dispAddress.toCharArray(copy, 4);
    int address = strtol(copy, 0, 16);
    if (displayObj.containsKey("updisp"))
      DISP_UPDATE = displayObj["updisp"];

    oledDisplay.dispEnabled = true;
    oledDisplay.change(address, oledDisplay.dispEnabled);
    DEBUG_MSG("OLED display: %d Address: %s Update: %d\n", oledDisplay.dispEnabled, dispAddress.c_str(), (DISP_UPDATE / 1000));
    os_timer_disarm(&TimerDisp);
    os_timer_arm(&TimerDisp, DISP_UPDATE, true);
  }
  else
  {
    useDisplay = false;
    oledDisplay.dispEnabled = false;
    DEBUG_MSG("OLED Display: %d\n", oledDisplay.dispEnabled);
    os_timer_disarm(&TimerDisp);
  }

  DEBUG_MSG("%s\n", "--------------------");

  // Misc Settings
  JsonArray miscArray = doc["misc"];
  JsonObject miscObj = miscArray[0];

  if (miscObj.containsKey("del_sen_act"))
    wait_on_Sensor_error_actor = miscObj["del_sen_act"];

  if (miscObj.containsKey("del_sen_ind"))
    wait_on_Sensor_error_induction = miscObj["del_sen_ind"];

  if (miscObj.containsKey("delay_mqtt"))
    wait_on_error_mqtt = miscObj["delay_mqtt"];

  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor/1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction/1000);

  if (miscObj["enable_mqtt"] == "1")
  {
    StopOnMQTTError = true;
    DEBUG_MSG("Switch off actors on error after %d\n", (wait_on_error_mqtt / 1000));
  }
  else
  {
    StopOnMQTTError = false;
    DEBUG_MSG("%s\n", "Switch off actors on error disabled");
  }

  if (miscObj.containsKey("delay_wlan"))
    wait_on_error_wlan = miscObj["delay_wlan"];

  if (miscObj["enable_wlan"] == "1")
  {
    StopOnWLANError = true;
    DEBUG_MSG("Switch off induction on error after %d sec\n", (wait_on_error_wlan / 1000));
  }
  else
  {
    StopOnWLANError = false;
    DEBUG_MSG("%s\n", "Switch off induction on error disabled");
  }
  if (miscObj.containsKey("mdns_name"))
    strlcpy(nameMDNS, miscObj["mdns_name"], sizeof(nameMDNS));

  if (miscObj["mdns"] == "1")
  {
    startMDNS = true;
    DEBUG_MSG("mDNS activated: %s\n", nameMDNS);
  }
  else
  {
    startMDNS = false;
    DEBUG_MSG("%s\n", "mDNS disabled");
  }

  if (miscObj.containsKey("upsen"))
    SEN_UPDATE = miscObj["upsen"];
  if (miscObj.containsKey("upact"))
    ACT_UPDATE = miscObj["upact"];
  if (miscObj.containsKey("upind"))
    IND_UPDATE = miscObj["upind"];
  os_timer_disarm(&TimerAct);
  os_timer_arm(&TimerAct, ACT_UPDATE, true);
  os_timer_disarm(&TimerInd);
  if (inductionCooker.isEnabled)
    os_timer_arm(&TimerInd, IND_UPDATE, true);
  os_timer_disarm(&TimerSen);
  os_timer_arm(&TimerSen, SEN_UPDATE, true);

  DEBUG_MSG("Sensors update intervall: %d sec\n", (SEN_UPDATE / 1000));
  DEBUG_MSG("Actors update intervall: %d sec\n", (ACT_UPDATE / 1000));
  DEBUG_MSG("Induction update intervall: %d sec\n", (IND_UPDATE / 1000));
  if (miscObj["tcp"] == "1")
    startTCP = true;
  else
    startTCP = false;
  if (miscObj.containsKey("uptcp"))
    TCP_UPDATE = miscObj["uptcp"];

  if (miscObj.containsKey("TCPHOST"))
    strlcpy(tcpHost, miscObj["TCPHOST"], sizeof(tcpHost));
  else
    startTCP = false;
  if (miscObj.containsKey("TCPPORT"))
    tcpPort = miscObj["TCPPORT"];
  else
    startTCP = false;
  if (startTCP)
    DEBUG_MSG("TCP Server IP %s Port %d Update %d\n", tcpHost, tcpPort, (TCP_UPDATE / 1000));
  else
    DEBUG_MSG("TCP Server: %d\n", startTCP);

  if (miscObj.containsKey("MQTTHOST"))
  {
    strlcpy(mqtthost, miscObj["MQTTHOST"], sizeof(mqtthost));
    DEBUG_MSG("MQTT server IP: %s\n", mqtthost);
  }
  else
  {
    DEBUG_MSG("MQTT server not found in config file. Using default server address: %s\n", mqtthost);
  }
  DEBUG_MSG("%s\n", "------ loadConfig finished ------");
  configFile.close();
  DEBUG_MSG("Config file size %d\n", size);
  size_t len = measureJson(doc);
  DEBUG_MSG("JSON config length: %d\n", len);
  int memoryUsed = doc.memoryUsage();
  DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);

  os_timer_disarm(&TimerTCP);
  if (startTCP)
  {
    setTCPConfig();
    os_timer_arm(&TimerTCP, TCP_UPDATE, true);
  }
  return true;
}

void saveConfigCallback()
{

  if (SPIFFS.begin())
  {
    saveConfig();
    shouldSaveConfig = true;
  }
  else
  {
    Serial.println("*** SYSINFO: WiFiManager failed to save MQTT broker IP");
  }
}

bool saveConfig()
{
  DEBUG_MSG("%s\n", "------ saveConfig started ------");
  //StaticJsonDocument<2048> doc; // 1400
  DynamicJsonDocument doc(2500);

  // Write Actors
  JsonArray actorsArray = doc.createNestedArray("actors");
  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = actorsArray.createNestedObject();
    actorsObj["PIN"] = PinToString(actors[i].pin_actor);
    actorsObj["NAME"] = actors[i].name_actor;
    actorsObj["SCRIPT"] = actors[i].argument_actor;
    actorsObj["INV"] = actors[i].getInverted();
    actorsObj["SW"] = actors[i].getSwitchable();
    actorsObj["kettle_id"] = actors[i].kettle_id;
    DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %s SW: %s ID: %s\n", (i + 1), actors[i].name_actor.c_str(), actors[i].argument_actor.c_str(), PinToString(actors[i].pin_actor).c_str(), actors[i].getInverted().c_str(), actors[i].getSwitchable().c_str(), actors[i].kettle_id.c_str());
  }
  if (numberOfActors == 0)
    DEBUG_MSG("Actors: %d\n", numberOfActors);

  DEBUG_MSG("%s\n", "--------------------");

  // Write Sensors
  JsonArray sensorsArray = doc.createNestedArray("sensors");
  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = sensorsArray.createNestedObject();
    sensorsObj["ADDRESS"] = sensors[i].getSens_adress_string();
    sensorsObj["NAME"] = sensors[i].sens_name;
    sensorsObj["OFFSET"] = sensors[i].sens_offset;
    sensorsObj["SCRIPT"] = sensors[i].sens_mqtttopic;
    sensorsObj["SW"] = sensors[i].getSwitchable();
    sensorsObj["kettle_id"] = sensors[i].kettle_id;
    DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s Offset: %f SW: %s ID: %s\n", (i + 1), sensors[i].sens_name.c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].sens_mqtttopic, sensors[i].sens_offset, sensors[i].getSwitchable().c_str(), sensors[i].kettle_id.c_str());
  }
  if (numberOfSensors == 0)
    DEBUG_MSG("Sensors: %d\n", numberOfSensors);

  DEBUG_MSG("%s\n", "--------------------");

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.isEnabled)
  {
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
    indObj["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
    indObj["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
    indObj["TOPIC"] = inductionCooker.mqtttopic;
    indObj["DELAY"] = inductionCooker.delayAfteroff;
    indObj["ENABLED"] = "1";
    indObj["PL"] = inductionCooker.powerLevelOnError;
    indObj["kettle_id"] = inductionCooker.kettle_id;
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d ID: %s\n", inductionCooker.isEnabled, inductionCooker.mqtttopic.c_str(), PinToString(inductionCooker.PIN_WHITE).c_str(), PinToString(inductionCooker.PIN_YELLOW).c_str(), PinToString(inductionCooker.PIN_INTERRUPT).c_str(), (inductionCooker.delayAfteroff / 1000), inductionCooker.powerLevelOnError, inductionCooker.kettle_id.c_str());
  }
  else
    DEBUG_MSG("Induction: %d\n", inductionCooker.isEnabled);

  DEBUG_MSG("%s\n", "--------------------");

  // Write Display
  JsonArray displayArray = doc.createNestedArray("display");
  if (oledDisplay.dispEnabled)
  {
    JsonObject displayObj = displayArray.createNestedObject();
    displayObj["ENABLED"] = "1";
    displayObj["ADDRESS"] = String(decToHex(oledDisplay.address, 2));
    displayObj["updisp"] = DISP_UPDATE;

    if (oledDisplay.address == 0x3C || oledDisplay.address == 0x3D)
    {
      display.ssd1306_command(SSD1306_DISPLAYON);
      cbpiEventSystem(EM_DISPUP);
    }
    else
    {
      displayObj["ENABLED"] = "0";
      oledDisplay.dispEnabled = false;
      useDisplay = false;
    }
    DEBUG_MSG("OLED display: %d Address: %s Update: %d\n", oledDisplay.dispEnabled, String(decToHex(oledDisplay.address, 2)).c_str(), (DISP_UPDATE / 1000));
    os_timer_disarm(&TimerDisp);
    os_timer_arm(&TimerDisp, DISP_UPDATE, true);
  }
  else
  {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    DEBUG_MSG("OLED display: %d\n", oledDisplay.dispEnabled);
    os_timer_disarm(&TimerDisp);
  }

  DEBUG_MSG("%s\n", "--------------------");

  // Write Misc Stuff
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor/1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction/1000);
  miscObj["delay_mqtt"] = wait_on_error_mqtt;
  if (StopOnMQTTError)
  {
    miscObj["enable_mqtt"] = "1";
    DEBUG_MSG("Switch off actors on error enabled after %d sec\n", (wait_on_error_mqtt / 1000));
  }
  else
  {
    miscObj["enable_mqtt"] = "0";
    DEBUG_MSG("%s\n", "Switch off actors on error disabled");
  }

  miscObj["delay_wlan"] = wait_on_error_wlan;

  if (StopOnWLANError)
  {
    miscObj["enable_wlan"] = "1";
    DEBUG_MSG("Switch off induction on error enabled after %d sec\n", (wait_on_error_wlan / 1000));
  }
  else
  {
    miscObj["enable_wlan"] = "0";
    DEBUG_MSG("%s\n", "Switch off induction on error disabled");
  }

  miscObj["mdns_name"] = nameMDNS;
  if (startMDNS)
    miscObj["mdns"] = "1";
  else
    miscObj["mdns"] = "0";

  miscObj["TCPHOST"] = tcpHost;
  miscObj["TCPPORT"] = tcpPort;
  miscObj["uptcp"] = TCP_UPDATE;
  if (startTCP)
  {
    miscObj["tcp"] = "1";
    DEBUG_MSG("TCP Server IP %s Port %d Update %d\n", tcpHost, tcpPort, (TCP_UPDATE / 1000));
  }
  else
  {
    miscObj["tcp"] = "0";
    DEBUG_MSG("TCP Server: %d\n", startTCP);
  }
  miscObj["MQTTHOST"] = mqtthost;  
  miscObj["upsen"] = SEN_UPDATE;
  miscObj["upact"] = ACT_UPDATE;
  miscObj["upind"] = IND_UPDATE;
  os_timer_disarm(&TimerAct);
  os_timer_arm(&TimerAct, ACT_UPDATE, true);
  os_timer_disarm(&TimerInd);
  if (inductionCooker.isEnabled)
    os_timer_arm(&TimerInd, IND_UPDATE, true);
  os_timer_disarm(&TimerSen);
  os_timer_arm(&TimerSen, SEN_UPDATE, true);

  DEBUG_MSG("Sensor update interval %d sec\n", (SEN_UPDATE / 1000));
  DEBUG_MSG("Actors update interval %d sec\n", (ACT_UPDATE / 1000));
  DEBUG_MSG("Induction update interval %d sec\n", (IND_UPDATE / 1000));
  DEBUG_MSG("MQTT broker IP: %s\n", mqtthost);

  size_t len = measureJson(doc);
  int memoryUsed = doc.memoryUsage();

  if (len > 2048 || memoryUsed > 2500)
  {
    DEBUG_MSG("JSON config length: %d\n", len);
    DEBUG_MSG("JSON memory usage: %d\n", memoryUsed);
    DEBUG_MSG("%s\n", "Failed to write config file - config too large");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    return false;
  }

  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile)
  {
    DEBUG_MSG("%s\n", "Failed to open config file for writing");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();
  DEBUG_MSG("%s\n", "------ saveConfig finished ------");
  aktIP = WiFi.localIP();
  String Network = WiFi.SSID();
  DEBUG_MSG("ESP8266 device IP Address: %s\n", aktIP.toString().c_str());
  DEBUG_MSG("Configured WLAN SSID: %s\n", Network.c_str());
  DEBUG_MSG("%s\n", "---------------------------------");
  os_timer_disarm(&TimerTCP);
  if (startTCP)
  {
    setTCPConfig();
    os_timer_arm(&TimerTCP, TCP_UPDATE, true);
  }
  return true;
}
