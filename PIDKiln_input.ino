/*
** Pidkiln input (rotary encoder, buttons) subsystem
**
*/

// Other variables
//

// Vars for interrupt function to keep track on encoder
volatile int lastEncoded = 0;
volatile int encoderValue = 0;
volatile unsigned long encoderButton = 0;

// Global value of the encoder position
int lastencoderValueT = 0;
byte menu_pos=0,screen_pos=0;

// Tell compiler it's interrupt function - otherwise it won't work on ESP
void ICACHE_RAM_ATTR handleInterrupt ();


// What to do if button pressed in menu
//
void pressed_menu(){
  switch(LCD_Menu){
    case M_MAIN_VIEW: LCD_display_main(); break;
    case M_LIST_PROGRAMS: LCD_display_programs(0); break;
    case M_INFORMATIONS: LCD_display_info(); break;
    case M_ABOUT: LCD_display_about(); break;
    default: break;
  }
}


// What to do if button pressed in main view
//
void pressed_main_view(){
  
}


// What to do if button pressed in program list
//
void pressed_program_list(){
  
}


// Just redirect pressed button to separate functions
//
void button_pressed(){
  if(LCD_State==MENU) pressed_menu();
  else if(LCD_State=MAIN_VIEW) pressed_main_view();
  else if(LCD_State=PROGRAM_LIST) pressed_program_list();
  else LCD_display_menu();  // if pressed something else - go back to menu
}


// Handle long press on encoder
//
void button_Long_Press(){

  if(LCD_State==MENU){ // we are in menu - switch to main screen
    LCD_State=MAIN_VIEW;
    LCD_display_main();
    return;
  }else if(LCD_State==PROGRAM_SHOW){ // if we are showing program - go to program list
    LCD_State=PROGRAM_LIST;
    LCD_display_programs(0); // LCD_Program is global
    return;
  }else{ // If we are in MAIN screen or Program list or in unknown area to to -> menu
    LCD_State=MENU; // switching to menu
    LCD_display_menu();
    return;    
  }
}


// Handle or rotation encoder input
//
void rotate(){

// If we are in MAIN screen view
 if(LCD_State==MAIN_VIEW){
  if(encoderValue<0){
    if(LCD_Main>MAIN_VIEW1) LCD_Main=(LCD_MAIN_View_enum)((int)LCD_Main-1);
    LCD_display_main();
    return;
  }else{
    if(LCD_Main<MAIN_end-1) LCD_Main=(LCD_MAIN_View_enum)((int)LCD_Main+1);
    LCD_display_main();
    return;
  }
 }else if(LCD_State==MENU){
   DBG Serial.printf("Rotate, MENU: Encoder turn: %d, Sizeof menu %d, Menu nr %d, \n",encoderValue, Menu_Size, LCD_Menu);
   if(encoderValue<0){
     if(LCD_Menu>M_MAIN_VIEW) LCD_Menu=(LCD_MENU_Item_enum)((int)LCD_Menu-1);
   }else{
     if(LCD_Menu<M_end-1) LCD_Menu=(LCD_MENU_Item_enum)((int)LCD_Menu+1);
   }
   LCD_display_menu();
   return;
 }else if(LCD_State==PROGRAM_LIST){
   DBG Serial.printf("Rotate, PROGRAMS: Encoder turn: %d\n",encoderValue);
   LCD_display_programs(encoderValue);
 }else if(LCD_State==PROGRAM_SHOW){
  
 }
}
















// Main input loop
//
void input_loop() {

   if(encoderButton){
      delay(ENCODER_BUTTON_DELAY);
      if(digitalRead(ENCODER0_BUTTON)==LOW) return; // Button is still pressed - skipp, perhaps it's a long press
      if(encoderButton+Long_Press>=millis()){ // quick press
        DBG Serial.printf("Button pressed %f seconds\n",(float)(millis()-encoderButton)/1000);
        button_pressed();
      }else{  // long press
        DBG Serial.printf("Button long pressed %f seconds\n",(float)(millis()-encoderButton)/1000);
        button_Long_Press();
      }
      encoderButton=0;
   }
   else if(encoderValue!=0){
      delay(ENCODER_ROTATE_DELAY);
      rotate(); // encoderValue is global..
      DBG Serial.printf("Encoder rotated %d\n",encoderValue);
      encoderValue=0;
  }
}


// Interrupt parser for rotary encoder and it's button
//
void handleInterrupt() {

  if(digitalRead(ENCODER0_BUTTON)==LOW){
      encoderButton=millis();
  }else{ // Those two events can be simultanouse - but this is also ok, usualy user does not press and turn
    int MSB = digitalRead(ENCODER0_PINA); //MSB = most significant bit
    int LSB = digitalRead(ENCODER0_PINB); //LSB = least significant bit
    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue=1;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue=-1;

    lastEncoded = encoded; //store this value for next time
  }
}


// Setup all input pins and interrupts
//
void setup_input() {

  pinMode(ENCODER0_PINA, INPUT); 
  pinMode(ENCODER0_PINB, INPUT);
  pinMode(ENCODER0_BUTTON, INPUT);

  digitalWrite(ENCODER0_PINA, HIGH); //turn pullup resistor on
  digitalWrite(ENCODER0_PINB, HIGH); //turn pullup resistor on
  digitalWrite(ENCODER0_BUTTON, HIGH); //turn pullup resistor on

  attachInterrupt(ENCODER0_PINA, handleInterrupt, CHANGE);
  attachInterrupt(ENCODER0_PINB, handleInterrupt, CHANGE);
  attachInterrupt(ENCODER0_BUTTON, handleInterrupt, FALLING);
}
