

// Mode names for the UI. Can be accessed by index
char* modes[]={
  "EasyMode",
  "Normal",
  "Direct",
  "KidMode",
  "Aggresive"
};
int modes_len = 4;

// UI variables
int display_mode = 0;
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
  
  // second line, first half: throttle
  lcd.setCursor(4, 1);
  lcd.print("%"); 
  
  // second line, second half: Display Mode
  lcd.setCursor(7, 1);
  lcd.print(1);
  
}


void display_mode_0() {
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

  lcd.setCursor(0, 1);
  for (int x = offset; x > 0; x--) {
    lcd.print(" ");
  }
  lcd.print(throttle_pct);
  
  // second line, second half: Board Voltage
  lcd.setCursor(5, 1);
  lcd.print(Telemetry.voltage);
}



void initialize_mode_1() {
  lcd.clear();
  
  // first line: Voltage
  lcd.setCursor(5, 0); 
  lcd.print("V");
  
  // second line: Amps
  lcd.setCursor(5, 1); 
  lcd.print("A");
  
    // second line, second half: Display Mode
  lcd.setCursor(7, 1);
  lcd.print(2);
  
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
  lcd.print("Raw:");
  
  // second line, first half: translated throttle
  lcd.setCursor(0, 1);
  lcd.print("ESC:");  
  
    // second line, second half: Display Mode
  lcd.setCursor(7, 1);
  lcd.print(3);
}

void display_mode_2() {
  lcd.setCursor(4, 0);
  lcd.print(throttle_raw);
 
  lcd.setCursor(6, 1);
  lcd.print(" ");
  lcd.setCursor(4, 1);
  lcd.print(Packet.throttle); 
}



void initialize_mode_3() {
  lcd.clear();
}
  
void display_mode_3() {
  lcd.setCursor(0, 0); 
  lcd.print("Mode3");  
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

