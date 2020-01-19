/*
   Name:		MQTTDevice
   Erstellt:	2020
   Author:	    Innuendo
   
   Sketch für ESP8266
   Kommunikation via MQTT mit CraftBeerPi v3

   Unterstützung für DS18B20 Sensoren
   Unterstützung für GPIO Aktoren
   Unterstützung für GGM Induktionskochfeld
   Unterstützung für "PWM" Steuerung mit GPIO (Heizstab)

   Unterstützung für Web Update
   Unterstützung für OLED Display 126x64 I2C (D1+D2)
*/

#include <OneWire.h>           // OneWire Bus Kommunikation
#include <DallasTemperature.h> // Vereinfachte Benutzung der DS18B20 Sensoren
#include <ESP8266WiFi.h>       // Generelle WiFi Funktionalität
#include <ESP8266WebServer.h>  // Unterstützung Webserver
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>  // WiFiManager zur Einrichtung
#include <DNSServer.h>    // Benötigt für WiFiManager
#include <PubSubClient.h> // MQTT Kommunikation
#include <FS.h>           // SPIFFS Zugriff
#include <ArduinoJson.h>  // Lesen und schreiben von JSON Dateien
#include <ESP8266mDNS.h>  // mDNS
#include <WiFiUdp.h>      // WiFi
#include <EventManager.h> // Eventmanager
#include <ArduinoOTA.h>   // OTA
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include <Ticker.h>

//#include <stdarg.h>
#include <CertStoreBearSSL.h>

extern "C"
{
#include "user_interface.h"
}

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

// Version
#define Version "2.0"

// Defines Pause
#define PAUSE1SEC 1000
#define PAUSE2SEC 2000
#define PAUSEDS18 750

// OneWire
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// WiFi und MQTT
ESP8266WebServer server(80);
WiFiManager wifiManager;
WiFiClient espClient;
WiFiClient tcpClient;
PubSubClient pubsubClient(espClient);
MDNSResponder mdns;
ESP8266HTTPUpdateServer httpUpdate;

// Induktion Signallaufzeiten
const int SIGNAL_HIGH = 5120;
const int SIGNAL_HIGH_TOL = 1500;
const int SIGNAL_LOW = 1280;
const int SIGNAL_LOW_TOL = 500;
const int SIGNAL_START = 25;
const int SIGNAL_START_TOL = 10;
const int SIGNAL_WAIT = 10;
const int SIGNAL_WAIT_TOL = 5;
#define DEF_DELAY_IND 120000 //default delay after power off induction

/*  Binäre Signale für Induktionsplatte */
int CMD[6][33] = {
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},  // Aus
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},  // P1
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},  // P2
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0},  // P3
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},  // P4
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}}; // P5
unsigned char PWR_STEPS[] = {0, 20, 40, 60, 80, 100};                                                     // Prozentuale Abstufung zwischen den Stufen
String errorMessages[10] = {
    "E0",
    "E1",
    "E2",
    "E3",
    "E4",
    "E5",
    "E6",
    "E7",
    "E8",
    "EC"};

bool pins_used[17];
const unsigned char numberOfPins = 9;
const unsigned char pins[numberOfPins] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const String pin_names[numberOfPins] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8"};

// Variablen
unsigned char numberOfSensors = 0; // Gesamtzahl der Sensoren
#define numberOfSensorsMax 6       // Maximale ANzahl an Sensoren
unsigned char addressesFound[numberOfSensorsMax][8];
unsigned char numberOfSensorsFound = 0;
unsigned char numberOfActors = 0; // Gesamtzahl der Aktoren
#define numberOfActorsMax 6       // Maximale Anzahl an Aktoren
// MQTT Server
char mqtthost[16];
// Set device name
int mqtt_chip_key = ESP.getChipId();
char mqtt_clientid[16]; // AP-Mode und Gerätename

// Zeitserver Einstellungen
#define NTP_OFFSET 60 * 60                // NTP in Sekunden
#define NTP_INTERVAL 60 * 1000            // NTP in ms
#define NTP_ADDRESS "europe.pool.ntp.org" // NTP change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// EventManager
EventManager gEM; //  Eventmanager
IPAddress aktIP;  //  Workaround IP changed

int SEN_UPDATE = 5000;  //  sensors update delay loop
int ACT_UPDATE = 5000;  //  actors update delay loop
int IND_UPDATE = 5000;  //  induction update delay loop
int DISP_UPDATE = 5000; //  NTP and display update
int TCP_UPDATE = 60000; //  TCP server Update interval

// Eventmanager
// System error events
#define EM_WLANER 1
#define EM_MQTTER 2

// System triggered events
#define EM_MQTTRES 10
#define EM_REBOOT 11

// System run & set events
#define EM_WLAN 20
#define EM_OTA 21
#define EM_MQTT 22
#define EM_MDNS 24
#define EM_NTP 25
#define EM_MDNSET 26
#define EM_MQTTCON 27
#define EM_MQTTSUB 28
#define EM_SETNTP 29
#define EM_DISPUP 30
#define EM_TCP 33
#define EM_LOG 35

// Sensor, actor and induction
#define EM_OK 0      // Normal mode
#define EM_CRCER 1   // Sensor CRC failed
#define EM_DEVER 2   // Sensor device error
#define EM_UNPL 3    // Sensor unplugged
#define EM_SENER 4   // Sensor all errors
#define EM_ACTER 10  // Actor error
#define EM_INDER 10  // Induction error
#define EM_ACTOFF 11 // Actor error
#define EM_INDOFF 11 // Induction error

// Event handling 
bool StopOnWLANError = false;         // Event handling für WLAN Fehler
bool StopOnMQTTError = false;         // Event handling für MQTT Fehler
unsigned long mqttconnectlasttry = 0; // Zeitstempel bei Fehler MQTT
unsigned long wlanconnectlasttry = 0; // Zeitstempel bei Fehler WLAN
bool mqtt_state = true;               // Status MQTT
bool wlan_state = true;               // Status WLAN

// Zeitintervall für Reconnects WLAN und MQTT
#define tickerWLAN 20        // für Ticker Objekt WLAN in Sekunden!
#define tickerMQTT 20        // für Ticker Objekt MQTT in Sekunden!

// Standard Verzögerungen für das Event handling
int wait_on_error_mqtt = 120000; // How long should device wait between tries to reconnect WLAN      - approx in ms
int wait_on_error_wlan = 120000; // How long should device wait between tries to reconnect WLAN      - approx in ms
int wait_on_Sensor_error_actor = 120000;     // How long should actors wait between tries to reconnect sensor    - approx in ms
int wait_on_Sensor_error_induction = 120000; // How long should induction wait between tries to reconnect sensor - approx in ms

// Start
bool startMDNS = true; // Standard mDNS Name ist ESP8266- mit mqtt_chip_key
char nameMDNS[16] = "MQTTDevice";
bool startTCP = false;
bool shouldSaveConfig = false; // WiFiManager

unsigned long lastSenAct = 0;      // Timestap actors on sensor error
unsigned long lastSenInd = 0;      // Timestamp induction on sensor error

int sensorsStatus = 0;
int actorsStatus = 0;
int inductionStatus = 0;

// TCP Server
int tcpPort = 9501; // TCP server Port
char tcpHost[16];   // TCP server IP

// FSBrowser
File fsUploadFile; // a File object to temporarily store the received file

// OLED Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128       // OLED display width, in pixels
#define SCREEN_HEIGHT 64       // OLED display height, in pixels
#define DISP_DEF_ADDRESS 0x3C  // Only used on init setup!
#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include "icons.h"
bool useDisplay = false;
#define SDL D1
#define SDA D2
#define numberOfAddress 2
const int address[numberOfAddress] = {0x3C, 0x3D};

// Timer Objekte
os_timer_t TimerSen;
os_timer_t TimerAct;
os_timer_t TimerInd;
os_timer_t TimerDisp;
os_timer_t TimerTCP;
// Ticker Objekte
Ticker TickerMQTT, TickerWLAN, TickerNTP;

bool timSen = false;
bool timAct = false;
bool timInd = false;
bool timDisp = false;
bool timTCP = false;

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.print("*** SYSINFO: MQTTDevice in AP mode ");
    Serial.println(WiFi.softAPIP());
    Serial.print("*** SYSINFO: Start configuration portal ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
}
