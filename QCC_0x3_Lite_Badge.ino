/* QueenCityCon 0x3 lite badge sketch - Written by Jeremy Courter <jeremy@courter.org>.
 * 
 * Live long and prosper.
 */


#include <math.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <RDA5807.h> 
#include "radioxmit.h"
#include "QCCBadge.h"

//----------------------------------------------------------------------------------------------+
//                                     DEBUG Defines
//----------------------------------------------------------------------------------------------+
#define DEBUG          true            // if true, sends debug info to the serial port

//----------------------------------------------------------------------------------------------+
//                                      Functions
//----------------------------------------------------------------------------------------------+

void setup(){

  Serial.begin(9600);                   // comspec 96,N,8,1

  pinMode(RED_LED_PIN,OUTPUT);              // setup LED pin
  digitalWrite(RED_LED_PIN,HIGH);
  pinMode(GREEN_LED_PIN,OUTPUT);              // setup LED pin
  digitalWrite(GREEN_LED_PIN,HIGH);
  pinMode(BLUE_LED_PIN,OUTPUT);              // setup LED pin
  digitalWrite(BLUE_LED_PIN,HIGH);
  pinMode(RADIO_SEEK_BUTTON,INPUT_PULLUP);
  pinMode(VOL_UP_BUTTON,INPUT_PULLUP);
  pinMode(VOL_DN_BUTTON,INPUT_PULLUP);

  if(readButton(RADIO_SEEK_BUTTON)== LOW) {
#if (DEBUG)
    Serial.println("Signal hunt mode enabled!");
#endif
    ledMode = LED_RSSI_MODE;
  } else if(readButton(VOL_UP_BUTTON)== LOW) {
#if (DEBUG)
    Serial.println("Random color mode enabled!");
#endif
    ledMode = LED_RANDOM_MODE;
  } else if (readButton(VOL_DN_BUTTON) == LOW) {
#if (DEBUG)
    Serial.println("Pulse mode enabled!");
#endif
    ledMode = LED_PULSE_MODE;
  } else {
    ledMode = LED_RSSI_MODE;  // Ddefault to RSSI mode for the Lite badge
  }

  rx.setup(); // Starts the FM radio receiver with default parameters
  rx.setBand(1);  // set the band to the Japanese broadcast band (76-91MHz), which overlaps nicely with the currently unused VHF TV channels 5 and 6 - code in the loop further limits this to 76-88MHz
  rx.setVolume(7);  // Start at the middle of the volume range
  rx.setBass(true);  //Turn on extra bass for the tiny earphones

  radioPeriodStart = millis();;     // start timers
}

void loop(){
  static unsigned long lastButtonTime;  // counter for pressing the button too quickly
  static boolean blnLogStarted = false;
  static unsigned int lastFrequency = 0;
  static unsigned int lastRssi = 65535;
  static unsigned int rgbValue = 0;
  static boolean rgbDirection = 0;
  static byte currentMorseMessage;

  if (cwTransmitEnabled) { //only do this if init function has been executed - this happens if someone presses the vol_up and vol_dn buttons simultaneously
    if(!morseSender->continueSending()) {
      if (currentMorseMessage==0) {
        currentMorseMessage=1;
        morseSender->setMessage(String(CTF_SECRET_MESSAGE));
      } else {
        currentMorseMessage=0;
        morseSender->setMessage(String(CTF_SECRET_MESSAGE_PREAMBLE));
      }
      morseSender->startSending();
    }
  } else {
    if (ledMode == LED_PULSE_MODE) {
      byte r, g, b;
      if (rgbDirection==0) {
        if (rgbValue>=65534) {
          rgbDirection=1;
        }
        rgbValue++;
      } else {
        if (rgbValue<=1) {
          rgbDirection=0;
        }
        rgbValue--;
      }
      getRGBFromSpectrum(rgbValue, &r, &g, &b);
      if (lastR!=r) {
        lastR=r;
        analogWrite(RED_LED_PIN, 255-r);
      }
      if (lastG!=g) {
        lastG=g;
        analogWrite(GREEN_LED_PIN, 255-g);
      }
      if (lastB!=b) {
        lastB=b;
        analogWrite(BLUE_LED_PIN, 255-b);
      }
    }
  }

  if (readButton(RADIO_SEEK_BUTTON)== LOW && millis() >= lastButtonTime + 500){ // wait a bit between button pushes
    lastButtonTime = millis();          // reset the period time
    #if (DEBUG)
      Serial.println("RADIO_SEEK_BUTTON");
    #endif
    if (cwTransmitEnabled) {
      stopMorseSender();
      rx.powerUp();
    } else {
      //TODO make this flip between con frequencies
      rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP,NULL);
    }
  }
 
  if (readButton(VOL_UP_BUTTON) == LOW && readButton(VOL_DN_BUTTON) == LOW && (!cwTransmitEnabled) && millis() >= lastButtonTime + 500){ // wait a bit between button pushes
    lastButtonTime = millis();          // reset the period time
    #if (DEBUG)
      Serial.println("VOL_UP_BUTTON && VOL_DN_BUTTON");
    #endif
    rx.powerDown();  //turn off our FM receiver because our noisy transmitter will be detected as noise and spoil all the fun
    analogWrite(RED_LED_PIN,192);
    analogWrite(GREEN_LED_PIN,255);
    analogWrite(BLUE_LED_PIN,255);
    lastR = 192;
    lastG = lastB = 255;
    initMorseSender();
    currentMorseMessage=0;
    morseSender->setMessage(String(CTF_SECRET_MESSAGE_PREAMBLE));
    morseSender->startSending();
  } else if (readButton(VOL_UP_BUTTON)== LOW && millis() >= lastButtonTime + 500){ // wait a bit between button pushes
    lastButtonTime = millis();          // reset the period time
    #if (DEBUG)
      Serial.println("VOL_UP_BUTTON");
    #endif
    rx.setVolumeUp();
  } else if (readButton(VOL_DN_BUTTON)== LOW && millis() >= lastButtonTime + 500){ // wait a bit between button pushes
    lastButtonTime = millis();          // reset the period time
    #if (DEBUG)
      Serial.println("VOL_DN_BUTTON");
    #endif
    rx.setVolumeDown();
  }

  if (millis() >= radioPeriodStart + 100){ //query the radio module every 100ms
    radioPeriodStart = millis();          // reset the period time
    if(!cwTransmitEnabled) {
      unsigned int freq = rx.getRealFrequency();
      unsigned int rssi = rx.getRssi();
      if (freq!=lastFrequency || rssi!=lastRssi) {
        lastFrequency = freq;
        lastRssi = rssi;
  #if (DEBUG)
        Serial.print("Tuned to ");
        Serial.print(freq);
        Serial.print(", RSSI: ");
        Serial.println(rssi);
  #endif
      }
      if(freq>=8800) {
#if (DEBUG)
        Serial.println("Freq out of bounds, forcing seek");
#endif
        rx.setFrequency(7600);
        rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP,NULL);
      }
      if (ledMode == LED_RSSI_MODE) {
        signalStrengthToRGB(rssi);  //Update the LED color based on the signal strength
      } else if (ledMode == LED_RANDOM_MODE) {
        analogWrite(RED_LED_PIN,rand()%256);
        analogWrite(GREEN_LED_PIN,rand()%256);
        analogWrite(BLUE_LED_PIN,rand()%256);
      }
    }
  }
}

unsigned long readVcc() { // SecretVoltmeter from TinkerIt
  unsigned long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

static void signalStrengthToRGB(unsigned long signalStrength) {
  unsigned char r, g, b;
  unsigned long scaleMax;
  unsigned int scaledSignalStrength;
  float normalizedSignalStrength;

  scaleMax = 100;            // we'll have full magenta at this number of counts per second
/*#if (DEBUG)
  Serial.print("signalStrength:");
  Serial.print(signalStrength);
  Serial.print(",");
#endif*/
  if (signalStrength<=30) {
    signalStrength=0;
  } else {
    signalStrength-=30;
  }
  if (signalStrength)
  if (signalStrength > scaleMax) signalStrength=scaleMax;
  
  normalizedSignalStrength=sqrt((float)signalStrength/(float)scaleMax);

  if (normalizedSignalStrength > 1.0) normalizedSignalStrength=1.0;
  
  scaledSignalStrength=(unsigned int)(normalizedSignalStrength * 65534.0);
  getRGBFromSpectrum(scaledSignalStrength, &r, &g, &b);

/*#if (DEBUG)
  Serial.print("normalizedSignalStrength:");
  Serial.print(normalizedSignalStrength);
  Serial.print(",scaledSignalStrength:");
  Serial.print(scaledSignalStrength);
  Serial.print(",r:");
  Serial.print(r);
  Serial.print(",g:");
  Serial.print(g);
  Serial.print(",b:");
  Serial.println(b);
#endif*/

  if (r!=lastR) {
    lastR=r;
    analogWrite(RED_LED_PIN, 255-r);
  }
  if (g!=lastG) {
    lastG=g;
    analogWrite(GREEN_LED_PIN, 255-g);
  }
  if (b!=lastB) {
    lastB=b;
    analogWrite(BLUE_LED_PIN,255-b);
  }
}

#if (BGYRM_SPECTRUM)
void getRGBFromSpectrum(unsigned int value, unsigned char *r, unsigned char *g, unsigned char *b) {
    /*Given a value from 0 to 65535, this function maps the number to a color on a spectrum starting
    with blue, changing to green, then progressing to yellow, then to red, and finally ending at magenta*/
    // Clamp value to 0-65535
    if (value > 65535) value = 65535;

    // Determine the region (each range is 65536 / 4 = 16384)
    unsigned int region = value / 16384;  // Total of 4 regions
    unsigned int position = value % 16384;  // Position within the region

    // Scale position to a 0-LED_MAX_INTENSITY range
    unsigned char intensity = ((unsigned long)position * (unsigned long)LED_MAX_INTENSITY) / (unsigned long)16384;

    // Calculate RGB based on region
    if (region == 0) {  // Blue to green (increase green, decrease blue)
        *r = 0;
        *g = intensity;
        *b = (LED_MAX_INTENSITY - intensity);
    } else if (region == 1) {  // Green to Yellow (increase red)
        *r = intensity;
        *g = LED_MAX_INTENSITY;
        *b = 0;
    } else if (region == 2) {  // Yellow to Red (decrease green)
        *r = LED_MAX_INTENSITY;
        *g = (LED_MAX_INTENSITY - intensity);
        *b = 0;
    } else if (region == 3) {  // Red to Magenta (increase blue)
        *r = LED_MAX_INTENSITY;
        *g = 0;
        *b = intensity;
    }
}
#else
void getRGBFromSpectrum(unsigned int value, unsigned char *r, unsigned char *g, unsigned char *b) {
    /*Given a value from 0 to 65535, this function maps the number to a color on a spectrum starting
    with green, then progressing to yellow, then to red, and finally ending at magenta*/
    // Clamp value to 0-65535
    if (value > 65535) value = 65535;

    // Determine the region (each range is 65536 / 3 = 21845)
    unsigned int region = value / 21845;  // Total of 3 regions
    unsigned int position = value % 21845;  // Position within the region

    // Scale position to a 0-LED_MAX_INTENSITY range
    unsigned char intensity = ((unsigned long)position * (unsigned long)LED_MAX_INTENSITY) / (unsigned long)21845;

    // Calculate RGB based on region
    if (region == 0) {  // Green to Yellow (increase red)
        *r = intensity;
        *g = LED_MAX_INTENSITY;
        *b = 0;
    } else if (region == 1) {  // Yellow to Red (decrease green)
        *r = LED_MAX_INTENSITY;
        *g = (LED_MAX_INTENSITY - intensity);
        *b = 0;
    } else if (region == 2) {  // Red to Magenta (increase blue)
        *r = LED_MAX_INTENSITY;
        *g = 0;
        *b = intensity;
    }
}
#endif

void initMorseSender() {
  static boolean initFlag = false;
  // save timer 1 values
  TCCR1A_default = TCCR1A;
  TCCR1B_default = TCCR1B;
  OCR1A_default = OCR1A;
  // set up Timer 1
  TCCR1A = _BV (COM1A0);           // toggle OC1A on Compare Match
  TCCR1B = _BV(WGM12) | _BV(CS10); // CTC, no prescaler
  OCR1A =  2; //(From frequency table in radioxmit.h ( compare A register value to 10 (zero relative))

  if (initFlag==false) {
    morseSender = new RFMorseSender(ANTENNA_PIN, 17.0);
    morseSender->setup();
    initFlag=true;
  }
  cwTransmitEnabled=true;
} 


void stopMorseSender() {
  TCCR1A = TCCR1A_default;
  TCCR1B = TCCR1B_default;
  OCR1A = OCR1A_default;
  cwTransmitEnabled = false;
}
//----------------------------------------------------------------------------------------------+
//                                      Utilities
//----------------------------------------------------------------------------------------------+

void Blink(byte led, byte times){ // just to flash the LED
  for (byte i=0; i< times; i++){
    digitalWrite(led,LOW);
    delay (150);
    digitalWrite(led,HIGH);
    delay (100);
  }
}


// variables created by the build process when compiling the sketch
extern int __bss_end;
extern void *__brkval;

int AvailRam(){ 
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
} 

byte getLength(unsigned long number){
  byte length = 0;
  unsigned long t = 1;
  do {
    length++;
    t*=10;
  } 
  while(t <= number);
  return length;
}

byte readButton(int buttonPin) { // reads LOW ACTIVE push buttom and debounces
  if (digitalRead(buttonPin)) return HIGH;    // still high, nothing happened, get out
  else {                                      // it's LOW - switch pushed
    delay(DEBOUNCE_MS);                       // wait for debounce period
    if (digitalRead(buttonPin)) return HIGH;  // no longer pressed
    else return LOW;                          // 'twas pressed
  }
}

static void serialprint_P(const char *text) {  // print a string from progmem to the serial object
  /* Usage: serialprint_P(pstring) or serialprint_P(pstring_table[5].  If the string 
   table is stored in progmem and the index is a variable, the syntax is
   serialprint_P((const char *)pgm_read_word(&(pstring_table[index])))*/
  while (pgm_read_byte(text) != 0x00)
    Serial.write(pgm_read_byte(text++));
}

static void cycleRGB() {
//This just runs the RGB LED through the spectrum defined in getRGBFromSpectrum()
  unsigned char r, g, b;
  static unsigned char lastR, lastG, lastB;

  // Test the function with some example values

  for (unsigned int i = 0; i < 65535; i++) {
    getRGBFromSpectrum(i, &r, &g, &b);
    if (r!=lastR) {
      lastR=r;
      analogWrite(RED_LED_PIN,255-r);
    }
    if (g!=lastG) {
      lastG=g;
      analogWrite(GREEN_LED_PIN, 255-g);
    }
    if (b!=lastB) {
      lastB=b;
      analogWrite(BLUE_LED_PIN,255-b);
    }
  }
  for (unsigned int i = 65535; i > 0; i--) {
    getRGBFromSpectrum(i, &r, &g, &b);
    if (r!=lastR) {
      lastR=r;
      analogWrite(RED_LED_PIN,255-r);
    }
    if (g!=lastG) {
      lastG=g;
      analogWrite(GREEN_LED_PIN, 255-g);
    }
    if (b!=lastB) {
      lastB=b;
      analogWrite(BLUE_LED_PIN,255-b);
    }
  }
}
