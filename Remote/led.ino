

void show_battery_state_led() {
  voltageValue = check_rc_voltage();
  
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



void led_blink(int color, int time, int repeats) {
  for (int x = 0; x < repeats; x++) {
    if (x > 0) { delay(time); } 
    digitalWrite(color, LOW);   
    delay(time);               
    digitalWrite(color, HIGH);
  }
} 

