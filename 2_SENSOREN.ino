class TemperatureSensor
{
public:
  char sens_mqtttopic[50];       // Für MQTT Kommunikation
  unsigned char sens_address[8]; // 1-Wire Adresse
  String sens_name;              // Name für Anzeige auf Website
  float sens_value = -127.0;     // Aktueller Wert
  bool sens_isConnected;         // check if Sensor is connected
  float sens_offset = 0.0;       // Offset
  bool sens_sw = false;          // Switchable
  bool sens_state = true;        // Error state sensor
  int sens_err = 0;
  String kettle_id = "0";

  String getSens_adress_string()
  {
    return SensorAddressToString(sens_address);
  }

  TemperatureSensor(String new_address, String new_mqtttopic, String new_name, float new_offset, String new_sw, String new_kettle_id)
  {
    change(new_address, new_mqtttopic, new_name, new_offset, new_sw, new_kettle_id);
  }

  void Update()
  {
    DS18B20.requestTemperatures();                        // new conversion to get recent temperatures
    sens_isConnected = DS18B20.isConnected(sens_address); // attempt to determine if the device at the given address is connected to the bus
    sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;

    if (!sens_isConnected && sens_address[0] != 0xFF && sens_address[0] != 0x00) // double check on !sens_isConnected. Billig Tempfühler ist manchmal für 1-2 loops nicht connected. 0xFF default address. 0x00 virtual test device (adress 00 00 00 00 00)
    {
      millis2wait(PAUSEDS18);                               // wait for approx 750ms before recheck connection
      sens_isConnected = DS18B20.isConnected(sens_address); // attempt to determine if the device at the given address is connected to the bus
      sens_isConnected ? sens_value = DS18B20.getTempC(sens_address) : sens_value = -127.0;
    }

    if (sens_value == 85.0)
    {                         // can be real 85 degrees or reset default temp or an error value eg cable too long
      millis2wait(PAUSEDS18); // wait for approx 750ms before request temp again
      DS18B20.requestTemperatures();
    }
    sensorsStatus = 0;
    sens_state = true;
    if (OneWire::crc8(sens_address, 7) != sens_address[7])
    {
      sensorsStatus = EM_CRCER;
      sens_state = false;
    }
    else if (sens_value == -127.00 || sens_value == 85.00)
    {
      if (sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor connected AND sensor address exists (not default FF)
        sensorsStatus = EM_DEVER;
        sens_state = false;
      }
      else if (!sens_isConnected && sens_address[0] != 0xFF)
      { // Sensor with valid address not connected
        sensorsStatus = EM_UNPL;
        sens_state = false;
      }
      else // not connected and unvalid address
      {
        sensorsStatus = EM_SENER;
        sens_state = false;
      }
    } // sens_value -127 || +85
    else
    {
      sensorsStatus = EM_OK;
      sens_state = true;
    }
    sens_err = sensorsStatus;
    publishmqtt();
  } // void Update

  void change(const String &new_address, const String &new_mqtttopic, const String &new_name, float new_offset, const String &new_sw, const String &new_kettle_id)
  {
    new_mqtttopic.toCharArray(sens_mqtttopic, new_mqtttopic.length() + 1);
    sens_name = new_name;
    sens_offset = new_offset;
    new_sw == "1" ? sens_sw = true : sens_sw = false;
    kettle_id = new_kettle_id;

    if (new_address.length() == 16)
    {
      char address_char[16];

      new_address.toCharArray(address_char, 17);

      char hexbyte[2];
      int octets[8];

      for (int d = 0; d < 16; d += 2)
      {
        // Assemble a digit pair into the hexbyte string
        hexbyte[0] = address_char[d];
        hexbyte[1] = address_char[d + 1];

        // Convert the hex pair to an integer
        sscanf(hexbyte, "%x", &octets[d / 2]);
        yield();
      }
      for (int i = 0; i < 8; i++)
      {
        sens_address[i] = octets[i];
      }
    }
    DS18B20.setResolution(sens_address, 10);
  }

  void publishmqtt()
  {
    if (pubsubClient.connected())
    {
      StaticJsonDocument<256> doc;
      JsonObject sensorsObj = doc.createNestedObject("Sensor");
      sensorsObj["Name"] = sens_name;
      if (sensorsStatus == 0)
      {
        sensorsObj["Value"] = (sens_value + sens_offset);
      }
      else
      {
        sensorsObj["Value"] = sens_value;
      }
      sensorsObj["Type"] = "1-wire";
      char jsonMessage[100];
      serializeJson(doc, jsonMessage);
      pubsubClient.publish(sens_mqtttopic, jsonMessage);
    }
  }
  char buf[5];
  char *getValueString()
  {
    //    char buf[5];
    dtostrf(sens_value, 2, 1, buf);
    return buf;
  }
  String getSwitchable()
  {
    if (sens_sw)
      return "1";
    else
      return "0";
  }
};

/* Initialisierung des Arrays */
TemperatureSensor sensors[numberOfSensorsMax] = {
    TemperatureSensor("", "", "", 0.0, "", "0"),
    TemperatureSensor("", "", "", 0.0, "", "0"),
    TemperatureSensor("", "", "", 0.0, "", "0"),
    TemperatureSensor("", "", "", 0.0, "", "0"),
    TemperatureSensor("", "", "", 0.0, "", "0"),
    TemperatureSensor("", "", "", 0.0, "", "0")};

/* Funktion für Loop */
void handleSensors()
{
  int max_status = 0;
  for (int i = 0; i < numberOfSensors; i++)
  {
    sensors[i].Update();

    // TCP Server
    if (startTCP)
    {
      if (sensors[i].kettle_id.toInt() > 0)
        setTCPTemp(sensors[i].kettle_id, (sensors[i].sens_value + sensors[i].sens_offset));
    }
    // get max sensorstatus
    if (sensors[i].sens_sw && max_status < sensors[i].sens_err)
      max_status = sensors[i].sens_err;
    //yield();
  }
  sensorsStatus = max_status;
}

unsigned char searchSensors()
{
  unsigned char i;
  unsigned char n = 0;
  unsigned char addr[8];

  while (oneWire.search(addr))
  {

    if (OneWire::crc8(addr, 7) == addr[7])
    {
      for (i = 0; i < 8; i++)
      {
        addressesFound[n][i] = addr[i];
      }
      n += 1;
    }
    yield();
  }
  return n;
  oneWire.reset_search();
}

String SensorAddressToString(unsigned char addr[8])
{
  char charbuffer[50];
  sprintf(charbuffer, "%02x%02x%02x%02x%02x%02x%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
  return charbuffer;
}

/* Funktionen für Web */

// Sensor wird geändert
void handleSetSensor()
{
  int id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfSensors;
    numberOfSensors += 1;
    if (numberOfSensors >= numberOfSensorsMax)
      return;
  }

  String new_mqtttopic = sensors[id].sens_mqtttopic;
  String new_name = sensors[id].sens_name;
  String new_address = sensors[id].getSens_adress_string();
  float new_offset = sensors[id].sens_offset;
  String new_sw = sensors[id].getSwitchable();
  String new_kettle_id = sensors[id].kettle_id;

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      new_name = server.arg(i);
    }
    if (server.argName(i) == "topic")
    {
      new_mqtttopic = server.arg(i);
    }
    if (server.argName(i) == "address")
    {
      new_address = server.arg(i);
    }
    if (server.argName(i) == "offset")
    {
      new_offset = formatDOT(server.arg(i));
    }
    if (server.argName(i) == "sw")
    {
      new_sw = server.arg(i);
    }
    if (server.argName(i) == "kettle_id")
    {
      if (isValidInt(server.arg(i)))
        new_kettle_id = server.arg(i);
      else
        new_kettle_id = "0";
    }
    yield();
  }

  sensors[id].change(new_address, new_mqtttopic, new_name, new_offset, new_sw, new_kettle_id);
  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelSensor()
{
  int id = server.arg(0).toInt();

  //  Alle einen nach vorne schieben
  for (int i = id; i < numberOfSensors; i++)
  {
    if (i == (numberOfSensorsMax - 1)) // 5 - Array von 0 bis (numberOfSensorsMax-1)
    {
      sensors[i].change("", "", "", 0.0, "", "0");
    }
    else
      sensors[i].change(sensors[i + 1].getSens_adress_string(), sensors[i + 1].sens_mqtttopic, sensors[i + 1].sens_name, sensors[i + 1].sens_offset, sensors[i + 1].getSwitchable(), sensors[i + 1].kettle_id);
  }

  // den letzten löschen
  numberOfSensors--;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handleRequestSensorAddresses()
{
  numberOfSensorsFound = searchSensors();
  int id = server.arg(0).toInt();
  String message;
  if (id != -1)
  {
    message += F("<option>");
    message += SensorAddressToString(sensors[id].sens_address);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfSensorsFound; i++)
  {
    message += F("<option>");
    message += SensorAddressToString(addressesFound[i]);
    message += F("</option>");
    yield();
  }
  server.send(200, "text/html", message);
}

void handleRequestSensors()
{
  StaticJsonDocument<1024> doc;
  JsonArray sensorsArray = doc.to<JsonArray>();

  for (int i = 0; i < numberOfSensors; i++)
  {
    JsonObject sensorsObj = doc.createNestedObject();
    sensorsObj["name"] = sensors[i].sens_name;
    sensorsObj["offset"] = sensors[i].sens_offset;
    sensorsObj["sw"] = sensors[i].getSwitchable();
    sensorsObj["state"] = sensors[i].sens_state;
    sensorsObj["kettle_id"] = sensors[i].kettle_id;
    if (sensors[i].sens_value != -127.0)
      sensorsObj["value"] = sensors[i].getValueString();
    else
    {
      if (sensors[i].sens_err == 1)
        sensorsObj["value"] = "CRC";
      if (sensors[i].sens_err == 2)
        sensorsObj["value"] = "DER";
      if (sensors[i].sens_err == 3)
        sensorsObj["value"] = "UNP";
      else
        sensorsObj["value"] = "ERR";
    }
    sensorsObj["mqtt"] = sensors[i].sens_mqtttopic;
    if (startTCP)
      sensorsObj["target_temp"] = getTCPTargetTemp(sensors[i].kettle_id);
    else
      sensorsObj["target_temp"] = "-1";
    yield();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestSensor()
{
  int id = server.arg(0).toInt();
  String request = server.arg(1);
  String message;

  if (id == -1)
  {
    message = "";
    goto SendMessage;
  }
  else
  {
    if (request == "name")
    {
      message = sensors[id].sens_name;
      goto SendMessage;
    }
    if (request == "offset")
    {
      message = sensors[id].sens_offset;
      goto SendMessage;
    }
    if (request == "sw")
    {
      message = sensors[id].getSwitchable();
      goto SendMessage;
    }
    if (request == "script")
    {
      message = sensors[id].sens_mqtttopic;
      goto SendMessage;
    }
    if (request == "kettle_id")
    {
      message = sensors[id].kettle_id;
      goto SendMessage;
    }
    message = "not found";
  }
  saveConfig();
SendMessage:
  server.send(200, "text/plain", message);
}

void timerSenCallback(void *pArg) // Timer Objekt Temperatur mit Pointer
{
  tickSen = true; // Bei true wird im nächsten loop readTemperature ausgeführt
}
