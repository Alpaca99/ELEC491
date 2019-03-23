// arduinoFFT - Version: Latest 
#include <arduinoFFT.h>

#include <ezTime.h>

const int Pin1 = A0;


//#define CAYENNE_DEBUG 		// Uncomment to show Cayenne debug messages
#define CAYENNE_PRINT Serial	// Comment this out to disable prints and save space

//Libraries
#include <CayenneMQTTWiFi.h>	// Cayenne required libraries
#include "WiFi.h"				// Required library for Arduino Uno Wifi to connect to Google Sheets 
#include <NTPClient.h>			// Libraries for retrieving time from NTP server
#include <WiFiUdp.h>

//Variables for retrieving time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


arduinoFFT FFT = arduinoFFT();

float Voltage_Log[512]; // Log size = (max impact duration)0.4 sec * (sampling frequency)4000 sample per second 
float vImag[512];
float peaks[4];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

// Website and ID for API to push data to Google Sheets
const char WEBSITE[] = "api.pushingbox.com";	//pushingbox API server
const String devid = "v6D9FBC33D781722";		//device ID from Pushingbox 

// WiFi network info.
char ssid[] = "ubcvisitor";
char wifiPassword[] = ""; // No password required for the ubcvisitor network
// However, the device MAC address should be registered with UBC IT before accessing the network
// This is noted as a limited service, and devices that are inactive for extended periods of time are de-registered

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "7f6eead0-9b4e-11e8-a42a-3f9fb83051a4";
char password[] = "3607a8f057cc2ac996a72d9d846c7239b27f1aff";
char clientID[] = "e3a5ca00-1b69-11e9-898f-c12a468aadce";
int bird_impact;
int state;
int disco = 0;

// Wifi client for sending data to Google Sheets
WiFiClient client;  //Instantiate WiFi object


//Arduino initialization - Connect to Cayenne and the time server
void setup() {
	pinMode(Pin1, INPUT); 
	Serial.begin(115200);
	Cayenne.begin(username, password, clientID, ssid, wifiPassword);
	// Initial data push to Cayenne
	Cayenne.loop();
	// Time setup
	timeClient.begin();
	timeClient.setTimeOffset(-28800);
	bird_impact=0;
	state=0;
}

void loop() {
  
	//state 0 variables
	bool enter_state = true;
	uint32_t start_time;
	//state 1 variables
	float voltage;
	float voltage_scaner[50];
	int scaner_index = 0;
	float maxV=0;
	float minV=1023;
  float noiseFloor = 2;

  
	//Check whether state is normal
	if( state<0 || state>3 ){
		state = 0;
	}//Return state to 0 if state is abnormal.
  
  
	//State 0: wait until voltage get stable(time = 5 secend)
	while (state == 0){
		if(enter_state == true){
			start_time = millis();
			enter_state = false;
			Serial.println("State 0");
		}
	//Serial.println(millis());
		if((millis()-start_time) > 3000){
			  state = 1;
			  enter_state = true;
		}//leaving state
	}// End of state 0


	// State 1: start scanning sample
	while (state == 1){
    
		start_time = micros();
    
		if(enter_state == true){
			Serial.println("State 1");
			enter_state = false;
			scaner_index = 0;
		}
    
		// scan the voltage
		voltage = analogRead(Pin1);
		voltage_scaner[scaner_index] = voltage;
		// Serial.println(voltage_scaner[scaner_index]);
    
		//Check array for triggering voltage difference
		if(scaner_index == 49){
			//Serial.println("OK");
			maxV = voltage_scaner[0];
			minV = voltage_scaner[0];
			int index=0;
			while(index<50){
				//Serial.println(index);
				//Serial.println(voltage_scaner[index]);
				if(maxV<voltage_scaner[index]){
					maxV = voltage_scaner[index];
					//Serial.println(maxV);
				}
				if(minV>voltage_scaner[index]){
					minV = voltage_scaner[index];
					//Serial.println(minV);
				}
				index++;
			}
		}
    
		if(scaner_index<50){
			scaner_index ++;
		}
		else{
			scaner_index = 0;
		}
		
		//Threshold to trigger the impulse detection
		if(maxV-minV > noiseFloor*10){                
			Serial.println("Suspect!");
			enter_state = true;
			state = 2;
		}// leaving state 1
	  
		// loop time controller
		while(micros()-start_time<500){
			//Serial.println("Wait");
			//do nothing  
		}
	} // End of state1

  // State 2: Impact Verification
	while (state == 2){
		if(enter_state == true){
			enter_state = false;
			Serial.println("State 2");
		}
		//Serial.println(millis());
    bool flag = false;
		int index2 = 50;
		
		while(index2<512){
			start_time = micros();
			Voltage_Log[index2]=analogRead(Pin1);
			index2++;
			while(micros()-start_time<500){
				//500us
			}
		}
		
		int index3 = 0;
		while(index3<50){
			Voltage_Log[index3]=voltage_scaner[index3];
			index3++;
		}
    
		// Check for duration
		int pulse_count=0;
		for(int i=0;i<512;i++){
			Serial.println(Voltage_Log[i]);
			
			if(i%50 == 0){
				maxV = 0;
				minV = 1023;
			}
			else{
				maxV = max(maxV,Voltage_Log[i]);
				minV = min(minV,Voltage_Log[i]);
			}
			if( (maxV-minV > noiseFloor*4) && (i%50==49)){
				pulse_count++;
			}
		}
		Serial.println(pulse_count);
    if(pulse_count>3){
      flag = true; 
    }
    
    // Voltage Damping    
    
		//frequency analysis
		if(flag == true){
		  float sumV=0;
      for(int j=0;j<512;j++){
        sumV+=Voltage_Log[j];
      }
      sumV=sumV/512;
      for(int k=0;k<512;k++){
        Voltage_Log[k]=Voltage_Log[k]-sumV;
        vImag[k] = 0.0;
      }
      FFT.Windowing(Voltage_Log, 512, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
      FFT.Compute(Voltage_Log, vImag, 512, FFT_FORWARD); /* Compute FFT */
      FFT.ComplexToMagnitude(Voltage_Log, vImag, 512); /* Compute magnitudes */
      //PrintVector(Voltage_Log, (512 >> 1), SCL_FREQUENCY);
      Serial.println("Peaks:");
      peaks[0] = findPeaks(Voltage_Log, 256, 2000);
      peaks[1] = findPeaks(Voltage_Log, 256, 2000);
      peaks[2] = findPeaks(Voltage_Log, 256, 2000);
      peaks[3] = findPeaks(Voltage_Log, 256, 2000);
      
		  flag = peakRangeCheck(peaks[0],1);
		  flag = peakRangeCheck(peaks[1],1);
		  
		  if(flag == false){
		    flag = peakRangeCheck(peaks[0],2);
		    flag = peakRangeCheck(peaks[1],2);
		    flag = peakRangeCheck(peaks[2],1);
		    flag = peakRangeCheck(peaks[3],1);
		  }
		}
    
		if((flag == false)){ // back to state 1
			state = 1;
  		enter_state = true;
	  } // leaving state

		else{ // go to state 3
			state = 3;
			enter_state = true;
		} // leaving state
    
	// state=3;
	} // End of state 2

	while(state == 3){
		Serial.println("Bird Impact!");
		bird_impact++;
		Serial.println(bird_impact);
    
		timeClient.update();
		String location = "Marine_Drive_15";
  
		// Condition is if a new impact has been registered

		Cayenne.loop();
    
		//Start API service using our WiFi Client through PushingBox
		if (client.connect(WEBSITE, 80)){ 
			//Serial.println(client.getTime());
			// Send GET request for PushingBox, including timestamp and location
			client.print("GET /pushingbox?devid=" + devid
			+ "&timestamp=" + (String) timeClient.getFormattedDate()
			+ "&location="      + location
			);

			client.println(" HTTP/1.1"); 
			client.print("Host: ");
			client.println(WEBSITE);
			client.println("Connection: close");
			client.println();
        }
		else {
			Serial.println("Error sending to Sheets!\n");
        }
		state = 1;
   }
	
	delay(1000);
}//End of program


// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
	// Send impact data to Cayenne
	Cayenne.virtualWrite(0, bird_impact);
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
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

void PrintVector(float *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    float abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
  break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / 2000);
  break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * 2000) / 512);
  break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

float findPeaks(float *vD, uint16_t samples, float samplingFrequency)
{
	float maxY = 0;
	uint16_t IndexOfMaxY = 0;
	
	for(uint16_t i=0;i<samples;i++){
    if((i > 0) && (i < samples-1)){
			if(vD[i] > maxY){
				maxY=vD[i];
				IndexOfMaxY=i;
			}
    }
  }
  if((IndexOfMaxY>0) && (IndexOfMaxY<samples-1)){
    vD[IndexOfMaxY]=0;
    vD[IndexOfMaxY+1]=0;
    vD[IndexOfMaxY-1]=0;
  }
  float peakF = 0;
  peakF = ((IndexOfMaxY * 1.0 * 2000) / 512);
  Serial.print(peakF);
  Serial.println("Hz");
  return peakF;
}

bool peakRangeCheck(float peak, int primary)
{
  if(primary=1){
    if((peak<90) && (peak>60)){
      return true;
    }
    if((peak<190) && (peak>160)){
      return true;
    }
  }
  else{
    if((peak<130) && (peak>110)){
      return true;
    }
    if((peak<260) && (peak>240)){
      return true;
    }
  } 
  return false;
}