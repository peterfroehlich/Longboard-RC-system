 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define potPin 0

int ledpin=8 ;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Single radio pipe address for the 2 nodes to communicate.
//const uint64_t pipe = 0xE8E8F0F0E1LL;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

char telemetric_data[10] = "";
//int telemetric_data[2] = { 0, 0 };

int throttle = 0;
int filterStep = 5;
float targetSpeed;
float currentSpeed;
int pwmSpeed;
int triggerCenterValue;

// Trigger Values. Better: Calibrate trigger routine. 
int throttle_max = 280;
int throttle_mid = 510;
int throttle_min = 688;
int throttle_resolution = throttle_min - throttle_max;
int servo_resolution = 180;

unsigned int send_error_counter = 0;


bool DEBUG = true;
long debug_time = millis(); 
 

void initilize_radio() {
  radio.begin();
  
  // This simple sketch opens a single pipe for these two nodes to communicate
  // back and forth.  One listens on it, the other talks to it.
  //radio.openWritingPipe(pipe);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
      
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(16);
  
  radio.enableAckPayload();
  
  radio.startListening();
  
  if (DEBUG) { radio.printDetails(); }
}

 
void setup()   {
  if (DEBUG) { 
    Serial.begin(57600);
    printf_begin();
  }
  
  pinMode(ledpin, OUTPUT);
  initilize_radio();
}
 

 
bool send_throttle() {
    radio.stopListening();

    if (DEBUG) { 
      printf("Sending value %d - ",throttle);
    }
    
    bool ok = radio.write( &throttle, sizeof(unsigned int));
    
    if (ok) {
      if ( DEBUG ) { printf("send ok - "); }
      if (send_error_counter > 0) { --send_error_counter; }
    } else {
      if ( DEBUG ) { printf("send failed!!!! - "); }
      if (send_error_counter < 10) { ++send_error_counter; }
    }
   
    // Now, continue listening
    radio.startListening(); 
    
    radio.read(&telemetric_data,sizeof(telemetric_data));
    printf("%s\r\n", telemetric_data);
    
    return ok;
}




int filterInput(int input) {
  // Kickdown rule? targetspeed über 100 mehr als currentspeed, grössere incremente? 
  if ((input % 10) >= 5) {
    targetSpeed = input + 10 - (input % 10);
  } else if ((input % 10) < 5) {
    targetSpeed = input - (input % 10);
  }
  
  if (targetSpeed < currentSpeed) {           //Accelerating: Slowly step up
    if (currentSpeed > triggerCenterValue) {  // If coming out of brake
      currentSpeed = targetSpeed;
    } else {
      currentSpeed -= filterStep;
    }
  } else if (targetSpeed > currentSpeed) {   //Decelerating: Quickly step down
    currentSpeed = targetSpeed;
  }
  
  // Rundungsfehler! Alles auf float umstellen, am ende runden... 
  //pwmSpeed = servo_resolution - ( (currentSpeed - throttle_max) / ( throttle_resolution / servo_resolution) );
  pwmSpeed = map(currentSpeed, throttle_min, throttle_max, 0, 179);
 
  if (DEBUG) { printf("Input: %d | Current: %d | PWM: %d \r\n", input, int(currentSpeed), pwmSpeed); };

  return pwmSpeed;
}



void led_blink() {
  if (send_error_counter < 6) {
    digitalWrite(ledpin, HIGH);       
    delay(50);               
    digitalWrite(ledpin, LOW);    
  }
} 


void alert_on_error() {
  if (send_error_counter >= 6) {
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {               // wait for a second
    digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
  }
} 



void loop()    {
  
  throttle = filterInput(analogRead(potPin));
  
  send_throttle();
  
  alert_on_error();
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   led_blink();
   delay(50);
   debug_time = millis(); 
  };
  
}







