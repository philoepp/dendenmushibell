#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "ArduinoSIP.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h> 
#include <SoftwareSerial.h>


/* -------------------------------------------------------------------------- 
* DEFINES
---------------------------------------------------------------------------- */
#define RX_PIN 12
#define TX_PIN 13

// SIP parameters
const char *SipIP   = "192.168.178.1";  // IP of the FRITZ!Box
const int   SipPORT = 5060;             // SIP port of the FRITZ!Box
const char *SipUSER = "Dendenmushi";        // SIP-Call username at the FRITZ!Box
const char *SipPW   = "sipUserPW";      // SIP-Call password at the FRITZ!Box
// SIP registration parameters
const int SipEXPIRES   = 600;           // [s] Registration expires in 600s; renew after SipEXPIRES/2

// AP parameters
const char *AP_SSID = "DenDenMushi";    // SSID of the access point

/* -------------------------------------------------------------------------- 
Objects/variables
---------------------------------------------------------------------------- */
SoftwareSerial mp3Serial(RX_PIN, TX_PIN);
DFRobotDFPlayerMini myDFPlayer;
char acSipIn[2048];
char acSipOut[2048];
Sip aSip(acSipOut, sizeof(acSipOut));
char lastCallfrom[256];

/* -------------------------------------------------------------------------- 
* STATIC FUNCTION PROTOTYPES
---------------------------------------------------------------------------- */
void callCallback(const char * from);
void cancelCallback(const char * from);

/* -------------------------------------------------------------------------- 
* FUNCTIONS
---------------------------------------------------------------------------- */

void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  mp3Serial.begin(9600);

  Serial.println("Starting DenDenMushi Bell...");

  Serial.println("Setting up WiFi manager...");
  WiFiManager wm;

  bool res;
  res = wm.autoConnect(AP_SSID);

  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }

  // Initialize the DFplayer serial connection
  if (!myDFPlayer.begin(mp3Serial, true, true)) 
  {
    while(true)
    {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3

  // After connection of WiFi, register to SIP server
  Serial.println("WiFi connected; starting SIP");

  char WiFiIP[16];
  String ip = WiFi.localIP().toString();
  ip.toCharArray(WiFiIP, 16);

  aSip.Init(SipIP, SipPORT, WiFiIP, SipPORT, SipUSER, SipPW, SipEXPIRES, 15);
  aSip.Subscribe(); // register to receive call notification
  aSip.setCallCallback(callCallback);
  aSip.setCancelCallback(cancelCallback);
}

void loop()
{
  static unsigned long timer = millis();
  
  if (millis() - timer > 3000) {
    timer = millis();
    myDFPlayer.next();  //Play next mp3 every 3 second.
  }
}

/* -------------------------------------------------------------------------- 
* STATIC FUNCTIONS
---------------------------------------------------------------------------- */
void callCallback(const char * from)
{
  Serial.printf("Received a call from: %s\n", from);
  myDFPlayer.play(1);  //Play the ringtone
}

void cancelCallback(const char * from)
{
  Serial.printf("Call canceled from: %s\n", from);
}