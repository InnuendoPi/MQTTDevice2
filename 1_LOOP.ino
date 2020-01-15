void loop()
{
  server.handleClient();    // Webserver handle
  cbpiEventSystem(EM_WLAN); // Check WLAN
  cbpiEventSystem(EM_MQTT); // Check MQTT
  cbpiEventSystem(EM_MDNS); // MDNS handle
  cbpiEventSystem(EM_NTP);  // NTP handle (NTP_INTERVAL)

  if (tickSen)
  {
    cbpiEventSensors(sensorsStatus); // Sensor handle
    tickSen = false;
  }
  if (tickAct)
  {
    cbpiEventActors(actorsStatus); // Actor handle
    tickAct = false;
  }
  if (tickInd)
  {
    cbpiEventInduction(inductionStatus); // Induction handle
    tickInd = false;
  }

  if (tickDisp)
  {
    cbpiEventSystem(EM_DISPUP); // Display Update
    tickDisp = false;
  }
  if (tickTCP)
  {
    cbpiEventSystem(EM_TCP); // TCP Server Update
    tickTCP = false;
  }
  gEM.processAllEvents();
}
