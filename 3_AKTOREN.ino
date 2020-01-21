class Actor
{
  unsigned long powerLast; // Zeitmessung für High oder Low
  bool isInverted = false;
  int dutycycle_actor = 5000;
  unsigned char OFF;
  unsigned char ON;

public:
  unsigned char pin_actor = 9; // the number of the LED pin
  String argument_actor;
  String name_actor;
  unsigned char power_actor;
  bool isOn;
  bool switchable;              // actors switchable on error events?
  bool isOnBeforeError = false; // isOn status before error event
  bool actor_state = true;      // Error state actor
  String kettle_id = "0";

  // MQTT Publish
  char actor_mqtttopic[50]; // Für MQTT Kommunikation

  Actor(String pin, String argument, String aname, String ainverted, String aswitchable, String akettle_id)
  {
    change(pin, argument, aname, ainverted, aswitchable, akettle_id);
  }

  void Update()
  {
    if (isPin(pin_actor))
    {
      if (isOn && power_actor > 0)
      {
        if (millis() > powerLast + dutycycle_actor)
        {
          powerLast = millis();
        }
        if (millis() > powerLast + (dutycycle_actor * power_actor / 100L))
        {
          digitalWrite(pin_actor, OFF);
        }
        else
        {
          digitalWrite(pin_actor, ON);
        }
      }
      else
      {
        digitalWrite(pin_actor, OFF);
      }
    }
  }

  void change(const String &pin, const String &argument, const String &aname, const String &ainverted, const String &aswitchable, const String &akettle_id)
  {
    // Set PIN
    if (isPin(pin_actor))
    {
      digitalWrite(pin_actor, HIGH);
      pins_used[pin_actor] = false;
      millis2wait(10);
    }

    pin_actor = StringToPin(pin);
    if (isPin(pin_actor))
    {
      pinMode(pin_actor, OUTPUT);
      digitalWrite(pin_actor, HIGH);
      pins_used[pin_actor] = true;
    }

    isOn = false;

    name_actor = aname;

    if (argument_actor != argument)
    {
      mqtt_unsubscribe();
      argument_actor = argument;
      mqtt_subscribe();

      // MQTT Publish - not yet ready
      // argument.toCharArray(actor_mqtttopic, argument.length() + 1);
    }
    if (ainverted == "1")
    {
      isInverted = true;
      ON = HIGH;
      OFF = LOW;
    }
    if (ainverted == "0")
    {
      isInverted = false;
      ON = LOW;
      OFF = HIGH;
    }
    aswitchable == "1" ? switchable = true : switchable = false;
    actor_state = true;
    isOnBeforeError = false;
    kettle_id = akettle_id;
  }

  /* MQTT Publish Not yet ready
  void publishmqtt() {
    if (client.connected()) {
      StaticJsonDocument<256> doc;
      JsonObject actorObj = doc.createNestedObject("Actor");
      if (isOn) {
        doc["State"] = "on";
        doc["power"] = String(power_actor);
      }
      else
        doc["State"] = "off";

      char jsonMessage[100];
      serializeJson(doc, jsonMessage);
      char new_argument_actor[50];
      new_argument_actor.toCharArray(argument_actor, new_argument_actor.length() + 1);
      client.publish(new_argument_actor, jsonMessage);
    }
  }
  */
  void mqtt_subscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Act: Subscribing to %s\n", subscribemsg);
      pubsubClient.subscribe(subscribemsg);
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      argument_actor.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Act: Unsubscribing from %s\n", subscribemsg);
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  void handlemqtt(char *payload)
  {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      DEBUG_MSG("Act: handlemqtt deserialize Json error %s\n", error.c_str());
      return;
    }
    String state = doc["state"];
    if (state == "off")
    {
      isOn = false;
      power_actor = 0;
      return;
    }
    if (state == "on")
    {
      int newpower = doc["power"];
      isOn = true;
      power_actor = min(100, newpower);
      power_actor = max(0, newpower);
      return;
    }
  }
  String getInverted()
  {
    if (isInverted)
      return "1";
    else
      return "0";
  }
  String getSwitchable()
  {
    if (switchable)
      return "1";
    else
      return "0";
  }
};

// Initialisierung des Arrays max 6
Actor actors[numberOfActorsMax] = {
    Actor("", "", "", "", "", "0"),
    Actor("", "", "", "", "", "0"),
    Actor("", "", "", "", "", "0"),
    Actor("", "", "", "", "", "0"),
    Actor("", "", "", "", "", "0"),
    Actor("", "", "", "", "", "0")};

// Funktionen für Loop im Timer Objekt
void handleActors()
{
  for (int i = 0; i < numberOfActors; i++)
  {
    actors[i].Update();
    // TCP Server
    if (startTCP)
    {
      if (actors[i].kettle_id.toInt() > 0)
        setTCPPowerAct(actors[i].kettle_id, actors[i].power_actor);
    }
    yield();
  }
}

/* Funktionen für Web */
void handleRequestActors()
{
  StaticJsonDocument<1024> doc;
  JsonArray actorsArray = doc.to<JsonArray>();

  for (int i = 0; i < numberOfActors; i++)
  {
    JsonObject actorsObj = doc.createNestedObject();

    actorsObj["name"] = actors[i].name_actor;
    actorsObj["status"] = actors[i].isOn;
    actorsObj["power"] = actors[i].power_actor;
    actorsObj["mqtt"] = actors[i].argument_actor;
    actorsObj["pin"] = PinToString(actors[i].pin_actor);
    actorsObj["sw"] = actors[i].switchable;
    actorsObj["state"] = actors[i].actor_state;
    actorsObj["kettle_id"] = actors[i].kettle_id;
    yield();
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestActor()
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
      message = actors[id].name_actor;
      goto SendMessage;
    }
    if (request == "script")
    {
      message = actors[id].argument_actor;
      goto SendMessage;
    }
    if (request == "pin")
    {
      message = PinToString(actors[id].pin_actor);
      goto SendMessage;
    }
    if (request == "inv")
    {
      message = actors[id].getInverted();
      goto SendMessage;
    }
    if (request == "sw")
    {
      message = actors[id].getSwitchable();
      goto SendMessage;
    }
    if (request == "kettle_id")
    {
      message = actors[id].kettle_id;
      goto SendMessage;
    }
    message = "not found";
  }
  saveConfig();
SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetActor()
{
  int id = server.arg(0).toInt();

  if (id == -1)
  {
    id = numberOfActors;
    numberOfActors += 1;
    if (numberOfActors >= numberOfActorsMax)
      return;
  }

  String ac_pin = PinToString(actors[id].pin_actor);
  String ac_argument = actors[id].argument_actor;
  String ac_name = actors[id].name_actor;
  String ac_isinverted = actors[id].getInverted();
  String ac_switchable = actors[id].getSwitchable();
  String ac_kettle_id = actors[id].kettle_id;

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "name")
    {
      ac_name = server.arg(i);
    }
    if (server.argName(i) == "pin")
    {
      ac_pin = server.arg(i);
    }
    if (server.argName(i) == "script")
    {
      ac_argument = server.arg(i);
    }
    if (server.argName(i) == "inv")
    {
      ac_isinverted = server.arg(i);
    }
    if (server.argName(i) == "sw")
    {
      ac_switchable = server.arg(i);
    }
    if (server.argName(i) == "kettle_id")
    {
      if (isValidInt(server.arg(i)))
        ac_kettle_id = server.arg(i);
      else
        ac_kettle_id = "0";
    }
    yield();
  }

  actors[id].change(ac_pin, ac_argument, ac_name, ac_isinverted, ac_switchable, ac_kettle_id);

  saveConfig();
  server.send(201, "text/plain", "created");
}

void handleDelActor()
{
  int id = server.arg(0).toInt();

  for (int i = id; i < numberOfActors; i++)
  {
    if (i == (numberOfActorsMax - 1)) // 5 - Array von 0 bis (numberOfActorsMax-1)
    {
      actors[i].change("", "", "", "", "", "");
    }
    else
    {
      actors[i].change(PinToString(actors[i + 1].pin_actor), actors[i + 1].argument_actor, actors[i + 1].name_actor, actors[i + 1].getInverted(), actors[i + 1].getSwitchable(), actors[i + 1].kettle_id);
    }
  }

  numberOfActors -= 1;
  saveConfig();
  server.send(200, "text/plain", "deleted");
}

void handlereqPins()
{
  int id = server.arg(0).toInt();
  String message;

  if (id != -1)
  {
    message += F("<option>");
    message += PinToString(actors[id].pin_actor);
    message += F("</option><option disabled>──────────</option>");
  }
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins_used[pins[i]] == false)
    {
      message += F("<option>");
      message += pin_names[i];
      message += F("</option>");
    }
    yield();
  }
  server.send(200, "text/plain", message);
}

unsigned char StringToPin(String pinstring)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pin_names[i] == pinstring)
    {
      return pins[i];
    }
  }
  return 9;
}

String PinToString(unsigned char pinbyte)
{
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      return pin_names[i];
    }
  }
  return "NaN";
}

bool isPin(unsigned char pinbyte)
{
  bool returnValue = false;
  for (int i = 0; i < numberOfPins; i++)
  {
    if (pins[i] == pinbyte)
    {
      returnValue = true;
      goto Ende;
    }
  }
Ende:
  return returnValue;
}
