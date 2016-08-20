//////////////////////////////////////////////////////////////////////////////
// Window Arduino Alarm using 2 Ultrasonic Sensors. 
//
//
// Menu for the LCD.
//      0 - STATUS
//      1 - SETUP ULTRASOUND LEFT
//      2 - SETUP ULTRASOUND RIGHT
//      3 - SETUP DISPLAY BACKLIGHT SETUP     (also available with a SHORT PRESS on Button 1 in STATUS MENU)
//      4 - SETUP ALARM TIME  (0,1,5,10 Sec)
//      5 - ALARM ON / OFF                    (also available with a SHORT PRESS on Button 2 in STATUS MENU)
//
//  Hardware connected as follow:
//                __,__
//   Pin A1 ------o   o------ GND      Button 1
//                __,__
//   Pin A0 ------o   o------ GND      Button 2
//
//        HC-SR04 LEFT
//   GND    ------------------         GND
//   Pin  7 ------------------         ECHO  
//   Pin  8 ------------------         TRIGGER
//   5V     ------------------         Vcc
//
//         HC-SR04 RIGHT
//   GND    ------------------         GND
//   Pin  5 ------------------         ECHO  
//   Pin  6 ------------------         TRIGGER
//   5V     ------------------         Vcc
//
//         LCD CONNECTION
//  * Pin 2 -------------------        LCD RS 
//  * Pin 4 -------------------        LCD Enable
//  * Pin 9 -------------------        LCD D4 
//  * Pin 10-------------------        LCD D5 
//  * Pin 11-------------------        LCD D6
//  * Pin 12-------------------        LCD D7 
//  * GND   -------------------        LCD R/W 
//  * Pin  3 ------------------        LCD backlight (check the current, maybe you will need a transistor to drive it)
//  * 10K resistor: (for contrast control)
//  * ends to +5V and ground
//  * wiper to LCD VO pin (pin 3)  
//
//  Solid state relay (for Alarm control)
//   Pin A3  ----------------       Solid state Relay . (use a reverse diode in parallel with the coil to prevent arduino pin destruction) 
//
//
//    EEPROM 
//
//  0   Backlight
//  1   Max distance to triger alarm on Left Sensor in cm
//  2   Max distance to triger alarm on Right Sensor in cm
//  3   siren time after detection in seconds
//  4   status of the alarm
//
//
// Button Long / Short Press script by: 
// (C) 2011 By P. Bauermeister
// http://www.instructables.com/id/Arduino-Dual-Function-Button-Long-PressShort-Press/
//
//
//   Released under a GPL licencse
//   10 July 2016
//   Author:  G4lile0
//   Twitter: @G4lile0
//   GitHub: https://github.com/G4lile0/
//   YouTube: https://www.youtube.com/channel/UCPxLTXmrzdSdE2pfa77A06Q
//////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>               //comes with Arduino
#include <LiquidCrystal.h>        //comes with Arduino
#include <LcdBarGraph.h>          //get it here http://playground.arduino.cc/Code/LcdBarGraph 
#include <NewPing.h>              //get it here https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home


// Adapt these to your board and application timings:

#define BUTTON1_PIN              A1  // Button 1
#define BUTTON2_PIN              A0  // Button 2

#define DEFAULT_LONGPRESS_LEN    10  // Min nr of loops for a long press
#define DELAY                    20  // Delay per loop in ms
#define BACKLIGHT_PIN             3  // Backlight pin-- if not enought current for the BL, then use a transistor.


#define TRIGGER_PIN_LEFT          8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_LEFT             7  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN_RIGHT         6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_RIGHT            5  // Arduino pin tied to echo pin on the ultrasonic sensor.

#define MAX_DISTANCE             250 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.


#define ALARM_PIN                 A3

NewPing sonar_left(TRIGGER_PIN_LEFT, ECHO_PIN_LEFT, MAX_DISTANCE);     // NewPing setup of pins and maximum distance.
NewPing sonar_right(TRIGGER_PIN_RIGHT, ECHO_PIN_RIGHT, MAX_DISTANCE);  // NewPing setup of pins and maximum distance.


//temporal to check speed of each menu and set the right delay. 
unsigned long time;


//////////////////////////////////////////////////////////////////////////////

enum { EV_NONE=0, EV_SHORTPRESS, EV_LONGPRESS };

//////////////////////////////////////////////////////////////////////////////
// Class definition

class ButtonHandler {
  public:
    // Constructor
    ButtonHandler(int pin, int longpress_len=DEFAULT_LONGPRESS_LEN);

    // Initialization done after construction, to permit static instances
    void init();

    // Handler, to be called in the loop()
    int handle();

  protected:
    boolean was_pressed;     // previous state
    int pressed_counter;     // press running duration
    const int pin;           // pin to which button is connected
    const int longpress_len; // longpress duration
};

ButtonHandler::ButtonHandler(int p, int lp)
: pin(p), longpress_len(lp)
{
}

void ButtonHandler::init()
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); // pull-up
  was_pressed = false;
  pressed_counter = 0;
}

int ButtonHandler::handle()
{
  int event;
  int now_pressed = !digitalRead(pin);

  if (!now_pressed && was_pressed) {
    // handle release event
    if (pressed_counter < longpress_len)
      event = EV_SHORTPRESS;
    else
      event = EV_LONGPRESS;
  }
  else
    event = EV_NONE;

  // update press running duration
  if (now_pressed)
    ++pressed_counter;
  else
    pressed_counter = 0;

  // remember state, and we're done
  was_pressed = now_pressed;
  return event;
}

//////////////////////////////////////////////////////////////////////////////

// Instanciate button objects
ButtonHandler button1(BUTTON1_PIN);
ButtonHandler button2(BUTTON2_PIN, DEFAULT_LONGPRESS_LEN*2);



byte lcdNumCols = 16; // -- number of columns in the LCD

LiquidCrystal lcd(2, 4, 9, 10, 11, 12);  // -- creating LCD instance

// -- creating a 4 chars wide bars
LcdBarGraph lbg0(&lcd, 7, 8, 0); // -- First line at column 0
LcdBarGraph lbg1(&lcd, 7, 8, 1); // -- First line at column 5
LcdBarGraph lbg2(&lcd, 6, 10, 0); // -- First line at column 10
LcdBarGraph lbg3(&lcd, 4, 0, 1); // -- Second line at column 0
LcdBarGraph lbg4(&lcd, 4, 5, 1);  // -- Second line at column 5
LcdBarGraph lbg5(&lcd, 4, 10, 1); // -- Second line at column 0

byte i0 = 0;
byte i1 = 0;
byte i2 = 0;
byte i3 = 0;
byte i4 = 0;
byte i5 = 0;
int menu = 0;

boolean backlight = true;
byte alarm_left  = 255;
byte alarm_right = 255;
boolean  alarm  = false;
byte siren_time = 0 ;


void setup(){
 
  // -- initializing the LCD
  lcd.begin(2, lcdNumCols);
  lcd.clear();
  // -- do some delay: some time I've got broken visualization
  delay(100);


 byte customChar[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b11111,
  0b00000,
  0b00100
  };

 lcd.createChar(7, customChar);

  
  Serial.begin(9600);


  // -- load backlight status from flash
  backlight= false;  
  if (EEPROM.read(0)==255)
      backlight = true;

  // -- load alarm status from flash
  alarm = false;  
  if (EEPROM.read(3)==255)
      alarm = true;


  // -- load siren_time from flash (initialitate it if detec that is the first run)
  
  if (EEPROM.read(4)==255)
      EEPROM.write(4,0);
      
  siren_time =  EEPROM.read(4); 


  alarm_left  = EEPROM.read(1);
  alarm_right = EEPROM.read(2);
  
  

   Serial.println("valor emprom");
   Serial.println(EEPROM.read(0));
   Serial.println(EEPROM.read(1));
   Serial.println(EEPROM.read(2));
   Serial.println(EEPROM.read(3));
   Serial.println(EEPROM.read(4));
                     
  
  pinMode(BACKLIGHT_PIN , OUTPUT);
  digitalWrite(BACKLIGHT_PIN, backlight); 

  pinMode(ALARM_PIN , OUTPUT);
  digitalWrite(ALARM_PIN, HIGH);
  delay(10);
  digitalWrite(ALARM_PIN, LOW);
   

  // init buttons pins; I suppose it's best to do here
  button1.init();
  button2.init();

//temporal to check speed of each menu and set the right delay. 
  time = millis();


// into secuence.
lcd.home();  // Move cursor the home position (0,0)
lcd.print("UltraSonic");  
for (int a = 0; a < 75; a++) {
      delay(90);
      lbg2.drawValue( i2, 255);
      lbg3.drawValue( i3, 255);
      lbg4.drawValue( i4, 255);
      lbg5.drawValue( i5, 255);
        // -- do some delay: frequent draw may cause broken visualization
      i2 -= 9;
      i3 += 11;
      i4 -= 13;
      i5 += 15;
      if (a==20)
      {
        lcd.home();
        lcd.print("   Alarm  ");  
        }
      if (a==50)
      {
        lcd.home();
        lcd.print("By G4lile0");  
        }
}

// end into secuence.
 lcd.clear();

}


void print_event(const char* button_name, int event)
{
  if (event)
    Serial.print(button_name);
  //  Serial.print(".SL"[event]);
 //   Serial.print(event);
}



void loop()
{

   // handle button
  int event1 = button1.handle();
  int event2 = button2.handle();

  // do other things
  print_event("1", event1);
  print_event("2", event2);



  if (event1 == 1)
    Serial.print("Boton 1 S");


  char buffer[16];

  
  // Menu managament if long press button 1 change menu
  // prior to change to a new menu, check if there is any change on the values to storage the changes on the flash. 
  if (event1 == 2)
          {

             switch (menu)
               {
                  case 0:                       // From Menu 0 to 1     "Status"  --> "SETUP ULTRASOUND LEFT"
                     Serial.println("de 0 a 1");
                     lcd.clear();
                     lcd.setCursor(3, 0);
                     lcd.print("MENU SETUP");  
                     lcd.setCursor(2, 1);
                     lcd.print("LEFT SENSOR");  
                     delay(1500);
                     lcd.clear();

                     break;     
                  case 1:                       // From Menu 1 to 2   "SETUP ULTRASOUND LEFT"  --> "SETUP ULTRASOUND RIGHT"
//                     Serial.println("de 1 a 2");
                     if (!(EEPROM.read(1) == alarm_left))     // if the distance has changed, write the changes in flash.
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(1,alarm_left);
                      }
                     lcd.clear();
                     lcd.setCursor(2, 0);
                     lcd.print("MENU SETUP");  
                     lcd.setCursor(1, 1);
                     lcd.print("RIGHT SENSOR");  
                     delay(1500);
                     lcd.clear();
                     break;
                     
                  case 2:                       // From Menu 2 to 3   "SETUP ULTRASOUND RIGHT" -->  "change backlight Status"  
//                     Serial.println("de 2 a 3");
                     if (!(EEPROM.read(2) == alarm_right))      // if the distance has changed, write the changes in flash.
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(2,alarm_right);
                      }
                     lcd.clear();
                     lcd.setCursor(1, 0);
                     lcd.print("MENU Backlight");  
                     break;
                    
                  case 3:                       // From Menu 3 to 4  "change backlight Status"  --> "ALARM TIME"                    
                     Serial.println("de 3 a 4");
                                               
                     if (EEPROM.read(0) == 0 & backlight == true)
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(0,255);
                      }
                      
                     if (EEPROM.read(0) == 255 & backlight == false)
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(0,0);
                      }
                     lcd.clear();
                     lcd.setCursor(0, 0);
                     lcd.print("MENU Alarm time");  
     
                          
                     break;
                  case 4:                       // From Menu 4 to 5    ALARM TIME"  -->   "ALARM ON/OFF"

                     if (!(EEPROM.read(4) == siren_time))      // if the alarm time has changed, write the changes in flash.
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(4,siren_time);
                      }

                     //siren_time =  EEPROM.read(4); 
                     
                     lcd.clear();
                     lcd.setCursor(1, 0);
                     lcd.print("MENU Status");  

                     Serial.println("de 4 a 5"); 
                     break;
                  case 5:                       // From Menu 5 to 6   "ALARM ON/OFF  --> "STATUS"
                     if (EEPROM.read(3) == 0 & alarm == true)
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(3,255);
                      }
                      
                     if (EEPROM.read(3) == 255 & alarm == false)
                      { Serial.println("Writing in EEprom");
                        EEPROM.write(3,0);
                      }
                     
                     Serial.println("de 5 a 0");  
                     break;
               }
            
           menu = (menu +1) % 6;                  // menu from 0 to 5
//           sprintf(buffer, "Menu is %d", menu);
//           Serial.println(buffer);
//           lcd.home();  // Move cursor the home position (0,0)
//           lcd.print(buffer);
        
          }


 // Action of the Short Press on Button 1 within each Menu.
   if (event1 == 1)
          {
             switch (menu)
               {
                  case 0:                       // Menu 0 "Status"  ShorPress 1 --> change backlight status
                     Serial.println("0 nada");
                     backlight = !backlight;
                     digitalWrite(BACKLIGHT_PIN, backlight); 
                     break;     
                  case 1:                       // Menu 1 "SETUP ULTRASOUND LEFT"  ShorPress 1 --> 2 cm less
                     Serial.println("1 nada");
                     alarm_left=alarm_left-2 ;
                     break;
                  case 2:                       // Menu 2 "SETUP ULTRASOUND RIGHT"  ShorPress 1 --> 2 cm more
                     Serial.println("2 nada");
                     alarm_right=alarm_right-2 ;

                     break;
                  case 3:                       // Menu 3 "change backlight Status"
                     Serial.println("3 nada");
                     backlight = !backlight;
                     digitalWrite(BACKLIGHT_PIN, backlight); 
                     break;
                  case 4:                        // Menu 4 " alarm time after dectection"
                     Serial.println("4 nada"); 
                     if (!(siren_time==0))
                         siren_time= siren_time-1;
                     break;
                  case 5:                        // Menu 5  " enable / disable alarm"
                     Serial.println("5 nada");  
                     alarm = !alarm;
                     break;
          
               }
          }
    

 // Action of the Short Press on Button 2 within each Menu.
   if (event2 == 1)
          {
             switch (menu)
               {
                  case 0:                       // Menu 0 "Status"  ShorPress  enable / disable alarm"
                     lcd.clear();
                     alarm = !alarm;
                     break;     
                  case 1:                       // Menu 1 "SETUP ULTRASOUND LEFT"  ShorPress 1 --> 2 cm less
                     Serial.println("1 nada");
                     alarm_left=alarm_left+2 ;
                     break;
                  case 2:                       // Menu 2 "SETUP ULTRASOUND RIGHT"  ShorPress 1 --> 2 cm more
                     Serial.println("2 nada");
                     alarm_right=alarm_right+2 ;
                     break;
                  case 3:                       // Menu 3 "change backlight Status"
                     Serial.println("3 nada");
                     backlight = !backlight;
                     digitalWrite(BACKLIGHT_PIN, backlight); 
                     break;
                  case 4:                        // Menu 4 " alarm time after dectection"
                     Serial.println("4 nada"); 
                     siren_time= siren_time + 1;
                     break;
                  case 5:                        // Menu 5  " enable / disable alarm"
                     Serial.println("5 nada");  
                     alarm = !alarm;
                     delay(100);
                     break;
          
               }
          }




// what to display and execute a depending on the Menu.
  switch (menu)
   {
   case 0:                       // Menu 0 "Status" 
     
      i0 = sonar_left.convert_cm(sonar_left.ping_median(5));
      if (i0 == 0 ) i0=MAX_DISTANCE;
      lbg0.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg0.drawValue( i0, MAX_DISTANCE);
      lcd.home();  // Move cursor the home position (0,0)
      sprintf(buffer, "DL %dcm", i0);
      lcd.print(buffer);
  
      if (((i0 < alarm_left) && alarm ))
      {
          lcd.clear();
          lcd.home();  // Move cursor the home position (0,0)
          lcd.print(" !!!!ALARM!!!!!"); 
          lcd.setCursor(0, 1);
          lcd.print("!!!LEFT SENSOR!!"); 
          digitalWrite(ALARM_PIN, HIGH);
          delay((siren_time*1000)+100);
          digitalWrite(ALARM_PIN, LOW);
          lcd.clear();
              }
 
            
      i1 = sonar_right.convert_cm(sonar_right.ping_median(5));
      if (i1 == 0 ) i1=MAX_DISTANCE;
      lbg1.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg1.drawValue( i1, MAX_DISTANCE);
      lcd.setCursor(0, 1);
      sprintf(buffer, "DR %dcm", i1);
      lcd.print(buffer);


     if (((i1 < alarm_right) && alarm ))
      {
          lcd.clear();
          lcd.home();  // Move cursor the home position (0,0)
          lcd.print(" !!!!ALARM!!!!!"); 
          lcd.setCursor(0, 1);
          lcd.print("!!RIGHT SENSOR!!"); 
          digitalWrite(ALARM_PIN, HIGH);
          delay((siren_time*1000)+100);
          digitalWrite(ALARM_PIN, LOW);
          lcd.clear();
              }
     
      break;     
      
    case 1:                       // Menu 1
      delay(10);
      i0 = sonar_left.convert_cm(sonar_left.ping_median(5));
      if (i0 == 0 ) i0=MAX_DISTANCE;
      lbg0.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg0.drawValue( i0, MAX_DISTANCE);
      sprintf(buffer, "DL %dcm", i0);
      lcd.home();  // Move cursor the home position (0,0)
      lcd.print(buffer);
      lcd.setCursor(0, 1);
      sprintf(buffer, "AL %dcm", alarm_left);
      lcd.print(buffer);
      lbg1.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg1.drawValue( alarm_left, MAX_DISTANCE);
      break;

    case 2:                       // Menu 2 "SETUP ULTRASOUND RIGHT"  ShorPress 1 --> 2 cm more
      delay(10);
      i0 = sonar_right.convert_cm(sonar_right.ping_median(5));
      if (i0 == 0 ) i0=MAX_DISTANCE;
      sprintf(buffer, "DR %dcm", i0);
      lcd.home();  // Move cursor the home position (0,0)
      lcd.print(buffer);
      lbg0.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg0.drawValue( i0, MAX_DISTANCE);
      lcd.setCursor(0, 1);
      sprintf(buffer, "AR %dcm", alarm_right);
      lcd.print(buffer);
      lbg1.drawValue( MAX_DISTANCE, MAX_DISTANCE);          // to fix a bug that make the bar dissapear.
      delay(10);
      lbg1.drawValue( alarm_right, MAX_DISTANCE);
      break;

    case 3:                       // Menu 3 "change backlight Status"
      delay(90);
      break;

    case 4:                       // Menu 4 " alarm time after dectection"
      lcd.setCursor(0, 1);
      sprintf(buffer, "Siren %d seconds", siren_time);
      lcd.print(buffer);
      delay(90);
      break;

    
    case 5:                        // Menu 5  " enable / disable alarm"
      delay(90);
      lcd.setCursor(3, 1);
      if (alarm)
         lcd.print("Alarm ON ");
       else
       {
         lcd.print("Alarm OFF");
         lcd.setCursor(15,1);
         lcd.write(" ");
       }
     
//      delay(DELAY);
      break;
          
   }


if (alarm) {
lcd.setCursor(15,1);
lcd.write(7);
  }



//temporal to check speed of each menu and set the right delay. 
  sprintf(buffer, "Men %d %u ", menu, (millis()- time));
//  Serial.println(buffer);
  time = millis();



}
