 // ToDo: 
 // Show battery state of controller batt to user
 // 
 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <LiquidCrystal.h>
#include "printf.h"
#include <display.ino>
#include <menu.ino>
#include <led.ino>

#define potPin 0 // Analog 0 
#define batPin 1 // Analog 1

#define ledPinRed 6
#define ledPinGreen 7
#define ledPinBlue 8

#define switchPin 2

// HD44780 Display
#define LCDRS 4
#define LCDE 5
#define LCDD4 16
#define LCDD5 17
#define LCDD6 18
#define LCDD7 19

#define VERSION "Ver 0.2"


bool DEBUG = false;
long debug_time = millis(); 


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(10,9);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0xABCDABCD82LL };

LiquidCrystal lcd(LCDRS, LCDE, LCDD4, LCDD5, LCDD6, LCDD7);

// Trigger Values. Better: Calibrate trigger routine. 
int throttle_max = 280;
int throttle_mid = 510;
int throttle_min = 688;
int throttle_raw = 0;

//Command packet
typedef struct
{
  int throttle;
  byte mode;
} cmdPacket;

cmdPacket Packet; 

 //Telemetry stuff
typedef struct
{
  int voltage;
  int current;
} telemetryPacket;

telemetryPacket Telemetry;


unsigned int send_error_counter = 0;

float voltageValue;

void initilize_rgb_led() {
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);

  digitalWrite(ledPinRed, HIGH); 
  digitalWrite(ledPinGreen, HIGH); 
  digitalWrite(ledPinBlue, HIGH); 
}





void set_drive_mode() {
  
  byte switches = 0;
  
  if (digitalRead(switchPin) == HIGH) {switches += B1; };
  //if (digitalRead(switchPinTwo) == HIGH) {switches += B10; };
  //if (digitalRead(switchPinThree) == HIGH) {switches += B100; };  
  
  Packet.mode = switches;

  if (DEBUG) {  printf("Set Drive Mode %d \r\n", Packet.mode); }  
}




void initialize_switches() {
  pinMode(switchPin, INPUT);
  //attachInterrupt(0, check_switch_input, RISING);
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


void initialize_lcd() {
  lcd.begin(8, 2); 
  lcd.print("LB-Ctrl");
  lcd.setCursor(0, 1); 
  lcd.print(VERSION);
  
  delay(1000);
}
  

 
void setup()   {
  if (DEBUG) { 
    Serial.begin(57600);
    printf_begin();
  }
  
  initilize_rgb_led();
  initialize_switches();
  initialize_lcd();
  set_drive_mode();

  show_battery_state_lcd();
  
  initilize_radio();
  initialize_main_lcd();
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
        radio.read(&Telemetry,sizeof(Telemetry));
        if ( DEBUG ) { printf("telemetry: mVolt: %d mAmps: %d \r\n", Telemetry.voltage, Telemetry.current ); };
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






// Main Loop
//
//

void loop() {
  throttle_raw = analogRead(potPin);
  Packet.throttle =  map(throttle_raw, throttle_min, throttle_max, 0, 179);
  
  send_throttle();
  
  if (send_error_counter < 6) {
    led_blink(ledPinGreen, 20, 1);
  } else {
    alert_on_error();
  }  
  
  main_lcd();
  
  check_switch_input();  
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   
   delay(50);
   debug_time = millis(); 
  };
  
}







