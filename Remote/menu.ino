
int switch_threshold_debounce = 50;
int switch_threshold_short_press = 900;

bool switch_pressed = false;
long start_press = millis();

char* menu_tree[]={
  "D. Mode",
  "Calib.RC",
  "-> exit"
};
int menu_tree_len = 2;


void check_switch_input() {
  if (digitalRead(switchPin) == HIGH) {
    if (DEBUG) { Serial.print(" Switch pressed ");  }
    if (switch_pressed == false) {
      switch_pressed = true;
      start_press = millis();
    } else {
      if (millis() - start_press > switch_threshold_short_press) {
        switch_pressed = false;
        start_menu();
      } 
    }
  } else if (switch_pressed == true) {
   switch_pressed = false;
   if (millis() - start_press < switch_threshold_short_press && millis() - start_press > switch_threshold_debounce) {
     if (display_mode == display_mode_max) {
       display_mode = 0;
     } else {
       display_mode += 1;
     } 
     initialize_main_lcd();
   } 
  }
}


  
int get_input() {  // return 0 is next, 1 is back, 2 is switch
  int threshold_plus = 50;
  int threshold_minus = -50;
  int debounce_space = 15;
  
  int pot_input = map(analogRead(potPin), throttle_min, throttle_max, -100, 100);
  
  // wait till trigger is in center position and switch is open
  while (pot_input > threshold_plus - debounce_space || pot_input < threshold_minus + debounce_space || digitalRead(switchPin) == HIGH) {
    pot_input = map(analogRead(potPin), throttle_min, throttle_max, -100, 100);
    delay(10);
  }
   
  while (true) {
    if (digitalRead(switchPin) == HIGH) {
      return 2;
    }
    
    pot_input = map(analogRead(potPin), throttle_min, throttle_max, -100, 100);
    if (pot_input > threshold_plus) {
      return 1;
    } else if (pot_input < threshold_minus) {
      return 0;
    }
  }
}

  
    
void display_menu_lev1(int point) {
  erase_line(0);
  lcd.print(menu_tree[point]);
}  
  

    
void start_menu() {
  lcd.clear();

  int menu_length = menu_tree_len;
  int menu_point = 0;
  int input;
  if (DEBUG) { 
    Serial.print("Starting menu with length ");  
    Serial.println(menu_length);  
  }
  
  bool go_on = true;
  while (go_on) {
    display_menu_lev1(menu_point);
    input = get_input();
    if (DEBUG) {
      Serial.print("Got input: ");
      Serial.println(input); 
    }
    switch (input) {
      case 0:
       if (menu_point == menu_length) { 
        menu_point = 0;
       } else {
        menu_point += 1;
       }
       break;
      case 1:
       if (menu_point == 0) {
        menu_point = menu_length;
       } else {
        menu_point -= 1;
       }
       break;
      case 2:
       if (menu_point == menu_length) { // last menupoint is exit
         go_on = false;
         while (digitalRead(switchPin)) { delay(1); }
         initialize_main_lcd();
         break;
       }
       select_menu(menu_point);
       break;  
    }
  }
}


void select_menu(int menu_point) {
  switch (menu_point) {
    case 0:
     switch_drive_mode();
     break;
    case 1:
     lcd.setCursor(0, 1);
     lcd.print("missing!");
     delay(2000);
     erase_line(1);
     break;
  }
}

void switch_drive_mode() {
  int menu_point = Packet.mode;
  int menu_length = modes_len;
  int input;

  while (true) {
    erase_line(1);
    lcd.print(modes[menu_point]);
    
    input = get_input();
    switch (input) {
      case 0:
       if (menu_point == menu_length) { 
        menu_point = 0;
       } else {
        menu_point += 1;
       }
       break;
      case 1:
       if (menu_point == 0) {
        menu_point = menu_length;
       } else {
        menu_point -= 1;
       }
       break;
      case 2:
       Packet.mode = menu_point;
       lcd.noDisplay();
       delay(500);
       lcd.display();
       delay(500);
       erase_line(1);
       return; 
    }
  }
}
    
    
