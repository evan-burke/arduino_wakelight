// sourced from the Adafruit RTC clock example but modified slightly by vcmc

// Clock example using a seven segment display & DS1307 real-time clock.
//
// Must have the Adafruit RTClib library installed too!  See:
//   https://github.com/adafruit/RTClib
//
// Designed specifically to work with the Adafruit LED 7-Segment backpacks
// and DS1307 real-time clock breakout:
// ----> http://www.adafruit.com/products/881
// ----> http://www.adafruit.com/products/880
// ----> http://www.adafruit.com/products/879
// ----> http://www.adafruit.com/products/878
// ----> https://www.adafruit.com/products/264
//
// Adafruit invests time and resources providing this open source code, 
// please support Adafruit and open-source hardware by purchasing 
// products from Adafruit!
//
// Written by Tony DiCola for Adafruit Industries.
// Released under a MIT license: https://opensource.org/licenses/MIT
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "RTClib.h"


// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70


// Create display and DS1307 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();
RTC_DS1307 rtc = RTC_DS1307();

// Keep track of the hours, minutes, seconds displayed by the clock.
// Start off at 0:00:00 as a signal that the time should be read from
// the DS1307 to initialize it.
int hours = 0;
int minutes = 0;
int seconds = 0;


// these are part of checks to ensure we poll RTC regularly but not too often
long unixtime = 0;
long lastUpdate = 0;
long timeSinceLastUpdate = 5000;


// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
// initial value:
bool blinkColon = true;

// display brightness, from 0 to 15: (initial)
int displayBrightness = 4;

// input pin for light sensor used for display dimming:
int inputPin = A1;

//manageBrightness function values
int brightVal = 0; // variable used for checking brightness
const int numReadings = 6;    // how many readings to average, one per second
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int br_total = 0;                  // the running total
int average = 0;                // the average


// -------------------------------------------

void printTime() {
      // Print out the time to serial for debug purposes:
      DateTime now = rtc.now();
      Serial.print("Read date & time from DS1307: ");
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
}

// -------------------------------------------

void manageBrightness() {
  brightVal = 1023 - analogRead(inputPin);
  // This is set up in the circuit so that max brightness brings the input pin low; low brightness brings it high.
  // 10k resistor from vcc to analog in & 1 pin of photoresistor; other pin of photores to gnd
  // We subtract the read value from 1023 to get a number whigh increases with higher brightness. 

// probably need to build some kind of gamma map here? maybe. unless the photoresistor is already on a log scale. 

// also smoothing function to prevent oscillation? smarter than this maybe:

// basic smoothing:
  // subtract the last reading:
  br_total = br_total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = brightVal;
  // add the reading to the total:
  br_total = br_total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = (br_total / numReadings);

  // while 0-15 is the range of the 7 segment display, in practice higher values are far too bright, so limit this to a smaller range.
  // 0-10 is probably decent
  int bright4b = map(average, 0, 1023, 0, 13);

  // we tend to be too sensitive at the low end of the range, so dial it back here
  if ( bright4b >= 2 ) { 
    bright4b = bright4b - 2; 
  } else {
    bright4b = 0; 
  }

  // debug: 
  if ( (seconds % 20) == 0 && Serial ) {
    Serial.print("brightness input avg: "); Serial.print(average); 
    Serial.print("  cur bright: ");   Serial.print(brightVal);
    Serial.print("  mapped to 0-15: "); Serial.println(bright4b);
    delay(100);
  }

  clockDisplay.setBrightness(bright4b);

}


// -------------------------------------------

void setup() {
  // Setup function runs once at startup to initialize the display
  // and DS1307 clock. 

  // Setup Serial port to print debug output.
  if (Serial) {
    Serial.begin(9600);
    Serial.println("Clock starting!");
    Serial.print("compiler current time: ");
    Serial.print(__DATE__); Serial.print(" ");
    Serial.println(__TIME__);
  }

  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.setBrightness(displayBrightness);
  clockDisplay.print(10000); // displays "- - - -" due to value out of range
  clockDisplay.writeDisplay();
  delay(100);

  // Setup the DS1307 real-time clock.
  rtc.begin();

// TODO: compare clock time to system time and set it if it's greater than a few minutes off

  // Set the DS1307 clock if it hasn't been set before.
  bool setClockTime = !rtc.isrunning();
  // Alternatively you can force the clock to be set again by
  // uncommenting this line:
  //setClockTime = true;
  if (setClockTime) {
    if (Serial) {
     Serial.println("Setting DS1307 time!");
    }
    // This line sets the DS1307 time to the exact date and time the
    // sketch was compiled:
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Alternatively you can set the RTC with an explicit date & time, 
    // for example to set January 21, 2014 at 3am you would uncomment:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    Serial.print("Time set to: ");
    printTime();
    Serial.println();
  }

// used in manageBrightness:
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  
} // end setup

// -------------------------------------------

void loop() {
  // Loop function runs over and over again to implement the clock logic.
  
  // figure out how long it's been since we last polled the RTC
  if ( unixtime > 100 ) {  
     timeSinceLastUpdate = ( unixtime - lastUpdate );

  //debug stuff:
     //if ((unixtime % 15) == 0) {
     //  Serial.print("time since last update: "); Serial.print(timeSinceLastUpdate); 
     //  Serial.print("  minute:sec "); Serial.print(minutes); Serial.print(":"); Serial.println(seconds);
     // //Serial.print("lastUpdate: "); Serial.print(lastUpdate); Serial.print("   unixtime: "); Serial.println(unixtime);
     //}
   } else {
    if (Serial) {
      Serial.println("Pulling time from RTC upon startup or possible error");
    }
     // just in case this doesn't get set elsewhere, somehow
     if ( timeSinceLastUpdate < 100 ) {
       timeSinceLastUpdate = 5000;
     }
   }

  // if it's been a bit since we got the latest, do an update
  if (timeSinceLastUpdate > 180 ) {
    // Get the time from the DS1307.
    DateTime now = rtc.now();

    if (Serial) { 
      // Print out the time for debug purposes:
      printTime();
    }
    // Now set the hours and minutes.
    hours = now.hour();
    minutes = now.minute();
    seconds = now.second();
    unixtime = now.unixtime();
    lastUpdate = now.unixtime();
  }

  // Show the time on the display by turning it into a numeric
  // value, like 3:30 turns into 330, by multiplying the hour by
  // 100 and then adding the minutes.
  int displayValue = hours*100 + minutes;

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hours > 12) {
      displayValue -= 1200;
    }
    // Hour 0 (midnight) should be shown as 12.
    else if (hours == 0) {
      displayValue += 1200;
    }
  }

  // Now print the time value to the display.
  clockDisplay.print(displayValue, DEC);
  
  // Also pad when the 10's minute is 0 and should be padded.
  if (minutes < 10) {
    // digit 2 on the display is the colon; 3 is actually the 3rd digit
    clockDisplay.writeDigitNum(3, 0);
  }

  // Blink the colon by flipping its value every loop iteration
  // (which happens every second).
  blinkColon = !blinkColon;
  clockDisplay.drawColon(blinkColon);

  // Adjust brightness if needed. You should probably comment this out if you're not using it, or brightness may get weird due to floating analog pin. 
  manageBrightness();

  // Now push out to the display the new values that were set above.
  clockDisplay.writeDisplay();

  // Pause for a second for time to elapse.  This value is in milliseconds
  // so 1000 milliseconds = 1 second.
  delay(1000);

  // Now increase the seconds by one.
  seconds += 1;
  unixtime += 1;
  // If the seconds go above 59 then the minutes should increase and
  // the seconds should wrap back to 0.
  if (seconds > 59) {
    seconds = 0;
    minutes += 1;
    // Again if the minutes go above 59 then the hour should increase and
    // the minutes should wrap back to 0.
    if (minutes > 59) {
      minutes = 0;
      hours += 1;
      // Note that when the minutes are 0 (i.e. it's the top of a new hour)
      // then the start of the loop will read the actual time from the DS1307
      // again.  Just to be safe though we'll also increment the hour and wrap
      // back to 0 if it goes above 23 (i.e. past midnight).
      if (hours > 23) {
        hours = 0;
      }
    }
  }

  // Loop code is finished, it will jump back to the start of the loop
  // function again!
 }
