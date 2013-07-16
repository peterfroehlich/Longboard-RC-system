Longboard-RC-system
===================

A rc system for my electric longboard, based on two Arduinos talking over RFM12

Hardware
--------

### In Board (electronics)
- Arduino (Clone) Nano V3 w. USB 
- RFM12 868MHz
- Hobbyking 150A car ESC
- BTS555 MOSFET as power switch and current sensor
- Voltage divider circut to measure battery voltage
- 5V BEC

### In Remote 
- Arduino Pro Mini 
- RFM12 868MHz
- Potentiometer from old RC car remote
- MAX1811 Lipo Charger (for internal Battery)
- Step-up 5V voltage regulator 
- Sainsmart ST7735 Display / HD44780 Display / just some LEDs...
- USB to SPI board

Roadmap
-------

####v1:
- Stable radio connection (every packed gets ACKed, with telemetry payload)
- Potentiometer input on Remote acts on pwm output of Board controller with filter algorithims
- Board controller activates ESC by triggering the BTS555. (useful for automatic reprogramming of the ESC) 

####v2:
- Current and battery voltage get transmitted to Remote 
- Display speed as a function of voltage and Motor KV
- temperature monitoring
- Voltage monitoring of Remote lipo

####Later: 
- Changeable maximum speed (to allow my niece to ride =) 
- Change ESC settings via Board controller 
- Show overload quotient (Voltbased speed vs measured wheel/motor rotation)
- Changeable throttle response
- Speed via GPS module
- RGB LED lighting
- RTGs as power source
- ...

