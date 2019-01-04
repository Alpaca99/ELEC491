const int signalPin = 0;
unsigned long time;

void setup()
{
  Serial.begin(57600);
}

int i=0;

void loop()
{
  float voltage;

  voltage = getVoltage(signalPin);

if( i<5000 ){
  Serial.print(voltage);
  Serial.print("\n");
  i++;
  Serial.print("Time: ");
  time = millis();
  Serial.print();
}

//  Serial.println(voltage);
  delayMicroseconds(100); // Sampling rate: 10000Hz
}


float getVoltage(int pin)
{
 
  return (analogRead(pin) * 0.004882814*6*0.1);

  // This equation converts the 0 to 1023 value that analogRead()
  // returns, into a 0.0 to 5.0 value that is the true voltage
  // being read at that pin.
}
