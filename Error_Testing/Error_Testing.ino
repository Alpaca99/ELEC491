/* #include <ezTime.h>
 */
#include <avr/wdt.h>
const int Pin1 = A0;
byte value = RSTCTRL.RSTFR;



//Arduino initialization - Connect to Cayenne and the time server
void setup() {
	byte value = RSTCTRL.RSTFR;
	pinMode(Pin1, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	Serial.println("Hello World");
	Serial.println("Hello World");
	int t = 0;	
	//Power-Up Errors
	//Check if Arduino Power was lost or reset
	/*
		Flags in the RSTCTRL.RSTFR (MCU Status Register) are changed upon powering
		the device via reset button, power outtage, watchdog reset, and
		brown-outs.
		Bits 7-6: Reserved
		Bit 5: UPDIRF - UPDI Reset Flag, bit is set if a UPDI reset occurs. 
			   The bit is reset by a Power-on, or by writing 1 to the flag
		Bit 4: SWRF - Software Reset Flag, bit is set if a software reset
			   occurs. The bit is reset by a Power-on, or by writing
			   1 to the flag
		Bit 3: WDRF - Watchdog Reset Flag, bit is set if a Watchdog system
			   reset occurs. The bit is reset by a Power-on, or by writing
			   1 to the flag
		Bit 2: EXTRF - External Reset Flag, bit is set if a external reset
			   occurred, like a physical button press. The bit is reset by 
			   a Power-on, or by writing 1 to the flag.
		Bit 1: BORF - Brown-Out Reset Flag, bit is set when a brown-out
			   occurs. Bit is reset by a Power-on, or by writing 1 to
			   the flag.
		Bit 0: PORF - Power-on Reset Flag, bit is set if a Power-on occurs.
			   Bit is only reset by writing 1 to the flag!
	*/

	//(((RSTCTRL.RSTFR >> 1) & 1 ) | ((RSTCTRL.RSTFR >> 0) & 1 ))
	if (RSTCTRL.RSTFR & (B00000010 | B00000001) ){
		//Brown-Out or Power-On reset occurred
		
		// What do we want to happen in the event of a power loss?
			// We need to re-load the number of impacts recorded 
			// and push that to cayenne. 
		// How can we tell if a Power-On reset was intentional
		// (setting up the device with 0 impacts) or truly a
		// power outtage?
		
		Serial.println("A Power-On Occurred!");
		Serial.println("RSTCTRL.RSTFR = ");
		Serial.println(value);
		
		
		while(t < 10){
			digitalWrite(LED_BUILTIN, HIGH);
			delay(1000);
			digitalWrite(LED_BUILTIN, LOW);
			delay(1000);
			t++;
		}
	}
 
	//(RSTCTRL.RSTFR & (1 << 2))
	//((RSTCTRL.RSTFR >> 2)& 1 )
	else if (RSTCTRL.RSTFR & B00000100){
		//Reset button or some software reset occurred
		
		
		Serial.println("A Reset Occurred!");
		Serial.println("RSTCTRL.RSTFR = ");
		Serial.println(value);
		while(t < 50){
			digitalWrite(LED_BUILTIN, HIGH);
			delay(100);
			digitalWrite(LED_BUILTIN, LOW);
			delay(100);
			t++;
		}
    
	}
	//((RSTCTRL.RSTFR >> 3)& 1 )
	else if (RSTCTRL.RSTFR & B00001000){
		//Watchdog reset occurred
		Serial.println("A Watchdog Reset Occurred!");
		Serial.println("RSTCTRL.RSTFR = ");
		Serial.println(value);
		while(t < 8){
			digitalWrite(LED_BUILTIN, HIGH);
			delay(1000);
			digitalWrite(LED_BUILTIN, LOW);
			delay(1000);
			t++;
		}
	}
	//Clear RSTCTRL.RSTFR registers 
	//RSTCTRL.RSTFR = 0xFF;
	t = 0; 
 
	Serial.println(RSTCTRL.RSTFR);
	RSTCTRL.RSTFR = RSTCTRL.RSTFR & 0xFF;
	//RSTCTRL.SWRR = RSTCTRL.SWRR & 0x00;
	Serial.println(RSTCTRL.RSTFR);
}

void loop() {
	
 
}//End of program
