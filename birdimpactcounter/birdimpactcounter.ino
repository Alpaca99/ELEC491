const int Pin1 = A0;


void setup() {
  Serial.begin(1000000);
}

void loop() {
  
  //state 0 variables
  int state = 0;
  bool enter_state = true;
  uint32_t start_time;
  //state 1 variables
  float voltage;
  float voltage_scaner[50];
  int scaner_index = 0;
  float maxV=0;
  float minV=1023;
  
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
  }//End of state 0


  //State 1: start scaning sample
  while (state == 1){
    
    start_time = micros();
    
    if(enter_state == true){
      Serial.println("State 1");
      enter_state = false;
      scaner_index = 0;
    }
    
    //scan the voltage
    voltage = analogRead(Pin1);
    voltage_scaner[scaner_index] = voltage;
    Serial.println(voltage_scaner[scaner_index]);
    if(scaner_index<50){
      scaner_index ++;
    }
    else{
      scaner_index = 0;
    }
    
    //Check array for suspect voltage difference
    if(scaner_index == 49){
       Serial.println("OK");
      maxV = 300.00;
      minV = 600.00;
      for(int i=0;i<50;i++){
        Serial.println(i);
        if(maxV<voltage_scaner[i]){
          maxV = voltage_scaner[i];
        }
        if(minV>voltage_scaner[i]){
          minV = voltage_scaner[i];
        }
      }
      float diff=maxV-minV;
      Serial.println(diff);
    }
    
    if(maxV-minV > 1000){                        //Change the threshold difference here!!!
      Serial.println("Suspect!");
      enter_state = true;
      state = 2;
    }//leaving state 1
  
    //loop time controller
    while(micros()-start_time<2000){
      //Serial.println("Wait");
      //do nothing  
    }
  }//End of state1

  //State 2: wait until voltage get stable(time = 5 secend)
  while (state == 2){
    if(enter_state == true){
        enter_state = false;
        Serial.println("State 2");
    }
    
    //Serial.println(millis());

    float Voltage_Log[1600];//Log size = (max impact duration)0.4 sec * (sampling frequency)4000 sample per second 
    
    for(int i=50;i<1600;i++){
      start_time = micros();
      Voltage_Log[i]=analogRead(Pin1);
      while(micros()-start_time<250){
        //250us
      }
    }
    for (int i=0;i<50;i++){
      Voltage_Log[i]=voltage_scaner[i];
    }

    
    //Check for duration
    int pulse_count=0;
    for(int i=0;i<1600;i++){
      
      if(i%50 == 0){
        maxV = 0;
        minV = 1023;
      }
      else{
          maxV = max(maxV,Voltage_Log[i]);
          minV = min(minV,Voltage_Log[i]);
        }    
      if( maxV-minV > 100 && i%50==49){                        //Change the threshold difference here!!!
        pulse_count++;
      }
      
    }
    
    //Voltage Damping

    
    if((pulse_count<10)){//back to state 1
        state = 1;
        enter_state = true;
    }//leaving state

    else{//go to state 3
        state = 3;
        enter_state = true;
    }//leaving state
    
  }//End of state 2

   while(state == 3){
    Serial.println("Bird Impact!");
   }
   
   delay(1000);
}//End of program
