void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); //Redirect to our html web page
  server.send(302, "text/plain", "");
}

void handleWebRequests()
{
  if (loadFromSpiffs(server.uri()))
  {
    return;
  }
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

bool loadFromSpiffs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";

  if (!SPIFFS.exists(path.c_str()))
  {
    return false;
  }
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }
  dataFile.close();
  return true;
}

void mqttcallback(char *topic, unsigned char *payload, unsigned int length)
{
  // DEBUG_MSG("Web: Received MQTT Topic: %s ", topic);
  // Serial.print("Web: Payload: ");
  // for (int i = 0; i < length; i++)
  // {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println(" ");
  char payload_msg[length];
  for (int i = 0; i < length; i++)
  {
    payload_msg[i] = payload[i];
  }

  if (inductionCooker.mqtttopic == topic)
  {
    if (inductionCooker.induction_state)
      inductionCooker.handlemqtt(payload_msg);
    else
      DEBUG_MSG("%s\n", "*** Verwerfe MQTT wegen Status Induktion (Event handling)");
  }

  if (startDB)
  {
    for (int i = 0; i < numberOfDBMax; i++)
    {
      if (dbInflux[i].kettle_topic == topic)
        dbInflux[i].handlemqtt(payload_msg);
    }
  }

  for (int i = 0; i < numberOfActors; i++)
  {
    if (actors[i].argument_actor == topic)
    {
      if (actors[i].actor_state)
        actors[i].handlemqtt(payload_msg);
      else
        DEBUG_MSG("%s\n", "*** Verwerfe MQTT wegen Status Aktoren (Event handling)");
    }
  }
}

void handleRequestMiscSet()
{
  StaticJsonDocument<512> doc;

  doc["MQTTHOST"] = mqtthost;
  doc["del_sen_act"] = wait_on_Sensor_error_actor / 1000;
  doc["del_sen_ind"] = wait_on_Sensor_error_induction / 1000;
  doc["enable_mqtt"] = StopOnMQTTError;
  doc["enable_wlan"] = StopOnWLANError;
  doc["mqtt_state"] = oledDisplay.mqttOK; // Anzeige MQTT Status -> mqtt_state verzögerter Status!
  doc["wlan_state"] = oledDisplay.wlanOK;
  doc["delay_mqtt"] = wait_on_error_mqtt / 1000;
  doc["delay_wlan"] = wait_on_error_wlan / 1000;
  doc["startdb"] = startDB;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestMisc()
{
  String request = server.arg(0);
  String message;
  if (request == "MQTTHOST")
  {
    message = mqtthost;
    goto SendMessage;
  }
  if (request == "mdns_name")
  {
    message = nameMDNS;
    goto SendMessage;
  }
  if (request == "mdns")
  {
    if (startMDNS)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_mqtt")
  {
    if (StopOnMQTTError)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "enable_wlan")
  {
    if (StopOnWLANError)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "delay_mqtt")
  {
    message = wait_on_error_mqtt / 1000;
    goto SendMessage;
  }
  if (request == "delay_wlan")
  {
    message = wait_on_error_wlan / 1000;
    goto SendMessage;
  }
  if (request == "del_sen_act")
  {
    message = wait_on_Sensor_error_actor / 1000;
    goto SendMessage;
  }
  if (request == "del_sen_ind")
  {
    message = wait_on_Sensor_error_induction / 1000;
    goto SendMessage;
  }
  if (request == "upsen")
  {
    message = SEN_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upact")
  {
    message = ACT_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "upind")
  {
    message = IND_UPDATE / 1000;
    goto SendMessage;
  }
  if (request == "firmware")
  {
    if (startMDNS)
    {
      message = nameMDNS;
      message += " V";
    }
    else
      message = "MQTTDevice V ";
    message += Version;
    goto SendMessage;
  }
  if (request == "dbserver")
  {
    message = dbServer;
    goto SendMessage;
  }
  if (request == "startdb")
  {
    if (startDB)
      message = "1";
    else
      message = "0";
    goto SendMessage;
  }
  if (request == "dbdatabase")
  {
    message = dbDatabase;
    goto SendMessage;
  }
  if (request == "dbuser")
  {
    message = dbUser;
    goto SendMessage;
  }
  if (request == "dbpass")
  {
    message = dbPass;
    goto SendMessage;
  }
  if (request == "dbup")
  {
    message = (upInflux / 1000);
    goto SendMessage;
  }
  if (request == "dburl")
  {
    File urlFile = SPIFFS.open("/urlChart.txt", "r");
    if (!urlFile)
    {
      DEBUG_MSG("%s\n", "Failed to open urlFile\n");
      message = "about:blank";
    }
    else
    {
      String line = "";
      while (urlFile.available())
      {
        line += char(urlFile.read());
      }
      urlFile.close();
      message = line;
    }
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetMisc()
{
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "reset")
    {
      if (server.arg(i) == "1")
      {
        WiFi.disconnect();
        wifiManager.resetSettings();
        delay(PAUSE2SEC);
        ESP.reset();
      }
    }
    if (server.argName(i) == "clear")
    {
      if (server.arg(i) == "1")
      {
        SPIFFS.remove("/config.txt");
        WiFi.disconnect();
        wifiManager.resetSettings();
        delay(PAUSE2SEC);
        ESP.reset();
      }
    }
    if (server.argName(i) == "MQTTHOST")
      server.arg(i).toCharArray(mqtthost, 16);
    if (server.argName(i) == "mdns_name")
    {
      server.arg(i).toCharArray(nameMDNS, 16);
      checkChars(nameMDNS);
    }
    if (server.argName(i) == "mdns")
    {
      if (server.arg(i) == "1")
        startMDNS = true;
      else
        startMDNS = false;
    }
    if (server.argName(i) == "enable_mqtt")
    {
      if (server.arg(i) == "1")
        StopOnMQTTError = true;
      else
        StopOnMQTTError = false;
    }
    if (server.argName(i) == "delay_mqtt")
      if (isValidInt(server.arg(i)))
      {
        wait_on_error_mqtt = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "enable_wlan")
    {
      if (server.arg(i) == "1")
        StopOnWLANError = true;
      else
        StopOnWLANError = false;
    }
    if (server.argName(i) == "delay_wlan")
      if (isValidInt(server.arg(i)))
      {
        wait_on_error_wlan = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_act")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_actor = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "del_sen_ind")
      if (isValidInt(server.arg(i)))
      {
        wait_on_Sensor_error_induction = server.arg(i).toInt() * 1000;
      }
    if (server.argName(i) == "upsen")
    {
      if (isValidInt(server.arg(i)))
      {
        int newsup = server.arg(i).toInt();
        if (newsup > 0)
          SEN_UPDATE = newsup * 1000;
      }
    }
    if (server.argName(i) == "upact")
    {
      if (isValidInt(server.arg(i)))
      {
        int newaup = server.arg(i).toInt();
        if (newaup > 0)
          ACT_UPDATE = newaup * 1000;
      }
    }
    if (server.argName(i) == "upind")
    {
      if (isValidInt(server.arg(i)))
      {
        int newiup = server.arg(i).toInt();
        if (newiup > 0)
          IND_UPDATE = newiup * 1000;
      }
    }
    if (server.argName(i) == "dbserver")
    {
      server.arg(i).toCharArray(dbServer, 30);
      checkChars(dbServer);
    }
    if (server.argName(i) == "startdb")
    {
      if (server.arg(i) == "1")
        startDB = true;
      else
        startDB = false;
    }
    if (server.argName(i) == "dbdatabase")
    {
      server.arg(i).toCharArray(dbDatabase, 15);
      checkChars(dbDatabase);
    }
    if (server.argName(i) == "dbuser")
    {
      server.arg(i).toCharArray(dbUser, 15);
      checkChars(dbUser);
    }
    if (server.argName(i) == "dbpass")
    {
      server.arg(i).toCharArray(dbPass, 15);
      checkChars(dbPass);
    }
    if (server.argName(i) == "dbup")
    {
      if (isValidInt(server.arg(i)))
      {
        upInflux = server.arg(i).toInt() * 1000;
      }
    }
    if (server.argName(i) == "dburl")
    {
      DEBUG_MSG("server.arg: %s\n", server.arg(i).c_str());
      File urlFile = SPIFFS.open("/urlChart.txt", "w");
      String line = server.arg(i);
      line.replace("!", "&"); // Ersetzen von &-Zeichen in index.html rückgängig machen
      int bytesWritten = urlFile.print(line);
      urlFile.close();
    }
    yield();
  }
  saveConfig();
}

// Some helper functions WebIf
void rebootDevice()
{
  cbpiEventSystem(EM_REBOOT);
}
