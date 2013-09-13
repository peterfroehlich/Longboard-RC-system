
// ToDo:
// Switch ESC of after 5 sec reception lost, switch back on after 2 sek stable connection.
// Thermodiode



#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "fmtDouble.h"
#include <Servo.h> 


#define escPin 6
#define btsPin 4
#define batteryPin 0         // +V from battery over voltage divider to analog pin 0
#define sensePin 1           // BTS555 current sensor on analog 1 

// Servo init
Servo esc;  // create servo object to control a servo 
unsigned int coast_throttle = 78;
unsigned int throttle = coast_throttle;
unsigned int next_throttle = coast_throttle;


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0xABCDABCD82LL };


char telemetric_data[20] = "Nothing.";
char telemetric_voltage[6] = "00.00";
char telemetric_current[7] = "000.00";

unsigned int output_throttle_old = 0;
unsigned long last_packet_received;
unsigned long last_throttle_update;
unsigned long start_time;
unsigned int receive_timeout = 1000;
unsigned int esc_off_timeout = 3000;
unsigned int esc_half_brake_setting = 30;

bool DEBUG = false;
bool esc_power_active = false; 


int filterStep = 1;
int targetSpeed;
int currentSpeed;
int triggerCenterValue;
int throttle_ms_steps = 100;


// ****** Voltage / Current measure settings ******
const float referenceVolts = 5;        // the default reference on a 5-volt board
//const float referenceVolts = 3.3;  // use this for a 3.3-volt board

const float R1 = 23780; // value for a maximum voltage of 10 volts
const float R2 = 4690;
// determine by voltage divider resistors, see text
const float resistorFactor = 1023.0 / (R2/(R1 + R2));  

const float calibration = 37.47;

const float currentFactor = 24.5;

float battery_current = 0;
float battery_voltage = 0;

// ************************************************


void initilize_radio() {
  radio.begin();
  
  // We will be using the Ack Payload feature, so please enable it
  radio.enableAckPayload();
  
  // Open 'our' pipe for reading  
  // Open the 'other' pipe for writing, in position #1 (we can have up to 5 pipes open for reading)
  //radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  //radio.openReadingPipe(1,pipe);
      
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(16);
  
  radio.startListening();
  
  if (DEBUG) { radio.printDetails(); }
}


bool initilize_esc() {
  if (DEBUG) { printf("Initilizing esc\r\n"); }
  esc.attach(escPin);  
  esc.write(coast_throttle);  
  
  // Get Relais/BTS555 to high and switch esc on
  pinMode(btsPin, OUTPUT);
}


bool esc_power_on() {
  if (DEBUG) { printf("Activating esc.\r\n"); }
  digitalWrite(btsPin, HIGH); 
  esc_power_active = true; 
}

bool esc_power_off() {
  if ( DEBUG ) { printf("ESC power cutoff\r\n"); }
  digitalWrite(btsPin, LOW); 
  esc_power_active = false; 
}

 
void setup()   {
  if (DEBUG) { 
    Serial.begin(57600);
    printf_begin();
  }
   
  initilize_radio();
  initilize_esc();
  
  // Initilize timeout variables
  last_packet_received = millis() - receive_timeout - esc_off_timeout;
  start_time = last_packet_received;
  last_throttle_update = millis();
  
  attachInterrupt(0, check_radio, FALLING);
  if (DEBUG) { printf("Setup done, waiting for remote\r\n"); }
}
 

void prepare_throttle() {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &next_throttle, sizeof(unsigned int) );

      //delay(50);
    }
}


void set_throttle() {
   //memcpy(next_throttle, throttle, sizeof(next_throttle));
   throttle = next_throttle;
}


void throttle_output() { 
  int output_throttle = filterInput(throttle);
  if (output_throttle != output_throttle_old) {
    if (DEBUG) { printf("Input: %d | Throttle: %d \r\n", targetSpeed, output_throttle); }
    esc.write(output_throttle);
    output_throttle_old = output_throttle;
  }
}
 

void activate_failsave() {
  if ( last_packet_received + receive_timeout + esc_off_timeout < millis() ) {
    if ( esc_power_active ) {
      esc_power_off();
    }
  } else {
    if ( DEBUG ) { printf("No packet the last %d milliseconds! Activating failsave.\n\r",receive_timeout); }
    esc.write(esc_half_brake_setting);
  }
}  


void check_radio(void)
{
  if (DEBUG) { printf("Got interrupt: "); }
  // What happened?
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);

  // Have we successfully transmitted?
  if ( tx )
  {
    if ( DEBUG ) { printf("Ack Payload: Sent\n\r"); }
    // prepare next throttle only if received succesfully (else 0 would be pushed to esc) 
    set_throttle();
    // Update last packet timestamp to reset failsave timeout
    last_packet_received=millis();
    
  }

  // Have we failed to transmit?
  if ( fail )
  {
      if ( DEBUG ) { printf("Ack Payload: Failed\n\r"); }
  }

  // Did we receive a message?
  if ( rx )
  {
    if (DEBUG) { printf("Received data: "); }
    build_telemetry();
    prepare_throttle();     
  }
    
}


void build_telemetry(void) {
  //telemetric_current = "017.05";
  fmtDouble(battery_voltage, 2, telemetric_voltage, sizeof(telemetric_voltage));
  fmtDouble(battery_current, 2, telemetric_current, sizeof(telemetric_current));
  
  strcpy(telemetric_data, telemetric_voltage);
  strcat(telemetric_data, "X");
  strcat(telemetric_data, telemetric_current);
  
  radio.writeAckPayload( 1, &telemetric_data, sizeof(telemetric_data) );

}

void measure_voltage() {
  battery_voltage = (analogRead(batteryPin) / resistorFactor) * referenceVolts * calibration; // calculate the ratio
  
  if ( false ) { 
    printf("Voltage: ");
    Serial.println(battery_voltage);
  }
}


float measure_current() {
  battery_current = (analogRead(sensePin) * currentFactor);
  
  if ( false ) { 
    printf("Current: "); 
    Serial.println(battery_current);
  }
}


int filterInput(int input) {
  // Kickdown rule? targetspeed über 100 mehr als currentspeed, grössere incremente? 
  //if ((input % 10) >= 5) {
  //  targetSpeed = input + 10 - (input % 10);
  //} else if ((input % 10) < 5) {
  //  targetSpeed = input - (input % 10);
  //}
  
  targetSpeed = input;
  
  if (targetSpeed < currentSpeed) {           //Accelerating: Slowly step up
    if (currentSpeed > triggerCenterValue) {  // If coming out of brake
      currentSpeed = targetSpeed;
    } else {
      if ( last_throttle_update + throttle_ms_steps < millis() ) {
        currentSpeed -= filterStep;
        last_throttle_update = millis();
      }
    }
  } else if (targetSpeed > currentSpeed) {   //Decelerating: Quickly step down
    currentSpeed = targetSpeed;
  }
  
  //if (DEBUG) { printf("Input: %d | ", input); };

  return currentSpeed;
}

 
 
 
void loop() {
  //if (DEBUG) { printf("%d\r\n", last_packet_received); }
  
  if ( last_packet_received + receive_timeout > millis() ) {
    if (( not esc_power_active ) && ( last_packet_received != start_time )) {
      esc_power_on();
    }
    throttle_output();
    measure_current();
    measure_voltage();
    
  } else { 
    activate_failsave();
    
  }
}







