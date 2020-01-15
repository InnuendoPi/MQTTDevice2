esptool.exe -cp COM3 -cd nodemcu -ce 
esptool.exe -cp COM3 -cd nodemcu -ca 0x000000 -cf ..\build\MQTTDevice2.ino.bin -ca 0x200000 -cf ..\build\MQTTDevice2.spiffs.bin
pause