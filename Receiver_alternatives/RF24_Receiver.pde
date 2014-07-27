 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


#include <Servo.h> 
Servo esc;  // create servo object to control a servo 
unsigned int coast_throttle = 70;


#define potPin 0

int ledpin=8 ;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


unsigned int throttle = coast_throttle;

bool DEBUG = true;


void initilize_radio() {
  radio.begin();
  
  // Open 'our' pipe for reading  
  // Open the 'other' pipe for writing, in position #1 (we can have up to 5 pipes open for reading)
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
      
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(16);
  
  radio.startListening();
  
  if (DEBUG) { radio.printDetails(); }
}

 
void setup()   {
  if (DEBUG) { 
    Serial.begin(57600);
    printf_begin();
  }
  
  pinMode(ledpin, OUTPUT);
  esc.attach(3);  
  esc.write(coast_throttle);   
  initilize_radio();
}
 

void receive_throttle() {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &throttle, sizeof(unsigned int) );

        // Spew it
        if (DEBUG) { printf("Got throttle value %d...",throttle); }

	// Delay just a little bit to let the other unit
	// make the transition to receiver
	delay(15);
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &throttle, sizeof(unsigned int) );
      if (DEBUG) { printf("Sent response.\n\r"); }

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
    
  
}


void throttle_output() {                               
   esc.write(throttle);              
}
 


 
void loop()    {
  
  receive_throttle();
  throttle_output();
  
}



