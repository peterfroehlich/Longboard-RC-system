 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define potPin 0 // Analog 0 
#define batPin 1 // Analog 1

#define ledPinRed 6
#define ledPinGreen 7
#define ledPinBlue 8

#define switchPinOne 2
#define switchPinTwo 4
#define switchPinThree 5
#define switchPinFourAnalog 6

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(10,9);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0xABCDABCD82LL };

char telemetric_data[20] = "";
//int telemetric_data[2] = { 0, 0 };


// Trigger Values. Better: Calibrate trigger routine. 
int throttle_max = 280;
int throttle_mid = 510;
int throttle_min = 688;
int throttle_raw = 0;
//int throttle = 0;

typedef struct
{
  int throttle;
  byte mode;
} cmdPacket;

// create an instance of the packet
cmdPacket Packet; 

unsigned int send_error_counter = 0;

float voltageValue;

bool DEBUG = true;
long debug_time = millis(); 


void initilize_rgb_led() {
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);

  digitalWrite(ledPinRed, HIGH); 
  digitalWrite(ledPinGreen, HIGH); 
  digitalWrite(ledPinBlue, HIGH); 
}
 

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
  
  initilize_rgb_led();
  
  Packet.mode = 0;
  
  show_battery_state();
  
  initilize_radio();
}
 

 
bool send_throttle() {

    if (DEBUG) { 
      printf("Sending value %d - ",Packet.throttle);
    }
    
    // First, stop listening so we can talk.
    //radio.stopListening();
    digitalWrite(ledPinBlue, LOW);
    
    bool ok = radio.write( &Packet, sizeof(Packet));
    
    // Now, continue listening
    //radio.startListening(); 
    digitalWrite(ledPinBlue, HIGH);
    
    if (ok) {
      if ( DEBUG ) { printf("send ok - "); }
      if (send_error_counter > 0) { --send_error_counter; }
      if ( radio.isAckPayloadAvailable() )  {
        radio.read(&telemetric_data,sizeof(telemetric_data));
        if ( DEBUG ) { printf("telemetry: %s\r\n", telemetric_data); };
      }
    } else {
      if ( DEBUG ) { printf("send failed!!!! - "); }
      if (send_error_counter < 10) { ++send_error_counter; }
    }
       
    return ok;
}



float get_voltage() {
  voltageValue = 0.0048875 * analogRead(batPin); 
  if ( DEBUG ) { 
    printf("\nBattery Voltage is ");
    Serial.println(voltageValue);
  }
  return voltageValue;
}


void show_battery_state() {
  voltageValue = get_voltage();
  
  if (voltageValue > 4.0) {
    led_blink(ledPinGreen, 400, 3);
  } else if (voltageValue > 3.6) {
    led_blink(ledPinGreen, 400, 2);
    led_blink(ledPinRed, 400, 1);
  } else if (voltageValue > 3.2) {
    led_blink(ledPinGreen, 400, 1);
    led_blink(ledPinRed, 400, 2);
  } else if (voltageValue > 2.9) {
    led_blink(ledPinRed, 500, 3); 
  } else { 
    led_blink(ledPinRed, 1000, 3600);
  }
  delay(1000);
}


void led_blink(int color, int time, int repeats) {
  for (int x = 0; x < repeats; x++) {
    if (x > 0) { delay(time); } 
    digitalWrite(color, LOW);   
    delay(time);               
    digitalWrite(color, HIGH);
  }
} 


void alert_on_error() {
  if (send_error_counter > 6) {
    digitalWrite(ledPinRed, LOW);   // turn the LED on (HIGH is the voltage level)
  } else {               // wait for a second
    digitalWrite(ledPinRed, HIGH);    // turn the LED off by making the voltage LOW
  }
} 



void loop()    {
  
  throttle_raw = analogRead(potPin);
  Packet.throttle =  map(throttle_raw, throttle_min, throttle_max, 0, 179);
  
  send_throttle();
  
  if (send_error_counter < 6) {
    led_blink(ledPinGreen, 20, 1);
  } else {
    alert_on_error();
  }  
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   
   delay(50);
   debug_time = millis(); 
  };
  
}






