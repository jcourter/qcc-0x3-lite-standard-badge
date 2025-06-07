//----------------------------------------------------------------------------------------------+
//               PIN MAP for  ATmega328P - Each I/O pin used is defined . . .
//----------------------------------------------------------------------------------------------+

// PIN MAP - Each I/O pin (used or unused) is defined . . .
//                        19             // (A5) RESERVED for I2C
//                        18             // (A4) RESERVED for I2C
#define LED5_PIN          17             // Discrete LED around the circumference of the board
#define LED4_PIN          16             // Discrete LED around the circumference of the board
#define SAO_GPIO2         15             // (A1) Simple Add-On header GPIO2 pin
#define SAO_GPIO1         14             // (A0) Simple Add-On header GPIO1 pin
#define GREEN_LED_PIN     10             // RGB LED Green Pin
#define ANTENNA_PIN        9             // PWM output for RF noise maker (it's cleaner than a spark gap)
#define TONE_PIN           9             // PWM output to speaker or piezo for tone mode
#define VOL_UP_BUTTON      8             // Volume up button (SW2)
#define VOL_DN_BUTTON      7             // Volume up button (SW3)
#define RED_LED_PIN        6             // RGB LED Red Pin
#define BLUE_LED_PIN       5             // RGB LED Blue Pin
#define RADIO_SEEK_BUTTON  4             // Button to move to the next FM station (SW4)
#define LED3_PIN           3             // Discrete LED around the circumference of the board
#define LED2_PIN           2             // Discrete LED around the circumference of the board
//                         D1 & D0          serial comm

//----------------------------------------------------------------------------------------------+
//                                 other defines . . .
//----------------------------------------------------------------------------------------------+
#define BGYRM_SPECTRUM    true           // set to true for blue->green->yellow->red->magenta, set to false for green->yellow->red->magenta

#define LED_MAX_INTENSITY   15         // maximum intensity for the LED

#define LED_RSSI_MODE       1           //Show signal strength on the LED
#define LED_PULSE_MODE      2           //Make the LED slowly pulse
#define LED_RANDOM_MODE     3           //The LED will flash random colors
#define LED_RADIATION_MODE  4           //Show radiation level on the LED
#define LED_MODE_ADDR       0x00        //EEPROM address for LED mode setting

#define ONE_SEC_MAX          20         // elements in the oneSecond accumulator array
#define DEBOUNCE_MS          50         // buttom debounce period in mS
#define INFINITY             65534      // if scalerPeriod is set to this value, it will just do a cumulative count forever

#define QCC_MAX_FREQ       7900		// max frequency for radio stations
#define QCC_MIN_FREQ       7600		// min frequency for radio stations

#define LOGGING_PERIOD    60            // defaults a 60 sec logging period

//----------------------------------------------------------------------------------------------+
//                                     Globals
//----------------------------------------------------------------------------------------------+

// These hold the local values that have been read from EEPROM
unsigned long LoggingPeriod;            // mS between writes to serial

// variables for counting periods and counts . . .
unsigned long logPeriodStart;           // for logging period
unsigned long radioPeriodStart;         // interval for checking signal strength
unsigned long ledPeriodStart;           // interval for updating the discrete LEDs

boolean cwTransmitEnabled = false;      // enables CW transmitter when set to true
byte ledMode = 0;                       // current mode for the LED - see defines above
static byte lastR, lastG, lastB = {0};  // stores the current RGB values for the LED

byte TCCR1A_default;                    // stores the value of the TCCR1A register before we mess with it
byte TCCR1B_default;                    // stores the value of the TCCR1B register before we mess with it
byte OCR1A_default;                     // stores the value of the OCR1A register before we mess with it

RDA5807 rx;                             // FM radio module

MorseSender *morseSender;               // Morse code transmitter

#if __has_include("ctf.h")
  #include "ctf.h"
#else
  #define CTF_SECRET_MESSAGE_PREAMBLE   F("      C C C ")
  #define CTF_SECRET_MESSAGE	      F("greetings qcc0x3 attendee B 73 de abend S")
#endif
