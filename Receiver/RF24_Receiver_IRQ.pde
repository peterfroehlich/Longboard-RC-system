 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "fmtDouble.h"

#define escPin 6
#define btsPin 4
#define batteryPin 0         // +V from battery over voltage divider to analog pin 0
#define sensePin 1           // BTS555 current sensor on analog 1 

#include <Servo.h> 
Servo esc;  // create servo object to control a servo 
unsigned int coast_throttle = 70;
unsigned int throttle = coast_throttle;
unsigned int next_throttle = coast_throttle;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0xABCDABCD82LL };

char telemetric_data[20] = "Nothing.";
char telemetric_voltage[6] = "00.00";
char telemetric_current[7] = "000.00";

unsigned long last_packet_received;
unsigned int receive_timeout = 500;
unsigned int esc_half_brake_setting = 30;

bool DEBUG = true;


// Trigger Values. Better: Calibrate trigger routine. 
int throttle_max = 280;
int throttle_mid = 510;
int throttle_min = 688;
int throttle_resolution = throttle_min - throttle_max;
int servo_resolution = 180;

int filterStep = 1;
float targetSpeed;
float currentSpeed;
int pwmSpeed;
int triggerCenterValue;


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
  digitalWrite(btsPin, HIGH); 
  
}


 
void setup()   {
  if (DEBUG) { 
    Serial.begin(57600);
    printf_begin();
  }
   
  initilize_radio();
  initilize_esc();
  
  // Last step, start receiving
  last_packet_received = millis();
  attachInterrupt(0, check_radio, FALLING);
}
 

void prepare_throttle() {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &next_throttle, sizeof(unsigned int) );

      // Spew it
      if (DEBUG) { printf("Got throttle value %d...",throttle); }

	// Delay just a little bit to let the other unit
	// make the transition to receiver
	//delay(50);
    }
    if (DEBUG) { printf("\r\n"); }
}


void set_throttle() {
   //memcpy(next_throttle, throttle, sizeof(next_throttle));
   throttle = next_throttle;
   if (DEBUG) { printf("Set. \r\n"); }
  
  
  
}

void throttle_output() { 
  int output_throttle = filterInput(throttle);
  if (DEBUG) { printf("Setting throttle to %d\r\n", output_throttle); }
  esc.write(output_throttle);              
}
 


void activate_failsave() {
  if ( DEBUG ) { printf("No packet the last %d milliseconds! Activating failsave.\n\r",receive_timeout); }
  esc.write(esc_half_brake_setting);
}  


void check_radio(void)
{
  if (DEBUG) { printf("Got interrupt\r\n"); }
  // What happened?
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);

  // Have we successfully transmitted?
  if ( tx )
  {
    if ( DEBUG ) { printf("Ack Payload: Sent\n\r"); }
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


 
void loop()    {
    
  if ( last_packet_received + receive_timeout < millis() ) {
    activate_failsave();
  } else { 
    throttle_output();
    measure_current();
    measure_voltage();
  }
}







