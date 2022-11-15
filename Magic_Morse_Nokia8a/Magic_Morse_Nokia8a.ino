/*
  MAGIC MORSE (c) 2011, 2012, 2013 by Mickey R. Burnette, AKA Ray Burne
  This implementation specific to Arduino Mini-Pro 328 running at 3.3V
  and used with a NOKIA 5110 Craphic LCD.
  Built under Arduino 1.0.5
  Binary sketch size: 11,108 bytes (of a 30,720 byte maximum) MiniPro 5V profile
  Version 8, 20130515 FreeMem() 1086
*/
// djrm 5 sept 2016
#include <EEPROM.h>
#include <Streaming.h>
//#include <PrintEx.h>
#include <avr/pgmspace.h>
#include "Defines.h"                // Nokia 5110 LCD pin usage as controlled by "ScrnFuncts.ino"

// constants
const int  BAUD            = 9600;  // any standard serial value: 300 - 115200
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  toneHz          = 748;   // 748 = 746.3 Hz on freq counter (750 = 757.6Hz)
const int  LED_flashdelay  = 2;     // mS ... small dealy to allow eye to see LED
const int  MaxConsoleCount = 80;    // Serial output characters before linefeed+cr
const long debounceDelay   = 20;    // the debounce time. Keep well below dotTime
// Arduino 328P pins (not physical)
const int  VarResPin       = A0;    // select the input pin for the potentiometer
const byte morseInPin      = A3;    // Used for Morse Key (other end to Gnd)
const byte VerbosePin      =  9;    // Echos from within MagicMorse() decode state of dit/dah
const byte forcePARISpin   = 10;    // When momentary LOW, forces PARIS to be output on LCD/Serial
const int  toneOutPin      = 11;    // PWM output for simulated 750Hz (F#1/Gb1 = 746. Hz)
const int  LED_RED         = 12;    // Red   LED indicates Dit
const int  LED_GREEN       = 13;    // Green LED indicates Dah
// global prog variables
byte WPM;                           // set by POT on pin#A0
byte nRow;                          // line count      (0-5 for NOKIA LCD)
byte nColumn;                       // character count (0-11 for NOKIA LCD)
byte ConsoleCount;                  // Serial character output counter
byte NOKIAcontrast         = 0xB2;  // LCD initialization contrast
boolean Verbose;                    // state of Pin#9
// Character array pointers
const char*  const PROGMEM msg0[]       = {"Magic Morse (c) 2013 by M. Burnette All Rights  Reserved    Ver 8.130529"};
const char* /*PROGMEM*/ msg1[]       = {"*Long Dash* With Morse  Key To Begin"};
const char* /*PROGMEM*/ msg2[]       = {"Currently:  WPM = "};
const char* /*PROGMEM*/ msg3[]       = {"Auto Cycling"};
const char* /*PROGMEM*/ msg4[]       = {"Release Key!"};
const char* /*PROGMEM*/ msg5[]       = {"LONG Dash To"};
const char* /*PROGMEM*/ msg6[]       = {"Save  EEPROM"};
const char* /*PROGMEM*/ msg7[]       = {"WPM = "};
const char* /*PROGMEM*/ BlankLine[]  = {"            "};// Nokia 12 x 6 (84 * 48)


void setup(void)
  { 
    pinMode(morseInPin, INPUT);
    digitalWrite(morseInPin, HIGH);  // activate internal pullup resistor
    pinMode(VerbosePin, INPUT);
    digitalWrite(VerbosePin, HIGH);  // activate internal pullup
    pinMode(toneOutPin, OUTPUT);
    tone(toneOutPin, toneHz);
    noTone(toneOutPin);
    pinMode(LED_RED, OUTPUT);    digitalWrite(LED_RED, HIGH);
    pinMode(LED_GREEN, OUTPUT);  digitalWrite(LED_GREEN, HIGH);
    pinMode(forcePARISpin, INPUT);
    digitalWrite(forcePARISpin, HIGH);  // internal pullup resistor
    LEDflasher(LED_GREEN); LEDflasher(LED_RED);
    WPM = EEPROM.read(EEaddr);
    // spool some basic information out diagnostic ASIC serial port
    Serial.begin(BAUD);
    Serial << (F("(c) 2011 - 2013 by M. R. Burnette")) << endl;
    Serial << (F("Free RAM available: ")) ;
    Serial << freeRam() << endl;
    Serial << (F("Version 8.130529")) << endl;
    // LCD
    LcdInitialise();  LcdClear(); LcdString(*msg0);  
    delay(2000);
    // Set WPM default and write to EEPROM IF Morse Key is closed at this point...
    if (!digitalRead(morseInPin) || WPM == 0 || WPM > 40) setWPM(WPM);
    LcdClear(); delay(500); nColumn = 0; nRow = 0;
    LcdString(*msg2); nRow = 2; dispcountt(WPM);
    setspeed(WPM);
    Djrm();
    //Paris();
    ++nRow; nColumn = 0;
    gotoXY(nColumn, nRow);
    LcdCurrentLine(nRow);
}


void loop(void)
  {
    Verbose = (!digitalRead(VerbosePin)); // IF pin is LOW, Verbose is TRUE, else FALSE
    if (!digitalRead(forcePARISpin)) 
      //Paris();
      Djrm();
    ReadMorseKeyState();
    char temp = MagicMorse();
    if (temp != 0) {
      Serial.print(temp);  // debug channel - physical ASIC on ATmega328P
      if (++ConsoleCount >79 ) {
        Serial << endl;
        ConsoleCount = 1; }
      SendMorseLCD(temp); }
  } // loop()

