void loop()
{
  server.handleClient();    // Webserver handle
  cbpiEventSystem(EM_WLAN); // Überprüfe WLAN
  cbpiEventSystem(EM_MQTT); // Überprüfe MQTT
  cbpiEventSystem(EM_MDNS); // MDNS handle
  
  if (timSen)              // Sensoren handle
  {
    cbpiEventSensors(sensorsStatus);
    timSen = false;
  }
  if (timAct)              // Aktoren handle
  {
    cbpiEventActors(actorsStatus);
    timAct = false;
  }
  if (timInd)              // Induktionskochfeld handle
  {
    cbpiEventInduction(inductionStatus);
    timInd = false;
  }

  if (timDisp)             // Display Update
  {
    cbpiEventSystem(EM_DISPUP);
    timDisp = false;
  }
  if (timTCP)              // TCP Server Update
  {
    cbpiEventSystem(EM_TCP);
    timTCP = false;
  }
  gEM.processAllEvents();
}
