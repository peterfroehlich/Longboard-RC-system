
int normalMode(void) {
  
  int lowSpeedHighSpeedBorder = 120;
  int highSpeedFilterStep = 2;
  int lowSpeedFilterStep = 4;
  int throttle_ms_steps = 200;
     
  if (SpeedThrottleInput < triggerCenterValue) {          //Input negetive
    SpeedCurrentOutput = SpeedThrottleInput;
  } else if (SpeedCurrentOutput < triggerCenterValue) {   //Current negative, Input positive
    SpeedCurrentOutput = triggerCenterValue;
  } else if (SpeedCurrentOutput >= SpeedThrottleInput) {   //Current positive, Input positive, current bigger
    SpeedCurrentOutput = SpeedThrottleInput;
  } else if (SpeedCurrentOutput < SpeedThrottleInput) {   //Current positive, Input positive, Input bigger
    if ( last_throttle_update + throttle_ms_steps < millis() ) {
      if (SpeedCurrentOutput < lowSpeedHighSpeedBorder) {
        SpeedCurrentOutput += lowSpeedFilterStep;
      } else {
        SpeedCurrentOutput += highSpeedFilterStep;
      }
      last_throttle_update = millis();
    }
  } 
 return SpeedCurrentOutput; 
}


int directMode(int SpeedThrottleInput) {
  return SpeedThrottleInput;
}


int easyMode(void) {
 
  int max_speed = 130;
  
  SpeedCurrentOutput = normalMode();
  if (SpeedCurrentOutput > max_speed) { SpeedCurrentOutput = max_speed; };
  
  return SpeedCurrentOutput;
}


int kidMode(void) {
 
  int max_speed = 100;
  
  SpeedCurrentOutput = normalMode();
  if (SpeedCurrentOutput > max_speed) { SpeedCurrentOutput = max_speed; };
  
  return SpeedCurrentOutput;
}


int agressiveMode(void) {
  int lowSpeedHighSpeedBorder = 120;
  int throttle_ms_steps = 150;
  int highSpeedFilterStep;
  int lowSpeedFilterStep;
  
  // Kickdown rule
  int kickdown_distance = 40;
  if ((SpeedThrottleInput - SpeedCurrentOutput) > kickdown_distance) {
    highSpeedFilterStep = 4;
    lowSpeedFilterStep = 8;
  } else {
    highSpeedFilterStep = 2;
    lowSpeedFilterStep = 4;
  }
     
     
  if (SpeedThrottleInput < triggerCenterValue) {          //Input negetive
    SpeedCurrentOutput = SpeedThrottleInput;
  } else if (SpeedCurrentOutput < triggerCenterValue) {   //Current negative, Input positive
    SpeedCurrentOutput = triggerCenterValue;
  } else if (SpeedCurrentOutput >= SpeedThrottleInput) {   //Current positive, Input positive, current bigger
    SpeedCurrentOutput = SpeedThrottleInput;
  } else if (SpeedCurrentOutput < SpeedThrottleInput) {   //Current positive, Input positive, Input bigger
    if ( last_throttle_update + throttle_ms_steps < millis() ) {
      if (SpeedCurrentOutput < lowSpeedHighSpeedBorder) {
        SpeedCurrentOutput += lowSpeedFilterStep;
      } else {
        SpeedCurrentOutput += highSpeedFilterStep;
      }
      last_throttle_update = millis();
    }
  } 
 return SpeedCurrentOutput; 
}
  
  
  
