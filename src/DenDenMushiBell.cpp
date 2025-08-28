#include <FS.h>
#include "Arduino.h"
#include "ArduinoSIP.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h> 
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"


/* -------------------------------------------------------------------------- 
* DEFINES
---------------------------------------------------------------------------- */

// SIP parameters
const char *SipIP   = "192.168.178.1";  // IP of the FRITZ!Box
const int   SipPORT = 5060;             // SIP port of the FRITZ!Box
const char *SipUSER = "Dendenmushi";        // SIP-Call username at the FRITZ!Box
const char *SipPW   = "sipUserPW";      // SIP-Call password at the FRITZ!Box
// SIP registration parameters
const int SipEXPIRES   = 600;           // [s] Registration expires in 600s; renew after SipEXPIRES/2

// AP parameters
const char *AP_SSID = "DenDenMushi";    // SSID of the access point

// I2S pins are fixed on ESP8266
// BCLK = GPIO15 (D8)
// LRC = GPIO2 (D4)
// DIN = GPIO3 (RX pin)

/* -------------------------------------------------------------------------- 
Objects/variables
---------------------------------------------------------------------------- */
char acSipIn[2048];
char acSipOut[2048];
Sip aSip(acSipOut, sizeof(acSipOut));
char lastCallfrom[256];

/* -------------------------------------------------------------------------- 
* STATIC FUNCTION PROTOTYPES
---------------------------------------------------------------------------- */
void callCallback(const char * from);
void cancelCallback(const char * from);
void playMp3File();

/* -------------------------------------------------------------------------- 
* FUNCTIONS
---------------------------------------------------------------------------- */

void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  Serial.begin(115200);
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

  // After connection of WiFi, register to SIP server
  Serial.println("WiFi connected; starting SIP");

  char WiFiIP[16];
  String ip = WiFi.localIP().toString();
  ip.toCharArray(WiFiIP, 16);

  Serial.print("IP:");
  Serial.println(ip);

  aSip.Init(SipIP, SipPORT, WiFiIP, SipPORT, SipUSER, SipPW, SipEXPIRES, 15);
  aSip.Subscribe(); // register to receive call notification
  aSip.setCallCallback(callCallback);
  aSip.setCancelCallback(cancelCallback);

  // Setup filesystem
  SPIFFS.begin();
}

void loop()
{
  // SIP processing incoming packets
  aSip.loop(acSipIn, sizeof(acSipIn)); // handels incoming calls and reregisters
}

/* -------------------------------------------------------------------------- 
* STATIC FUNCTIONS
---------------------------------------------------------------------------- */
void callCallback(const char * from)
{
  Serial.printf("Received a call from: %s\n", from);
  playMp3File();
}

void cancelCallback(const char * from)
{
  Serial.printf("Call canceled from: %s\n", from);
}

void playMp3File() 
{
  AudioFileSourceSPIFFS file("/ringtone.mp3");
  AudioOutputI2S out;
  AudioGeneratorMP3 mp3;

  mp3.begin(&file, &out);

  while (mp3.isRunning()) 
  {
    if (!mp3.loop()) 
    {
      mp3.stop();
    }
  }
}