# qcc-0x3-lite-standard-badge

<b>Badge Summary</b>

The QCC 0x3 standard badge contains an RDA5807M FM radio receiver, an RGB LED, 4 discrete single color LEDs, and an ATmega328P microcontroller to manage it all.  The microcontroller runs at 8MHz and has been flashed with an Arduino bootloader, so you can use the Arduino IDE to modify the code and upload it to the badge with a USB to TTL serial dongle.  The badge is compatible with an Arduino Pro Mini (8MHz/3.3v), so choose that in the IDE as your board type.

<i><b>NOTE FOR CTF PARTICIPANTS:</b> The badge contains a flag which is not present in the public release of the source code.  If you are planning to work on the CTF badge challenge, I highly recommend you DO NOT re-flash the badge until you've completed it.</i>

<b>Badge Operation</b>

When the badge powers on, the central RGB LED will change colors based on signal strength reported by the RDA5807M for the currently tuned frequency.  In addition, the 4 standard LEDs around the circumference of the badge will blink in a pattern which will increase in speed as the signal strength increases.

To listen to low power signals at the conference, plug in the included earbuds.  Pressing SW4 will cause the RDA5807M to step to the next frequency. The frequency band is 3MHz wide and each step is 200kHz, so 15 presses of SW4 will cycle you back to where you started. Pressing SW2 and SW3 will adjust the volume up and down, respectively.  To make the radio receiver useful after the conference, you'll want to change the code that limits its frequency band.  For more information, refer to the <a href=https://pu2clr.github.io/RDA5807/extras/apidoc/html/index.html>RDA5807 Arduino Library documentation</a>.

To initiate the hidden signal for the CTF challenge, press SW2 and SW3 simultaneously.  The LED will glow a dim red color when the signal is being emitted.  To return to normal operation, press SW4 or try turning the badge off and then on again.

<b>Alternate LED modes</b>

There are a few different modes of operation for the LED.

If you press and hold SW2 while powering on the badge, the RGB LED will flash random colors about every 100ms.

If you press and hold SW3 while powering on the badge, the RGB LED will cycle through the color spectrum continuously.

If you press and hold SW4 while powering on the badge, the LEDs will show the signal strength of the currently tuned radio signal (this is the default).

All of these settings are persistent - your badge will keep the setting even if power is cycled.
