/*
Ideas:
  Indicator lights
  Alarm output and timeouts
*/
//GRBL Mega pins
#define FEED_HOLD 2 
#define START     3
#define SPINDLE   4

//inputs from torch
#define TORCH_UP_SIG    7
#define TORCH_DWN_SIG   8
#define TORCH_BUMP      9
#define TORCH_DRV_UP    10
#define TORCH_DRV_DWN   11

//state definitions
#define IDLE    0
#define PIERCE  1
#define CUTTING 2
#define RETRACT 3
int state=0;

unsigned long TORCH_RETRACT_TIME = 500;

void setup(){
  Serial.begin(115200);

  //Configure pins
  pinMode(FEED_HOLD, OUTPUT);
  pinMode(START, OUTPUT);
  pinMode(SPINDLE, INPUT_PULLUP);
  pinMode(TORCH_UP_SIG, INPUT_PULLUP);
  pinMode(TORCH_DWN_SIG, INPUT_PULLUP);
  pinMode(TORCH_BUMP, INPUT_PULLUP);
  pinMode(TORCH_DRV_UP, OUTPUT);
  pinMode(TORCH_DRV_DWN, OUTPUT);
}

void loop(){
  while(state==IDLE){
    if( digitalRead(SPINDLE) ){ state=PIERCE; }  
  }
  
  while(state==PIERCE){
    digitalWrite(FEED_HOLD,1);          //Pause machine motion, may need a dwell here
    digitalWrite(TORCH_DRV_DWN,1);
    while( !digitalRead(TORCH_BUMP) ){ }  //Leave motor running until we see a limit switch
    digitalWrite(TORCH_DRV_DWN,0);
    digitalWrite(TORCH_DRV_UP,1);
    delay(TORCH_RETRACT_TIME);
    digitalWrite(TORCH_DRV_UP,0);       //How is the torch being "lit"?
    digitalWrite(FEED_HOLD,0);          //Release machine hold, shouldn't move until it sees START
    digitalWrite(START,1);              //Resume motion
    delayMicroseconds(100);                   //Debounce, extend as necessary
    digitalWrite(START,0);
    state=CUTTING;
  }

  while(state==CUTTING){
    if( digitalRead(TORCH_UP_SIG) ){
      digitalWrite(TORCH_DRV_UP,1);
      while( digitalRead(TORCH_UP_SIG) && !digitalRead(SPINDLE) ){  }  
      digitalWrite(TORCH_DRV_UP,0);
    }
    if( digitalRead(TORCH_DWN_SIG) ){
      digitalWrite(TORCH_DRV_DWN,1);
      while(digitalRead(TORCH_UP_SIG) && !digitalRead(TORCH_BUMP) && !digitalRead(SPINDLE) ){  }  
      digitalWrite(TORCH_DRV_DWN,0);
      if(digitalRead(TORCH_BUMP) ){
        //handle torch hard limit
      }
    }
    if( !digitalRead(SPINDLE) ){
      state=RETRACT;
    }
  }
  
  while(state==RETRACT){
    //Turn of torch?
    digitalWrite(FEED_HOLD,1);          //Pause machine motion, may need a dwell here
    digitalWrite(TORCH_DRV_UP,1);
    while( !digitalRead(TORCH_BUMP) ){ }  //Leave motor running until we see a limit switch
    digitalWrite(TORCH_DRV_UP,0);
    digitalWrite(FEED_HOLD,0);          //Release machine hold, shouldn't move until it sees START
    digitalWrite(START,1);              //Resume motion
    delayMicroseconds(100);                   //Debounce, extend as necessary
    digitalWrite(START,0);
    state=IDLE;
  }
  
}