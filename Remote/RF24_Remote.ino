 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <LiquidCrystal.h>
#include "printf.h"

#define potPin 0 // Analog 0 
#define batPin 1 // Analog 1

#define ledPinRed 6
#define ledPinGreen 7
#define ledPinBlue 8

#define switchPinOne 2
#define switchPinTwo 3
#define switchPinThree 5

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

// Mode names for the UI. Can be accessed by index
char* modes[]={
  "Normal",
  "Direct",
  "EasyMode",
  "KidMode",
  "Aggresive"
};


void initilize_rgb_led() {
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);

  digitalWrite(ledPinRed, HIGH); 
  digitalWrite(ledPinGreen, HIGH); 
  digitalWrite(ledPinBlue, HIGH); 
}

void initialize_switches() {
  pinMode(switchPinOne, INPUT);
  pinMode(switchPinTwo, INPUT); 
 // pinMode(switchPinThree, INPUT);
}

void set_drive_mode() {
  
  byte switches = 0;
  
  if (digitalRead(switchPinOne) == HIGH) {switches += B1; };
  if (digitalRead(switchPinTwo) == HIGH) {switches += B10; };
  //if (digitalRead(switchPinThree) == HIGH) {switches += B100; };  
  
  Packet.mode = switches;

  if (DEBUG) {  printf("Set Drive Mode %d \r\n", Packet.mode); }  
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


void show_battery_state_led() {
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


void show_battery_state_lcd() {
  voltageValue = get_voltage();
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Battery:");
  lcd.setCursor(0, 1); 
  
  if (voltageValue > 4.0) {
    lcd.print("OK");
  } else if (voltageValue > 3.5) {
    lcd.print("Mid");
  } else if (voltageValue > 2.9) {
    lcd.print("Low");
  } else { 
    lcd.print(" EMPTY! ");
    while ( not DEBUG ) {
      lcd.display();
      delay(500);
      lcd.noDisplay();
      delay(500);
    }
  }
  
  lcd.print(" ");
  lcd.print(voltageValue);
  
  delay(1500);
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
    digitalWrite(ledPinRed, LOW);   // turn the LED on 
  } else {               
    digitalWrite(ledPinRed, HIGH);    // turn the LED off 
  }
} 


// LCD Stuff
//
//

void initialize_main_lcd() {
  lcd.clear();
  
  // first line: mode
  lcd.setCursor(0, 0); 
  lcd.print(modes[Packet.mode]);
  
  // second line, first half: throttle
  lcd.setCursor(4, 1);
  lcd.print("%"); 
  
  main_lcd();
}

void main_lcd() {
  int throttle_pct = map(throttle_raw, throttle_min, throttle_max, -100, 100);
  int offset = 0;
  if (throttle_pct > 0) {
   offset += 1;
  }
  if (abs(throttle_pct) < 10) {
   offset += 2;
  } else if (abs(throttle_pct) < 100 ) {
   offset += 1;
  } 

  lcd.setCursor(0, 1);
  for (int x = offset; x > 0; x--) {
    lcd.print(" ");
  }
  lcd.print(throttle_pct);
  
  // second line, second half: Board Voltage
  lcd.setCursor(5, 1);
  lcd.print(Telemetry.voltage);
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
  
  
  if (DEBUG) { 
   printf("Time: %d\r\n", millis() - debug_time);
   
   delay(50);
   debug_time = millis(); 
  };
  
}







