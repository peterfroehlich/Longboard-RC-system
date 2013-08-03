 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define potPin 0
#define ledpin 8 

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0xABCDABCD82LL };

char telemetric_data[20] = "";
//int telemetric_data[2] = { 0, 0 };

int throttle = 0;


unsigned int send_error_counter = 0;


bool DEBUG = true;
long debug_time = millis(); 
 

void initilize_radio() {
  radio.begin();
  
  // This simple sketch opens a single pipe for these two nodes to communicate
  // back and forth.  One listens on it, the other talks to it.
  //radio.openWritingPipe(pipe);
  radio.openWritingPipe(pipes[0]);
  //radio.openReadingPipe(1,pipes[1]);
      
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
    //radio.stopListening();

    if (DEBUG) { 
      printf("Sending value %d - ",throttle);
    }
    
    bool ok = radio.write( &throttle, sizeof(unsigned int));
    
    if (ok) {
      if ( DEBUG ) { printf("send ok - "); }
      if (send_error_counter > 0) { --send_error_counter; }
      if ( radio.isAckPayloadAvailable() )  {
        radio.read(&telemetric_data,sizeof(telemetric_data));
        if ( DEBUG ) { printf("%s\r\n", telemetric_data); };
      }
    } else {
      if ( DEBUG ) { printf("send failed!!!! - "); }
      if (send_error_counter < 10) { ++send_error_counter; }
    }
   
    // Now, continue listening
    //radio.startListening(); 
   
      
    return ok;
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
  
  throttle = analogRead(potPin);
  
  send_throttle();
  
  alert_on_error();
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   led_blink();
   delay(50);
   debug_time = millis(); 
  };
  
}







