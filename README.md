# Arduino_ultrasonic_alarm

<b>Warning: This is a prototype, some parts need to be improved.</B>



 Window Arduino Alarm using 2 Ultrasonic Sensors. 


 Menu for the LCD.
      0 - STATUS
      1 - SETUP ULTRASOUND LEFT
      2 - SETUP ULTRASOUND RIGHT
      3 - SETUP DISPLAY BACKLIGHT SETUP     (also available with a SHORT PRESS on Button 1 in STATUS MENU)
      4 - SETUP ALARM TIME  (0,1,5,10 Sec)
      5 - ALARM ON / OFF                    (also available with a SHORT PRESS on Button 2 in STATUS MENU)

  Hardware connected as follow:
                __,__
   Pin A1 ------o   o------ GND      Button 1
                __,__
   Pin A0 ------o   o------ GND      Button 2

        HC-SR04 LEFT
   GND    ------------------         GND
   Pin  7 ------------------         ECHO  
   Pin  8 ------------------         TRIGGER
   5V     ------------------         Vcc

         HC-SR04 RIGHT
   GND    ------------------         GND
   Pin  5 ------------------         ECHO  
   Pin  6 ------------------         TRIGGER
   5V     ------------------         Vcc

         LCD CONNECTION
  * Pin 2 -------------------        LCD RS 
  * Pin 4 -------------------        LCD Enable
  * Pin 9 -------------------        LCD D4 
  * Pin 10-------------------        LCD D5 
  * Pin 11-------------------        LCD D6
  * Pin 12-------------------        LCD D7 
  * GND   -------------------        LCD R/W 
  * Pin  3 ------------------        LCD backlight (check the current, maybe you will need a transistor to drive it)
  * 10K resistor: (for contrast control)
  * ends to +5V and ground
  * wiper to LCD VO pin (pin 3)  

  Solid state relay (for Alarm control)
   Pin A3  ----------------       Solid state Relay . (use a reverse diode in parallel with the coil to prevent arduino pin destruction) 


    EEPROM 

  0   Backlight
  1   Max distance to triger alarm on Left Sensor in cm
  2   Max distance to triger alarm on Right Sensor in cm
  3   siren time after detection in seconds
  4   status of the alarm


##License

<img src="./images/by-sa.png" width="200" align = "center">

All these products are released under [Creative Commons Attribution-ShareAlike 4.0 International License](http:creativecommons.org/licenses/by-sa/4.0/).

Todos estos productos están liberados mediante [Creative Commons Attribution-ShareAlike 4.0 International License](http:creativecommons.org/licenses/by-sa/4.0/).
