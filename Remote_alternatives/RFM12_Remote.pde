
// Display Pins: You can use any (4 or) 5 pins 
#define sclk 4
#define mosi 5
#define cs 6
#define dc 7
#define rst 8  // you can also connect this to the Arduino reset

// Trigger poti
#define potPin 0
int triggerVal = 0;
String triggerValStr;

// Fiter Input
int filterStep = 5;
int targetSpeed;
int currentSpeed;
int pwmSpeed;
int triggerCenterValue;


// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

// Text
int LineSpacing = 10;

#include <ST7735.h>
#include <SPI.h>
#include <RFM12B.h>
#include <avr/sleep.h>


// Display Option 1: use any pins but a little slower
ST7735 tft = ST7735(cs, dc, mosi, sclk, rst);  


// RFM12B: You will need to initialize the radio by telling it what ID it has and what network it's on
// The NodeID takes values from 1-127, 0 is reserved for sending broadcast messages (send to all nodes)
// The Network ID takes values from 0-255
// By default the SPI-SS line used is D10 on Atmega328. You can change it by calling .SetCS(pin) where pin can be {8,9,10}
#define NODEID        2  //network ID used for this unit
#define NETWORKID    212  //the network ID we are on. Must be 212 for RFM12
#define GATEWAYID     1  //the node ID we're sending to
#define ACK_TIME     50  // # of ms to wait for an ack
#define SERIAL_BAUD  115200
uint8_t KEY[] = "ABCDABCDABCDABCD";
int interPacketDelay = 100; //wait this many ms between sending packets
RFM12B radio;

int counter = 1;

// Trigger Values. Better: Calibrate trigger routine. 
int throttle_max = 280;
int throttle_mid = 510;
int throttle_min = 688;
int throttle_resolution = throttle_min - throttle_max;
int servo_resolution = 180;


void setup(void) {  
  tft.initR();               // initialize a ST7735R chip
  tft.writecommand(ST7735_DISPON);
  
  tft.fillScreen(BLACK);
  
  Serial.begin(SERIAL_BAUD);
  Serial.println("hello!");
  
  // Calibrate Trigger at starup
  //calibrateTrigger();  # Future use. Now: Fixed values.
  triggerCenterValue = throttle_mid;
  targetSpeed = triggerCenterValue;
  currentSpeed = triggerCenterValue;
  
  //DebugShit();
  
  // Initilize the RFM12
  radio.Initialize(NODEID, RF12_868MHZ, NETWORKID);
  radio.Encrypt(KEY);
  //radio.Sleep(); //sleep right away to save power
  
  // Debug LED
  pinMode(9, OUTPUT);
  Serial.println("End Setup.");
}

void loop() {  
  
  //debugTrigger();
  sendTriggerValue();
  
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
  pwmSpeed = servo_resolution - ( (currentSpeed - throttle_max) / ( throttle_resolution / servo_resolution) );
  
  
  
  Serial.print(input);
  Serial.print(" ");
  Serial.print(currentSpeed);
  Serial.print(" ");
  Serial.println(pwmSpeed);
  
  
  
  return currentSpeed;
}



void sendTriggerValue() {
  
  byte sendSize=3;
  int interPacketDelay=20;
  //char outputTriggerChar[4];
  
  while (true) { 
    char outputTriggerChar[3]  = { filterInput(analogRead(potPin)) };
    //triggerVal.toCharArray(outputTriggerChar, 4);
    radio.Send(GATEWAYID, outputTriggerChar, sendSize, true);
    
    long now = millis();
    
    if (!waitForAck()) BlinkLED();
   
    delay(interPacketDelay);
  }    
}
  
  
static bool waitForAck() {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(GATEWAYID)) {
      for (byte i = 0; i < *radio.DataLen; i++) //can also use radio.GetDataLen() if you don't like pointers
        Serial.print((char)radio.Data[i]);
      Serial.println();
      return true;
    }
  return false;
}


  








//
//
// Debug Stuff
//
//

void debugTrigger() {
  char* debugTriggerMsg = "Trigger Output: ";
  tft.drawString(0, 0, debugTriggerMsg, WHITE);
  
  char* debugFilterMsg = "Filter Output: ";
  tft.drawString(0, LineSpacing, debugFilterMsg, WHITE);
  
  while (true) {
    int gap_between_lines = 100;
    
    triggerVal = analogRead(potPin);
    String inputTriggerStr = String(triggerVal);
    char outputTriggerChar[4];
    inputTriggerStr.toCharArray(outputTriggerChar, 4);
    
    String inputFilterStr = String(filterInput(triggerVal));
    
    char outputFilterChar[4];
    inputFilterStr.toCharArray(outputFilterChar, 4);
    
    tft.fillRect(gap_between_lines, 0, 20, (2 * LineSpacing), BLACK);
    
    tft.drawString(gap_between_lines, 0, outputTriggerChar, WHITE);
    tft.drawString(gap_between_lines, LineSpacing, outputFilterChar, WHITE);
    
    delay(50);
  }
}



void BlinkLED() {
  digitalWrite(9, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);               // wait for a second
  digitalWrite(9, LOW);    // turn the LED off by making the voltage LOW
  
}


void DebugShit() {
  Serial.println("CurrentSpeed: " + currentSpeed);
  Serial.println("filterStep: " + filterStep);

}


void calibrateTrigger() {
  // improve! Or fix? (max: 280, mid: 511, min: 688)
  triggerCenterValue = analogRead(potPin);
  if ((triggerCenterValue % 10) >= 5) {
    triggerCenterValue = triggerCenterValue + 10 - (triggerCenterValue % 10);
  } else if ((triggerCenterValue % 10) < 5) {
    triggerCenterValue = triggerCenterValue - (triggerCenterValue % 10);
  }
}


