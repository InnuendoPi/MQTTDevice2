class induction
{
  unsigned long timeTurnedoff;

  long timeOutCommand = 5000;  // TimeOut für Seriellen Befehl
  long timeOutReaction = 2000; // TimeOut für Induktionskochfeld
  unsigned long lastInterrupt;
  unsigned long lastCommand;
  bool inputStarted = false;
  unsigned char inputCurrent = 0;
  unsigned char inputBuffer[33];
  bool isError = false;
  unsigned char error = 0;

  //int storePower = 0;
  long powerSampletime = 20000;
  unsigned long powerLast;
  long powerHigh = powerSampletime; // Dauer des "HIGH"-Anteils im Schaltzyklus
  long powerLow = 0;

public:
  unsigned char PIN_WHITE = 9;     // RELAIS
  unsigned char PIN_YELLOW = 9;    // AUSGABE AN PLATTE
  unsigned char PIN_INTERRUPT = 9; // EINGABE VON PLATTE
  int power = 0;
  int newPower = 0;
  unsigned char CMD_CUR = 0; // Aktueller Befehl
  boolean isRelayon = false; // Systemstatus: ist das Relais in der Platte an?
  boolean isInduon = false;  // Systemstatus: ist Power > 0?
  boolean isPower = false;
  String mqtttopic = "";
  boolean isEnabled = false;
  long delayAfteroff = 120000;
  int powerLevelOnError = 100;   // 100% schaltet das Event handling für Induktion aus
  int powerLevelBeforeError = 0; // in error event save last power state
  bool induction_state = true;   // Error state induction
  String kettle_id = "0";

  // MQTT Publish - not yet ready
  // char induction_mqtttopic[50];      // Für MQTT Kommunikation

  induction()
  {
    setupCommands();
  }

  void change(unsigned char pinwhite, unsigned char pinyellow, unsigned char pinblue, String topic, long delayoff, bool is_enabled, int powerLevel, String new_kettle_id)
  {
    if (isEnabled)
    {
      // aktuelle PINS deaktivieren
      if (isPin(PIN_WHITE))
      {
        digitalWrite(PIN_WHITE, HIGH);
        pins_used[PIN_WHITE] = false;
      }

      if (isPin(PIN_YELLOW))
      {
        digitalWrite(PIN_YELLOW, HIGH);
        pins_used[PIN_YELLOW] = false;
      }

      if (isPin(PIN_INTERRUPT))
      {
        //detachInterrupt(PIN_INTERRUPT);
        //pinMode(PIN_INTERRUPT, OUTPUT);
        digitalWrite(PIN_INTERRUPT, HIGH);
        pins_used[PIN_INTERRUPT] = false;
      }

      mqtt_unsubscribe();
    }

    // Neue Variablen Speichern

    PIN_WHITE = pinwhite;
    PIN_YELLOW = pinyellow;
    PIN_INTERRUPT = pinblue;

    mqtttopic = topic;
    delayAfteroff = delayoff;
    powerLevelOnError = powerLevel;
    induction_state = true;
    kettle_id = new_kettle_id;

    // MQTT Publish - not yet ready
    //mqtttopic.toCharArray(induction_mqtttopic, mqtttopic.length() + 1);

    isEnabled = is_enabled;

    if (isEnabled)
    {
      // neue PINS aktiveren
      if (isPin(PIN_WHITE))
      {
        pinMode(PIN_WHITE, OUTPUT);
        digitalWrite(PIN_WHITE, LOW);
        pins_used[PIN_WHITE] = true;
      }

      if (isPin(PIN_YELLOW))
      {
        pinMode(PIN_YELLOW, OUTPUT);
        digitalWrite(PIN_YELLOW, LOW);
        pins_used[PIN_YELLOW] = true;
      }

      if (isPin(PIN_INTERRUPT))
      {
        pinMode(PIN_INTERRUPT, INPUT_PULLUP);
        //attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), readInputWrap, CHANGE);
        pins_used[PIN_INTERRUPT] = true;
      }
      if (pubsubClient.connected())
      {
        mqtt_subscribe();
      }
    }
  }

  void mqtt_subscribe()
  {
    if (isEnabled)
    {
      if (pubsubClient.connected())
      {
        char subscribemsg[50];
        mqtttopic.toCharArray(subscribemsg, 50);
        DEBUG_MSG("Ind: Subscribing to %s\n", subscribemsg);
        pubsubClient.subscribe(subscribemsg);
      }
    }
  }

  void mqtt_unsubscribe()
  {
    if (pubsubClient.connected())
    {
      char subscribemsg[50];
      mqtttopic.toCharArray(subscribemsg, 50);
      DEBUG_MSG("Ind: Unsubscribing from %s\n", subscribemsg);
      pubsubClient.unsubscribe(subscribemsg);
    }
  }

  /*    //  Not yet ready
          // MQTT Publish
          void publishmqtt() {
            if (client.connected()) {
              StaticJsonBuffer<256> jsonBuffer;
              JsonObject& json = jsonBuffer.createObject();
              if (isInduon) {
                json["state"] = "on";
                json["power"] = String(power);
              }
              else
                json["state"] = "off";

              char jsonMessage[100];
              json.printTo(jsonMessage);
              client.publish(induction_mqtttopic, jsonMessage);
              DBG_PRINT("MQTT pub message: ");
              DBG_PRINTLN(jsonMessage);
            }
          }
    */

  void handlemqtt(char *payload)
  {
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, (const char *)payload);
    if (error)
    {
      DEBUG_MSG("Ind: handlemqtt deserialize Json error %s\n", error.c_str());
      return;
    }
    String state = doc["state"];
    if (state == "off")
    {
      newPower = 0;
      if (kettle_id.toInt() > 0)
        setTCPPowerInd(kettle_id, newPower);
      return;
    }
    else
    {
      newPower = doc["power"];
    }
    if (kettle_id.toInt() > 0)
      setTCPPowerInd(kettle_id, newPower);
  }

  void setupCommands()
  {
    for (int i = 0; i < 33; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        if (CMD[j][i] == 1)
        {
          CMD[j][i] = SIGNAL_HIGH;
        }
        else
        {
          CMD[j][i] = SIGNAL_LOW;
        }
      }
    }
  }

  bool updateRelay()
  {
    if (isInduon == true && isRelayon == false)
    { /* Relais einschalten */
      digitalWrite(PIN_WHITE, HIGH);
      return true;
    }

    if (isInduon == false && isRelayon == true)
    { /* Relais ausschalten */
      if (millis() > timeTurnedoff + delayAfteroff)
      {
        digitalWrite(PIN_WHITE, LOW);
        return false;
      }
    }

    if (isInduon == false && isRelayon == false)
    { /* Ist aus, bleibt aus. */
      return false;
    }

    return true; /* Ist an, bleibt an. */
  }

  void Update()
  {
    updatePower();

    isRelayon = updateRelay();

    if (isInduon && power > 0)
    {
      if (millis() > powerLast + powerSampletime)
      {
        powerLast = millis();
      }
      if (millis() > powerLast + powerHigh)
      {
        sendCommand(CMD[CMD_CUR - 1]);
        isPower = false;
      }
      else
      {
        sendCommand(CMD[CMD_CUR]);
        isPower = true;
      }
    }
    else if (isRelayon)
    {
      sendCommand(CMD[0]);
    }
  }

  void updatePower()
  {
    lastCommand = millis();

    if (power != newPower)
    { /* Neuer Befehl empfangen */

      if (newPower > 100)
      {
        newPower = 100; /* Nicht > 100 */
      }
      if (newPower < 0)
      {
        newPower = 0; /* Nicht < 0 */
      }
      //DBG_PRINT("Setting Power to ");
      //DBG_PRINTLN(newPower);
      power = newPower;

      timeTurnedoff = 0;
      isInduon = true;
      long difference = 0;

      if (power == 0)
      {
        CMD_CUR = 0;
        timeTurnedoff = millis();
        isInduon = false;
        difference = 0;
        goto setPowerLevel;
      }

      for (int i = 1; i < 7; i++)
      {
        if (power <= PWR_STEPS[i])
        {
          CMD_CUR = i;
          difference = PWR_STEPS[i] - power;
          goto setPowerLevel;
        }
      }

    setPowerLevel: /* Wie lange "HIGH" oder "LOW" */
      if (difference != 0)
      {
        powerLow = powerSampletime * difference / 20L;
        powerHigh = powerSampletime - powerLow;
      }
      else
      {
        powerHigh = powerSampletime;
        powerLow = 0;
      };
    }
  }

  void sendCommand(int command[33])
  {
    digitalWrite(PIN_YELLOW, HIGH);
    millis2wait(SIGNAL_START);
    digitalWrite(PIN_YELLOW, LOW);
    millis2wait(SIGNAL_WAIT);
    for (int i = 0; i < 33; i++)
    {
      digitalWrite(PIN_YELLOW, HIGH);
      delayMicroseconds(command[i]);
      digitalWrite(PIN_YELLOW, LOW);
      delayMicroseconds(SIGNAL_LOW);
    }
  }

  void readInput()
  {
    // Variablen sichern
    bool ishigh = digitalRead(PIN_INTERRUPT);
    unsigned long newInterrupt = micros();
    long signalTime = newInterrupt - lastInterrupt;

    // Glitch rausfiltern
    if (signalTime > 10)
    {

      if (ishigh)
      {
        lastInterrupt = newInterrupt; // PIN ist auf Rising, Bit senden hat gestartet :)
      }
      else
      { // Bit ist auf Falling, Bit Übertragung fertig. Auswerten.

        if (!inputStarted)
        { // suche noch nach StartBit.
          if (signalTime < 35000L && signalTime > 15000L)
          {
            inputStarted = true;
            inputCurrent = 0;
          }
        }
        else
        { // Hat Begonnen. Nehme auf.
          if (inputCurrent < 34)
          { // nur bis 33 aufnehmen.
            if (signalTime < (SIGNAL_HIGH + SIGNAL_HIGH_TOL) && signalTime > (SIGNAL_HIGH - SIGNAL_HIGH_TOL))
            {
              // HIGH BIT erkannt
              inputBuffer[inputCurrent] = 1;
              inputCurrent += 1;
            }
            if (signalTime < (SIGNAL_LOW + SIGNAL_LOW_TOL) && signalTime > (SIGNAL_LOW - SIGNAL_LOW_TOL))
            {
              // LOW BIT erkannt
              inputBuffer[inputCurrent] = 0;
              inputCurrent += 1;
            }
          }
          else
          { // Aufnahme vorbei.

            /* Auswerten */
            //newError = BtoI(13,4);          // Fehlercode auslesen.

            /* von Vorne */
            //timeLastReaction = millis();
            inputCurrent = 0;
            inputStarted = false;
          }
        }
      }
    }
  }

  unsigned long BtoI(int start, int numofbits)
  { //binary array to integer conversion
    unsigned long integer = 0;
    //   unsigned long mask=1;
    //   for (int i = numofbits+start-1; i >= start; i--) {
    //     if (inputBuffer[i]) integer |= mask;
    //     mask = mask << 1;
    //   }
    return integer;
  }
}

inductionCooker = induction();

void readInputWrap()
{
  inductionCooker.readInput();
}

/* Funktion für Loop */
void handleInduction()
{
  inductionCooker.Update();
}

void handleRequestInduction()
{
  StaticJsonDocument<256> doc;
  doc["enabled"] = inductionCooker.isEnabled;
  if (inductionCooker.isEnabled)
  {
    doc["relayOn"] = inductionCooker.isRelayon;
    doc["power"] = inductionCooker.power;
    doc["relayOn"] = inductionCooker.isRelayon;
    doc["state"] = inductionCooker.induction_state;
    doc["kettle_id"] = inductionCooker.kettle_id;
    doc["pl"] = inductionCooker.powerLevelOnError;
    if (inductionCooker.isPower)
    {
      doc["powerLevel"] = inductionCooker.CMD_CUR;
    }
    else
    {
      doc["powerLevel"] = max(0, inductionCooker.CMD_CUR - 1);
    }
  }
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleRequestIndu()
{
  String request = server.arg(0);
  String message;

  if (request == "isEnabled")
  {
    if (inductionCooker.isEnabled)
    {
      message = "1";
    }
    else
    {
      message = "0";
    }
    goto SendMessage;
  }
  if (request == "topic")
  {
    message = inductionCooker.mqtttopic;
    goto SendMessage;
  }
  if (request == "delay")
  {
    message = inductionCooker.delayAfteroff / 1000;
    goto SendMessage;
  }
  if (request == "pl")
  {
    message = inductionCooker.powerLevelOnError;
    goto SendMessage;
  }
  if (request == "kettle_id")
  {
    message = inductionCooker.kettle_id;
    goto SendMessage;
  }
  if (request == "pins")
  {
    int id = server.arg(1).toInt();
    unsigned char pinswitched;
    switch (id)
    {
    case 0:
      pinswitched = inductionCooker.PIN_WHITE;
      break;
    case 1:
      pinswitched = inductionCooker.PIN_YELLOW;
      break;
    case 2:
      pinswitched = inductionCooker.PIN_INTERRUPT;
      break;
    }
    if (isPin(pinswitched))
    {
      message += F("<option>");
      message += PinToString(pinswitched);
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
    goto SendMessage;
  }

SendMessage:
  server.send(200, "text/plain", message);
}

void handleSetIndu()
{
  unsigned char pin_white = inductionCooker.PIN_WHITE;
  unsigned char pin_blue = inductionCooker.PIN_INTERRUPT;
  unsigned char pin_yellow = inductionCooker.PIN_YELLOW;
  long delayoff = inductionCooker.delayAfteroff;
  bool is_enabled = inductionCooker.isEnabled;
  String topic = inductionCooker.mqtttopic;
  int pl = inductionCooker.powerLevelOnError;
  String new_kettle_id = inductionCooker.kettle_id;

  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "enabled")
    {
      if (server.arg(i) == "1")
      {
        is_enabled = true;
      }
      else
      {
        is_enabled = false;
      }
    }
    if (server.argName(i) == "topic")
    {
      topic = server.arg(i);
    }
    if (server.argName(i) == "pinwhite")
    {
      pin_white = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinyellow")
    {
      pin_yellow = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "pinblue")
    {
      pin_blue = StringToPin(server.arg(i));
    }
    if (server.argName(i) == "delay")
    {
      delayoff = server.arg(i).toInt() * 1000;
    }
    if (server.argName(i) == "pl")
    {
      if (isValidInt(server.arg(i)))
        pl = server.arg(i).toInt();
      else
        pl = 100;
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

  inductionCooker.change(pin_white, pin_yellow, pin_blue, topic, delayoff, is_enabled, pl, new_kettle_id);

  saveConfig();
}

void timerIndCallback(void *pArg) // Timer Objekt Temperatur mit Pointer
{
	if (inductionCooker.isEnabled)
    tickInd = true; // Bei true wird im nächsten loop readTemperature ausgeführt
}
