Longboard-RC-system
===================

A rc system for my electric longboard, based on two Arduinos talking over RF24

Current state
-------------

Prototype 3 is currently in use. 

The hand controller is still based on an trimmed down RC pistol-style controller (only original part is the poti in the trigger). With HD44780 Display to show drive mode, debug output, battery state and to access the menu. 
It allows multiple drive modes with different acceleration algorithems and/or limitation of top speed. 

The board controller can control the main power to the ESC over an BTS555 Mosfet, monitors energy consumption and battery voltage. It has a standby mode, emergency braking on link loss, undervoltage protection and of cause motor control. 


Hardware
--------

### In Board (electronics)
- Arduino (Clone) Nano V3 w. USB 
- nRF24L01
- Hobbyking 150A car ESC
- BTS555 MOSFET as power switch and current sensor
- SFH610-3 optocoupler to switch BTS555 isolated from Arduino
- Voltage divider circut to measure battery voltage
- 5V BEC

### In Remote 
- Arduino (Clone) Nano V3 w. USB 
- nRF24L01
- Potentiometer from old RC car remote
- HD44780 8x2 lines Display
- MAX1811 Lipo Charger (for internal Battery)
- Step-up 5V voltage regulator 
- RGB LED
- Single button

Roadmap
-------

####v1:
- Stable radio connection (every packed gets ACKed, with telemetry payload)
- Potentiometer input on Remote acts on pwm output of Board controller with filter algorithims
- Board controller activates ESC by triggering the BTS555. (useful for automatic reprogramming of the ESC) 

####v2:
- Current and battery voltage get transmitted to Remote 
- Voltage monitoring of Remote lipo
- Display all these infos on Remote

####Later: 
- temperature monitoring
- Display speed as a function of voltage and Motor KV
- Changeable maximum speed (to allow my niece to ride =) 
- Change ESC settings via Board controller 
- Show overload quotient (Voltbased speed vs measured wheel/motor rotation)
- Changeable throttle response
- Speed via GPS module
- RGB LED lighting
- RTGs as power source
- ...

