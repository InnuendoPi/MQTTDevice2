class TCPServer
{
public:
    int temperature = 0;
    int target_temp = 0;    // Quelle MQTT
    int powerlevel = 0;     // Quelle Aktoren und Induktion
    String kettle_id = "0"; // Quelle Sensoren
    String tcpTopic;
    String ID = "";
    String name = "";

    TCPServer(String new_kettle_id)
    {
        change(new_kettle_id);
    }

    void change(const String &new_kettle_id)
    {
        if (kettle_id != new_kettle_id)
        {
            mqtt_unsubscribe();
            kettle_id = new_kettle_id;
            setTopic(kettle_id);
            mqtt_subscribe();
        }
    }
    void mqtt_subscribe()
    {
        if (pubsubClient.connected())
        {
            char subscribemsg[50];
            tcpTopic.toCharArray(subscribemsg, 50);
            DEBUG_MSG("TCP: Subscribing to %s\n", subscribemsg);
            pubsubClient.subscribe(subscribemsg);
        }
    }
    void mqtt_unsubscribe()
    {
        if (pubsubClient.connected())
        {
            char subscribemsg[50];
            tcpTopic.toCharArray(subscribemsg, 50);
            DEBUG_MSG("TCP: Unsubscribing from %s\n", subscribemsg);
            pubsubClient.unsubscribe(subscribemsg);
        }
    }

    void handlemqtt(char *payload)
    {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (const char *)payload);
        if (error)
        {
            DEBUG_MSG("TCP: handlemqtt deserialize Json error %s\n", error.c_str());
            return;
        }
        target_temp = doc["target_temp"];
    }
};

// Erstelle Array mit Anzahl maxSensoren == max Anzahl TCP
// kettle_id, name, id, act_temp, target_temp, powerlevel
TCPServer tcpServer[numberOfSensorsMax] = {
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0"),
    TCPServer("0")};

void publishTCP()
{
    for (int i = 1; i < numberOfSensorsMax; i++)
    {
        if (tcpServer[i].kettle_id == "0")
            continue;
        tcpClient.connect(tcpHost, tcpPort);
        if (tcpClient.connect(tcpHost, tcpPort))
        {
            StaticJsonDocument<256> doc;
            doc["name"] = tcpServer[i].name;
            doc["ID"] = tcpServer[i].ID;
            doc["temperature"] = tcpServer[i].temperature;
            doc["temp_units"] = "C";
            doc["RSSI"] = WiFi.RSSI();
            doc["interval"] = TCP_UPDATE;
            // Send additional but sensless data to act as an iSpindle device
            // json from iSpindle:
            // Input Str is now:{"name":"iSpindle","ID":1234567,"angle":22.21945,"temperature":15.6875,
            // "temp_units":"C","battery":4.207508,"gravity":1.019531,"interval":900,"RSSI":-59}

            doc["angle"] = tcpServer[i].target_temp;
            doc["battery"] = tcpServer[i].powerlevel;
            doc["gravity"] = 0;
            char jsonMessage[256];
            serializeJson(doc, jsonMessage);
            tcpClient.write(jsonMessage);
            //DEBUG_MSG("TCP: TCP message %s", jsonMessage);
        }
    }
}

void setTCPConfig()
{
    // Init TCP array
    // Das Array ist nicht lin aufsteigend sortiert
    // Das Array beginnt bei 1 mit der ersten ID aus MQTTPub (CBPi Plugin)
    // das Array Element 0 ist unbelegt
    for (int i = 0; i < numberOfSensors; i++)
    {
        if (sensors[i].kettle_id.toInt() > 0)
        {
            // Setze Kettle ID
            tcpServer[sensors[i].kettle_id.toInt()].kettle_id = sensors[i].kettle_id;
            // Setze MQTTTopic
            tcpServer[sensors[i].kettle_id.toInt()].tcpTopic = "MQTTDevice/kettle/" + sensors[i].kettle_id;
            tcpServer[sensors[i].kettle_id.toInt()].name = sensors[i].sens_name;
            tcpServer[sensors[i].kettle_id.toInt()].ID = SensorAddressToString(sensors[i].sens_address);
            tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);
        }
    }
    for (int i = 1; i < numberOfSensorsMax; i++)
    {
        if (tcpServer[i].kettle_id == "0")
            break;
        //DEBUG_MSG("TCP Server: %s Topic %s Powerlevel %d", tcpServer[i].kettle_id.c_str(), tcpServer[i].tcpTopic.c_str(), tcpServer[i].powerlevel);
    }
}

void setTopic(const String &id)
{
    tcpServer[id.toInt()].tcpTopic = "MQTTDevice/kettle/" + id;
    //DEBUG_MSG("TCP: topic %s set %s", id.c_str(), tcpServer[id.toInt()].tcpTopic.c_str());
}

void setTCPTemp(const String &id, const float &temp)
{
    tcpServer[id.toInt()].temperature = temp;
}

int getTCPTargetTemp(const String &id)
{
    if (id != "0")
        return tcpServer[id.toInt()].target_temp;
    else
        return 0;
}

void setTCPTemp()
{
    for (int i = 0; i < numberOfSensors; i++)
    {
        if (sensors[i].kettle_id.toInt() > 0)
            tcpServer[sensors[i].kettle_id.toInt()].temperature = (sensors[i].sens_value + sensors[i].sens_offset);
    }
}

void setTCPPowerAct(const String &id, const int &power)
{
    tcpServer[id.toInt()].powerlevel = power;
}
void setTCPPowerInd(const String &id, const int &power)
{
    tcpServer[id.toInt()].powerlevel = power;
}

void timerTCPCallback(void *pArg) // Timer Objekt Temperatur mit Pointer
{
	tickTCP = true; // Bei true wird im nächsten loop readTemperature ausgeführt
}