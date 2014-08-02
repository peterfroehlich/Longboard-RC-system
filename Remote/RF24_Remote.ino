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
#define LCDD4 16 // Analog 2
#define LCDD5 17 // Analog 3
#define LCDD6 18 // Analog 4
#define LCDD7 19 // Analog 5

#define VERSION "Ver 0.3"


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

byte notifications = 0;
int notification_period = 4000;
long last_notification = millis();  
unsigned int send_error_counter = 0;
int voltage_low_threshold = 20000;
float rc_voltage_low_threshold = 2.8;

int voltage_low = 19500;
int voltage_high = 25300;

float voltageValue;

// UI variables
int display_mode = 0;



// SETUP 
//
//

void initilize_rgb_led() {
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);

  digitalWrite(ledPinRed, HIGH); 
  digitalWrite(ledPinGreen, HIGH); 
  digitalWrite(ledPinBlue, HIGH); 
  
  led_blink(ledPinRed, 300, 1);
  led_blink(ledPinGreen, 300, 1);
  led_blink(ledPinBlue, 300, 1);
}



void set_drive_mode() {
  
  byte switches = 0;
  
  if (digitalRead(switchPin) == HIGH) {switches += B1; };
  Packet.mode = switches;

  if (DEBUG) {  printf("Set Drive Mode %d \r\n", Packet.mode); }  
}



void initialize_switches() {
  pinMode(switchPin, INPUT);
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
  
  delay(800);
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
  delay(1500);
  
  initilize_radio();
  initialize_main_lcd();
}
 

 
bool send_throttle() {

    if (DEBUG) { 
      printf("Sending value %d - ",Packet.throttle);
    }
    
    bool ok = radio.write( &Packet, sizeof(Packet));
    
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



float check_rc_voltage() {
  voltageValue = 0.0048875 * analogRead(batPin);
  if (voltageValue < rc_voltage_low_threshold) {
    notifications = notifications | B1; 
  }
  if ( DEBUG ) { 
    printf("\nBattery Voltage is ");
    Serial.println(voltageValue);
  }
  return voltageValue;
}


void check_board_voltage() {
  if (Telemetry.voltage < voltage_low_threshold && Telemetry.voltage != 0) {
    notifications = notifications | B10;
  }
}




// Main Loop
//
//

void loop() {
  throttle_raw = analogRead(potPin);
  Packet.throttle =  map(throttle_raw, throttle_min, throttle_max, 0, 179);
  
  send_throttle();
  
  if (send_error_counter < 8) {
    if (DEBUG) { 
      led_blink(ledPinGreen, 20, 1);
    } else {
      delay(25);
    }
  } else {
    notifications = notifications | B1000;
    alert_on_error();
    delay(10);
  }  
    
  check_board_voltage();  
    
  if (notifications && millis() - last_notification > notification_period ) {
    last_notification = millis();
    display_mode = 3;
  }
  main_lcd();
  
  check_switch_input();  
  
  check_rc_voltage();
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   
   delay(50);
   debug_time = millis(); 
  }
  
}







