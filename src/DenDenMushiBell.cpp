#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#define FPSerial Serial1

DFRobotDFPlayerMini myDFPlayer;

// https://github.com/RalfJL/ESP32-SIP-CallMonitor

void setup()
{
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/10, /*tx =*/9);

  Serial.begin(9600);

  if (!myDFPlayer.begin(FPSerial, true, true)) 
  {
    while(true)
    {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3
}

void loop()
{
  static unsigned long timer = millis();
  
  if (millis() - timer > 3000) {
    timer = millis();
    myDFPlayer.next();  //Play next mp3 every 3 second.
  }
}