void loop()
{
  server.handleClient();    // Webserver handle
  cbpiEventSystem(EM_WLAN); // Check WLAN
  cbpiEventSystem(EM_MQTT); // Check MQTT
  cbpiEventSystem(EM_MDNS); // MDNS handle
  
  if (tickSen)              // Sensoren handle
  {
    cbpiEventSensors(sensorsStatus);
    tickSen = false;
  }
  if (tickAct)              // Aktoren handle
  {
    cbpiEventActors(actorsStatus);
    tickAct = false;
  }
  if (tickInd)              // Induktionskochfeld handle
  {
    cbpiEventInduction(inductionStatus);
    tickInd = false;
  }

  if (tickDisp)             // Display Update
  {
    cbpiEventSystem(EM_DISPUP);
    tickDisp = false;
  }
  if (tickTCP)              // TCP Server Update
  {
    cbpiEventSystem(EM_TCP);
    tickTCP = false;
  }
  if (tickNTP)              // NTP Update
  {
    cbpiEventSystem(EM_NTP); 
    tickNTP = false;
  }
  gEM.processAllEvents();
}
