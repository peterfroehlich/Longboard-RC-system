

// Mode names for the UI. Can be accessed by index
char* modes[]={
  "EasyMode",
  "Normal",
  "Direct",
  "KidMode",
  "Aggresive"
};
int modes_len = 4;

// Notification texts (must fill all chars)
char* notification_texts[]={
  " RC LOW ",
  "BATT LOW",
  "! TEMP !",
  "NO LINK "
};

int display_mode_max = 3;


void erase_line(int line) {
  lcd.setCursor(0, line);
  lcd.print("        ");
  lcd.setCursor(0, line);
}


void initialize_main_lcd() {
  switch (display_mode) {
    case 0:
     initialize_mode_0();
     break;
    case 1:
     initialize_mode_1();
     break;
    case 2:
     initialize_mode_2();
     break;
    case 3:
     initialize_mode_3();
     break;
  }
  main_lcd();  
}

void main_lcd() {
  
  switch (display_mode) {
    case 0:
     display_mode_0();
     break;
    case 1:
     display_mode_1();
     break;
    case 2:
     display_mode_2();
     break;
    case 3:
     display_mode_3();
     break;
  }
  
}


void initialize_mode_0() {
  lcd.clear();
  
  // first line: mode
  lcd.setCursor(0, 0); 
  lcd.print(modes[Packet.mode]);
  
  // second line, first half: Battery
  lcd.setCursor(0, 1);
  lcd.print("%"); 
  
  lcd.setCursor(5, 1);
  lcd.print("Bat");
}


void display_mode_0() {

  // second line, Battery percent
  lcd.setCursor(4, 1);
  lcd.print(" ");  
  
  lcd.setCursor(1, 1);
  lcd.print(map(Telemetry.voltage, voltage_low, voltage_high, 0, 100));

}



void initialize_mode_1() {
  lcd.clear();
  
  // first line: Voltage
  lcd.setCursor(5, 0); 
  lcd.print("V");
  
  // second line: Amps
  lcd.setCursor(5, 1); 
  lcd.print("A");
}

void display_mode_1() {
  lcd.setCursor(0, 0);
  lcd.print(Telemetry.voltage / 1000.0); 
  
  lcd.setCursor(0, 1);
  lcd.print(Telemetry.current / 1000.0);  
}
  
  
  
void initialize_mode_2() {
  lcd.clear();
  
  // first line: Raw throttle
  lcd.setCursor(0, 0); 
  lcd.print("R");
  
  // first line, second half: translated throttle
  lcd.setCursor(4, 0);
  lcd.print("E"); 
 
  // procent throttle
  lcd.setCursor(4, 1);
  lcd.print("%");  
}

void display_mode_2() {
  int throttle_pct = map(throttle_raw, throttle_min, throttle_max, -100, 100);
  int offset = 0;
  if (throttle_pct >= 0) {
   offset += 1;
  }
  if (abs(throttle_pct) < 10) {
   offset += 2;
  } else if (abs(throttle_pct) < 100 ) {
   offset += 1;
  } 
  
  lcd.setCursor(1, 0);
  lcd.print(throttle_raw);
 
  lcd.setCursor(7, 0);
  lcd.print(" ");
  lcd.setCursor(5, 0);
  lcd.print(Packet.throttle); 
  
  lcd.setCursor(0, 1);
  for (int x = offset; x > 0; x--) {
    lcd.print(" ");
  }
  lcd.print(throttle_pct);
}



void initialize_mode_3() { // reserved for notifications
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("No error");
}
  
void display_mode_3() {
  // notifications byte: 
  //  1:  RC batt voltage low
  //  2:  Board batt voltage low
  //  4:  Temperature alert 
  //  8:  Link lost
  
  if (notifications) {
    lcd.clear();
    byte line = 0;
    byte mask = 1;
    int counter = 0;
    
    for (mask = 0001; mask > 0; mask <<= 1) {  // later notifications overwrite earlier ones -> priority in notification

      if (notifications & mask) {
        notifications = notifications ^ mask; // unset notification
        
        lcd.setCursor(0, line); 
        lcd.print(notification_texts[counter]); 
       
        line = B1 ^ line;
      }
      counter += 1;
    } 
  }
}



  
void show_battery_state_lcd() {
  voltageValue = check_rc_voltage();
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
}



