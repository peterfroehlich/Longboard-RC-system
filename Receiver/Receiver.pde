
#include <RFM12B.h>
#include <avr/sleep.h>
#include <Servo.h> 

// The NodeID takes values from 1-127, 0 is reserved for sending broadcast messages (send to all nodes)
// The Network ID takes values from 0-255
#define NODEID           1  //network ID used for this unit
#define NETWORKID       212  //the network ID we are on. Must be 212 for RFM12
#define SERIAL_BAUD 115200
#define GATEWAYID     2  //Sender RFM12 NODEID

// provide a 16-byte encryption KEY (same on all nodes that talk encrypted)
uint8_t KEY[] = "ABCDABCDABCDABCD";

// Need an instance of the Radio Module
RFM12B radio;

// Brushless Controller
Servo esc;
int throttle = 0;

char buffer[] = "000";

bool debug = true;

void setup()
{
  radio.Initialize(NODEID, RF12_868MHZ, NETWORKID);
  radio.Encrypt(KEY);   
  
  if (debug) {
    Serial.begin(SERIAL_BAUD); 
    Serial.println("Listening..."); 
  }  
  esc.attach(9); 
  
}

void loop()
{
  
  if (false) {
    for(throttle = 0; throttle < 180; throttle += 1)  // goes from 0 degrees to 180 degrees 
    {                                  // in steps of 1 degree 
      esc.write(throttle);              // tell servo to go to position in variable 'throttle' 
      delay(15);                       // waits 15ms for the servo to reach the position 
    } 
    for(throttle = 180; throttle>=1; throttle-=1)     // goes from 180 degrees to 0 degrees 
    {                                
      esc.write(throttle);              // tell servo to go to position in variable 'pos' 
      delay(15);                       // waits 15ms for the servo to reach the position 
    } 
  }
  
  
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      Serial.print('[');Serial.print(radio.GetSender());Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++) { //can also use radio.GetDataLen() if you don't like pointers
        buffer[i] = (char)radio.Data[i];
      }
      Serial.print(buffer);
      esc.write(throttle);   

      if (radio.ACKRequested())
      {
        char payload[] = "TESTACK";
        byte sendSize=7;
        radio.SendACK(payload, sendSize);
        Serial.print(" - ACK sent");
      }
    }
    else
      Serial.print("BAD-CRC");
    
    Serial.println();
    
    //counter += 1;
    //if (!(counter % 3)) {
    //  Serial.println("Sending Status");
    //  byte sendSize=5;
    //  char payload[] = "abcd";
    //  radio.Send(GATEWAYID, payload, sendSize, false);
    //}
  }
}
