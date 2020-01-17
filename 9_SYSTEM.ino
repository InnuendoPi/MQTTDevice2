void millis2wait(const int &value)
{
  unsigned long pause = millis();
  while (millis() < pause + value)
  {
    yield(); //wait approx. [period] ms
  }
}

// Prüfe WebIf Eingaben
float formatDOT(String str)
{
  str.replace(',', '.');
  if (isValidFloat(str))
    return str.toFloat();
  else
    return 0;
}

bool isValidInt(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (isdigit(str.charAt(i)))
      continue;
    if (str.charAt(i) == '.')
      return false;
    return false;
  }
  return true;
}

bool isValidFloat(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

bool isValidDigit(const String &str)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str.charAt(i) == '.')
      continue;
    if (isdigit(str.charAt(i)))
      continue;
    return false;
  }
  return true;
}

void setTimer()
{
  // Timer für Loop
  os_timer_setfn(&TimerSen, timerSenCallback, NULL);
  os_timer_arm(&TimerSen, SEN_UPDATE, true); // Zeitintervall Temperatursensoren in ms
  os_timer_setfn(&TimerAct, timerActCallback, NULL);
  os_timer_arm(&TimerAct, ACT_UPDATE, true); // Zeitintervall Aktoren in ms
  os_timer_setfn(&TimerInd, timerIndCallback, NULL);
  os_timer_arm(&TimerInd, IND_UPDATE, true); // Zeitintervall Induktion in ms
  os_timer_setfn(&TimerDisp, timerDispCallback, NULL);
  os_timer_arm(&TimerDisp, DISP_UPDATE, true); // Zeitintervall Display in ms
  os_timer_setfn(&TimerTCP, timerTCPCallback, NULL);
  os_timer_arm(&TimerTCP, TCP_UPDATE, true); // Zeitintervall TCPServer in ms
  // os_timer_setfn(&TimerNTP, timerNTPCallback, NULL);
  // os_timer_arm(&TimerNTP, NTP_INTERVAL, true); // Zeitintervall TCPServer in ms
  TimerNTP.attach(NTP_INTERVAL, tickerNTP );
}

void tickerMQTTER() // Ticker helper function calling Event MQTT Error
{
  cbpiEventSystem(EM_MQTTER);
}

void tickerWLANER() // Ticker helper function calling Event WLAN Error
{
  cbpiEventSystem(EM_WLANER);
}

void tickerNTP() // Ticker helper function calling Event WLAN Error
{
  timeClient.update();
}


String decToHex(unsigned char decValue, unsigned char desiredStringLength)
{
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength)
    hexString = "0" + hexString;

  return "0x" + hexString;
}

unsigned char convertCharToHex(char ch)
{
  unsigned char returnType;
  switch (ch)
  {
  case '0':
    returnType = 0;
    break;
  case '1':
    returnType = 1;
    break;
  case '2':
    returnType = 2;
    break;
  case '3':
    returnType = 3;
    break;
  case '4':
    returnType = 4;
    break;
  case '5':
    returnType = 5;
    break;
  case '6':
    returnType = 6;
    break;
  case '7':
    returnType = 7;
    break;
  case '8':
    returnType = 8;
    break;
  case '9':
    returnType = 9;
    break;
  case 'A':
    returnType = 10;
    break;
  case 'B':
    returnType = 11;
    break;
  case 'C':
    returnType = 12;
    break;
  case 'D':
    returnType = 13;
    break;
  case 'E':
    returnType = 14;
    break;
  case 'F':
    returnType = 15;
    break;
  default:
    returnType = 0;
    break;
  }
  return returnType;
}
