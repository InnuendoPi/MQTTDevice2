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
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  DynamicJsonDocument doc(2500);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    DEBUG_MSG("Conf: Error Json %s\n", error.c_str());
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
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
      String actorGrafana = actorObj["GRAF"];

      actors[i].change(actorPin, actorScript, actorName, actorInv, actorSwitch, actorGrafana);
      DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %s SW: %s GRAF: %s\n", (i + 1), actorName.c_str(), actorScript.c_str(), actorPin.c_str(), actorInv.c_str(), actorSwitch.c_str(), actorGrafana.c_str());
      i++;
    }
  }

  if (numberOfActors == 0)
  {
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  }
  DEBUG_MSG("%s\n", "--------------------");

  JsonArray sensorsArray = doc["sensors"];
  numberOfSensors = sensorsArray.size();

  if (numberOfSensors > numberOfSensorsMax)
    numberOfSensors = numberOfSensorsMax;
  i = 0;
  for (JsonObject sensorsObj : sensorsArray)
  {
    if (i < numberOfSensors)
    {
      String sensorsAddress = sensorsObj["ADDRESS"];
      String sensorsScript = sensorsObj["SCRIPT"];
      String sensorsName = sensorsObj["NAME"];
      String sensorsSwitch = sensorsObj["SW"];
      float sensorsOffset = 0.0;
      if (sensorsObj.containsKey("OFFSET"))
        sensorsOffset = sensorsObj["OFFSET"];

      sensors[i].change(sensorsAddress, sensorsScript, sensorsName, sensorsOffset, sensorsSwitch);
      DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s Offset: %f SW: %s\n", (i + 1), sensorsName.c_str(), sensorsAddress.c_str(), sensorsScript.c_str(), sensorsOffset, sensorsSwitch.c_str());
      i++;
    }
    else
      sensors[i].change("", "", "", 0.0, "");
  }

  if (numberOfSensors == 0)
  {
    DEBUG_MSG("Sensors: %d\n", numberOfSensors);
  }
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
    String indGrafana = indObj["GRAF"];
    long indDelayOff = DEF_DELAY_IND; //default delay
    int indPowerLevel = 100;
    if (indObj.containsKey("PL"))
      indPowerLevel = indObj["PL"];

    if (indObj.containsKey("DELAY"))
      indDelayOff = indObj["DELAY"];

    inductionCooker.change(StringToPin(indPinWhite), StringToPin(indPinYellow), StringToPin(indPinBlue), indScript, indDelayOff, indEnabled, indPowerLevel, indGrafana);
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d\n", inductionStatus, indScript.c_str(), indPinWhite.c_str(), indPinYellow.c_str(), indPinBlue.c_str(), (indDelayOff / 1000), indPowerLevel);
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
    TickerDisp.config(DISP_UPDATE, 0);
    TickerDisp.start();
  }
  else
  {
    useDisplay = false;
    oledDisplay.dispEnabled = false;
    DEBUG_MSG("OLED Display: %d\n", oledDisplay.dispEnabled);
    TickerDisp.stop();
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

  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor / 1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction / 1000);

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

  if (miscObj["buzzer"] == "1")
  {
    startBuzzer = true;
    DEBUG_MSG("%s\n", "Buzzer activated");
  }
  else
  {
    startBuzzer = false;
    DEBUG_MSG("%s\n", "Buzzer disabled");
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

  TickerSen.config(SEN_UPDATE, 0);
  TickerAct.config(ACT_UPDATE, 0);
  TickerInd.config(IND_UPDATE, 0);

  if (numberOfSensors > 0)
    TickerSen.start();
  if (numberOfActors > 0)
    TickerAct.start();
  if (inductionCooker.isEnabled)
    TickerInd.start();

  DEBUG_MSG("Sensors update intervall: %d sec\n", (SEN_UPDATE / 1000));
  DEBUG_MSG("Actors update intervall: %d sec\n", (ACT_UPDATE / 1000));
  DEBUG_MSG("Induction update intervall: %d sec\n", (IND_UPDATE / 1000));

  if (miscObj.containsKey("STARTDB") == 1)
    startDB = miscObj["STARTDB"];
  else
    startDB = false;

  if (miscObj.containsKey("DBSERVER"))
    strlcpy(dbServer, miscObj["DBSERVER"], sizeof(dbServer));
  if (miscObj.containsKey("DB"))
    strlcpy(dbDatabase, miscObj["DB"], sizeof(dbDatabase));
  if (miscObj.containsKey("DBUSER"))
    strlcpy(dbUser, miscObj["DBUSER"], sizeof(dbUser));
  if (miscObj.containsKey("DBPASS"))
    strlcpy(dbPass, miscObj["DBPASS"], sizeof(dbPass));
  if (miscObj.containsKey("DBUP"))
    upInflux = miscObj["DBUP"];

  if (startDB)
  {
    DEBUG_MSG("InfluxDB Server URL %s User: %s Pass: %s Update %d\n", dbServer, dbUser, dbPass, upInflux);
  }
  else
  {
    DEBUG_MSG("InfluxDB Server: %d\n", startDB);
  }
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

  // Influx Datenbank
  if (startDB)
  {
    setInfluxDB();
    TickerInfluxDB.config(upInflux, 0);
    TickerInfluxDB.start();
  }
  else
    TickerInfluxDB.stop();

  if (startBuzzer)
    sendAlarm(ALARM_ON);

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
    actorsObj["GRAF"] = actors[i].getGrafana();
    DEBUG_MSG("Actor #: %d Name: %s MQTT: %s PIN: %s INV: %s SW: %s GRAF: %s\n", (i + 1), actors[i].name_actor.c_str(), actors[i].argument_actor.c_str(), PinToString(actors[i].pin_actor).c_str(), actors[i].getInverted().c_str(), actors[i].getSwitchable().c_str(), actors[i].getGrafana().c_str());
  }
  if (numberOfActors == 0)
  {
    DEBUG_MSG("Actors: %d\n", numberOfActors);
  }
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
    DEBUG_MSG("Sensor #: %d Name: %s Address: %s MQTT: %s Offset: %f SW: %s\n", (i + 1), sensors[i].sens_name.c_str(), sensors[i].getSens_adress_string().c_str(), sensors[i].sens_mqtttopic, sensors[i].sens_offset, sensors[i].getSwitchable().c_str());
  }
  if (numberOfSensors == 0)
    DEBUG_MSG("Sensors: %d\n", numberOfSensors);

  DEBUG_MSG("%s\n", "--------------------");

  // Write Induction
  JsonArray indArray = doc.createNestedArray("induction");
  if (inductionCooker.isEnabled)
  {
    inductionStatus = 1;
    JsonObject indObj = indArray.createNestedObject();
    indObj["PINWHITE"] = PinToString(inductionCooker.PIN_WHITE);
    indObj["PINYELLOW"] = PinToString(inductionCooker.PIN_YELLOW);
    indObj["PINBLUE"] = PinToString(inductionCooker.PIN_INTERRUPT);
    indObj["TOPIC"] = inductionCooker.mqtttopic;
    indObj["DELAY"] = inductionCooker.delayAfteroff;
    indObj["ENABLED"] = "1";
    indObj["PL"] = inductionCooker.powerLevelOnError;
    indObj["GRAF"] = inductionCooker.setGrafana;
    DEBUG_MSG("Induction: %d MQTT: %s Relais (WHITE): %s Command channel (YELLOW): %s Backchannel (BLUE): %s Delay after power off %d Power level on error: %d\n", inductionCooker.isEnabled, inductionCooker.mqtttopic.c_str(), PinToString(inductionCooker.PIN_WHITE).c_str(), PinToString(inductionCooker.PIN_YELLOW).c_str(), PinToString(inductionCooker.PIN_INTERRUPT).c_str(), (inductionCooker.delayAfteroff / 1000), inductionCooker.powerLevelOnError);
  }
  else
  {
    inductionStatus = 0;
    DEBUG_MSG("Induction: %d\n", inductionCooker.isEnabled);
  }
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
      // Display mit SDD1306 Chip:
      // display.ssd1306_command(SSD1306_DISPLAYON);

      // Display mit SH1106 Chip:
      display.SH1106_command(SH1106_DISPLAYON);
      cbpiEventSystem(EM_DISPUP);
    }
    else
    {
      displayObj["ENABLED"] = "0";
      oledDisplay.dispEnabled = false;
      useDisplay = false;
    }
    DEBUG_MSG("OLED display: %d Address: %s Update: %d\n", oledDisplay.dispEnabled, String(decToHex(oledDisplay.address, 2)).c_str(), (DISP_UPDATE / 1000));
    TickerDisp.config(DISP_UPDATE, 0);
    TickerDisp.start();
  }
  else
  {
    // Display mit SSD1306 Chip:
    // display.ssd1306_command(SSD1306_DISPLAYOFF);

    // Display mit SH1106 Chip:
    display.SH1106_command(SH1106_DISPLAYOFF);
    DEBUG_MSG("OLED display: %d\n", oledDisplay.dispEnabled);
    TickerDisp.stop();
  }
  DEBUG_MSG("%s\n", "--------------------");

  // Write Misc Stuff
  JsonArray miscArray = doc.createNestedArray("misc");
  JsonObject miscObj = miscArray.createNestedObject();

  miscObj["del_sen_act"] = wait_on_Sensor_error_actor;
  miscObj["del_sen_ind"] = wait_on_Sensor_error_induction;
  DEBUG_MSG("Wait on sensor error actors: %d sec\n", wait_on_Sensor_error_actor / 1000);
  DEBUG_MSG("Wait on sensor error induction: %d sec\n", wait_on_Sensor_error_induction / 1000);
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

  if (startBuzzer)
    miscObj["buzzer"] = "1";
  else
    miscObj["buzzer"] = "0";

  miscObj["mdns_name"] = nameMDNS;
  if (startMDNS)
    miscObj["mdns"] = "1";
  else
    miscObj["mdns"] = "0";

  miscObj["STARTDB"] = startDB;
  miscObj["DBSERVER"] = dbServer;
  miscObj["DB"] = dbDatabase;
  miscObj["DBUSER"] = dbUser;
  miscObj["DBPASS"] = dbPass;
  miscObj["DBUP"] = upInflux;
  if (startDB)
  {
    DEBUG_MSG("InfluxDB Server URL %s User: %s Pass: %s Update %d\n", dbServer, dbUser, dbPass, upInflux);
  }
  else
  {
    DEBUG_MSG("InfluxDB Server: %d\n", startDB);
  }

  miscObj["MQTTHOST"] = mqtthost;
  miscObj["upsen"] = SEN_UPDATE;
  miscObj["upact"] = ACT_UPDATE;
  miscObj["upind"] = IND_UPDATE;

  TickerSen.config(SEN_UPDATE, 0);
  TickerAct.config(ACT_UPDATE, 0);
  TickerInd.config(IND_UPDATE, 0);

  if (numberOfSensors > 0)
    TickerSen.start();
  else
    TickerSen.stop();
  if (numberOfActors > 0)
    TickerAct.start();
  else
    TickerAct.stop();
  if (inductionCooker.isEnabled)
    TickerInd.start();

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
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }

  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile)
  {
    DEBUG_MSG("%s\n", "Failed to open config file for writing");
    DEBUG_MSG("%s\n", "------ saveConfig aborted ------");
    if (startBuzzer)
      sendAlarm(ALARM_ERROR);
    return false;
  }
  serializeJson(doc, configFile);
  configFile.close();
  DEBUG_MSG("%s\n", "------ saveConfig finished ------");
  String Network = WiFi.SSID();
  DEBUG_MSG("ESP8266 device IP Address: %s\n", WiFi.localIP().toString().c_str());
  DEBUG_MSG("Configured WLAN SSID: %s\n", Network.c_str());
  DEBUG_MSG("%s\n", "---------------------------------");
  if (startBuzzer)
    sendAlarm(ALARM_ON);
  return true;
}
