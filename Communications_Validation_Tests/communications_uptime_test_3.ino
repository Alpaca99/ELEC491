/*
This code is to test the communication reliability of the microcontroller to the two databases.
The two databases under test are:
1. MyDevices Cayenne
2. Google Sh

This code is adapted from code located at:
https://www.hackster.io/detox/transmit-esp8266-data-to-google-sheets-8fc617 retrieved January 18, 2019
https://cayenne.mydevices.com/cayenne/dashboard/arduino/pending//1 retrieved January 18, 2019

The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.
This code utilises the Arduino Uno Wifi Rev2 to connect to Cayenne and Google Sheets using Wifi.

*/

#define TENMINUTES (600*1000L) // 10 minute marker

// Cayenne required libraries
//#define CAYENNE_DEBUG       // Uncomment to show Cayenne debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneMQTTWiFi.h>

// Required library for Arduino Uno Wifi to connect to Google Sheets 
#include "WiFi.h"

// Libraries for retrieving time from NTP server
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Website and ID for API to push data to Google Sheets
const char WEBSITE[] = "api.pushingbox.com"; //pushingbox API server
const String devid = "v6D9FBC33D781722"; //device ID from Pushingbox 

// WiFi network info.
char ssid[] = "ubcvisitor";
char wifiPassword[] = ""; // No password required for the ubcvisitor network
// However, the device MAC address should be registered with UBC IT before accessing the network
// This is noted as a limited service, and devices that are inactive for extended periods of time are de-registered

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "7f6eead0-9b4e-11e8-a42a-3f9fb83051a4";
char password[] = "3607a8f057cc2ac996a72d9d846c7239b27f1aff";
char clientID[] = "e3a5ca00-1b69-11e9-898f-c12a468aadce";

// Impact storage
int impact = 0;
int previmpact = 0;

// Wifi client for sending data to Google Sheets
WiFiClient client;  //Instantiate WiFi object

void setup() {
  // Cayenne setup
	Serial.begin(9600);
	Cayenne.begin(username, password, clientID, ssid, wifiPassword);
	// Initial data push to Cayenne
	Cayenne.loop();
	// Time setup
	timeClient.begin();
	timeClient.setTimeOffset(-28800);
}

unsigned long last10Minutes;

void loop() {
  // Keeping the time client up to date to avoid issues
  timeClient.update();
  
  // Experimentally incrementing impacts
  if(millis()-last10Minutes>=TENMINUTES){ // Trigger impact detection every 10 minutes
    last10Minutes += TENMINUTES; // Incrementing trigger by 10 minutes
    // Serial message for impact detection, in case system is connected to a serial port
    Serial.println("Impact Detected!");
    Serial.println(timeClient.getFormattedDate());
    // Experimentally increment impact counter
    previmpact=impact;
    impact = impact+1;
  }

  // Dummy time sample to send
  String timestamp = "2019-02-14T00:00:00";
  // Dummy location sample to send
  String location = "sample_location";
  
  // Condition is if a new impact has been registered
  if(impact != previmpact){
    
  	Cayenne.loop();
    
      //Start API service using our WiFi Client through PushingBox
      if (client.connect(WEBSITE, 80))
        { 
          // Send GET request for PushingBox, including timestamp and location
           client.print("GET /pushingbox?devid=" + devid
         + "&timestamp=" + timeClient.getFormattedDate()
         + "&location="      + location
           );

        client.println(" HTTP/1.1"); 
        client.print("Host: ");
        client.println(WEBSITE);
        client.println("Connection: close");
        client.println();
        }
      else
        {
          Serial.println("Error sending to Sheets!\n");
        }
    previmpact = impact;
  }

}

// Default function for sending sensor data at intervals to Cayenne.
CAYENNE_OUT_DEFAULT()
{
	// Send impact data to Cayenne
	Cayenne.virtualWrite(0, impact);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
CAYENNE_IN_DEFAULT()
{
	CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

CAYENNE_CONNECTED()
{
	if(disco == 1){
		Cayenne.virtualWrite(0,bird_impact);
		disco = 0;
	}
}

CAYENNE_DISCONNECTED()
{
	Serial.println("Cayenne Disconnected! Error code:404");
	//Cayenne.begin(username, password, clientID, ssid, wifiPassword);
	//Cayenne.loop();
	Cayenne.virtualWrite(0,bird_impact);
	disco = 1;
}
