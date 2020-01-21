void loop()
{
  server.handleClient();    // Webserver handle
  cbpiEventSystem(EM_WLAN); // Überprüfe WLAN
  cbpiEventSystem(EM_MQTT); // Überprüfe MQTT
  cbpiEventSystem(EM_MDNS); // MDNS handle
  gEM.processAllEvents();

  if (numberOfSensors > 0)  // Sensoren Ticker
    TickerSen.update();
  if (numberOfActors > 0)   // Aktoren Ticker
    TickerAct.update();
  if (inductionStatus > 0)  // Induktion Ticker
    TickerInd.update();
  if (useDisplay)           // Display Ticker
    TickerDisp.update();
  if (startTCP)             // TCP Server Ticker
    TickerTCP.update();

  TickerNTP.update();       // NTP Ticker
}
