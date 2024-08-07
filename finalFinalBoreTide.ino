
#include "pwmWrite.h"
#include <FastLED.h>
#include <Wire.h> // Required for RTClib
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
#define LED_PIN     D8
#define NUM_LEDS    33
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
// Real Time Clock setup
RTC_DS3231 rtc;  // Uncomment when using this chip
// Tide calculation library setup.
// Change the library name here to predict for a different site.
#include "TidelibAnchorageKnikArmCookInletAlaska.h"
// Other sites available at http://github.com/millerlp/Tide_calculator
TideCalc myTideCalc; // Create TideCalc object called myTideCalc
bool changer = 1;
int slope = 0;
int oldSlope = 0;
 float highKeep = 0.0;
 float lowKeep = 0.0;
 float speed = 140.0;
 unsigned long startTime;
 const unsigned long travelTime = 60000; // 60 seconds to travel across 35 LEDs
int currMinute; // Keep track of current minute value in main loop
float results; // results holds the output from the tide calc. Units = ft.
DateTime oldTime;
DateTime newTime;
int hoursDiff = 5;
uint32_t lowTideTime;
//*******************************************************************
// Welcome to the setup loop
const int servoPin = D7;
// Parameters for the moving green LEDs
Pwm pwm = Pwm();
float ke = 0.6;
// duration of travel in ms is degrees to move / speed * 1000
float dur = 180.0 / speed * 1000.0;
float ye;
const int groupSize = 3; // Number of green LEDs in the group
const int groupSpacing = 1; // Space between the green LEDs in the group

// Very dark red and very dark blue color palettes
CRGBPalette16 redPalettes[] = {
    CRGBPalette16(CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed),
    CRGBPalette16(CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon),
    CRGBPalette16(CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson),
    CRGBPalette16(CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed, 
                  CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed, CRGB::Crimson, 
                  CRGB::Maroon, CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, 
                  CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed)
};

CRGBPalette16 bluePalettes[] = {
    CRGBPalette16(CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue),
    CRGBPalette16(CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy),
    CRGBPalette16(CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue),
    CRGBPalette16(CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue, 
                  CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue, CRGB::MidnightBlue, 
                  CRGB::Navy, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, 
                  CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue)
};

const int numRedPalettes = sizeof(redPalettes) / sizeof(CRGBPalette16);
const int numBluePalettes = sizeof(bluePalettes) / sizeof(CRGBPalette16);
const int paletteChangeInterval = 3000; // Time to change palette in milliseconds
unsigned long lastPaletteChange = 0;
unsigned long transitionStartTime = 0;
int currentRedIndex = 0;

// Function to create a dynamic palette with smooth transitions
CRGB getBlendedColor(CRGBPalette16& currentPalette, CRGBPalette16& nextPalette, uint8_t index, uint8_t blendFactor) {
    CRGB color1 = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    CRGB color2 = ColorFromPalette(nextPalette, index, 255, LINEARBLEND);
    return blend(color1, color2, blendFactor);
}


void applyBlendedPalette(CRGBPalette16& currentPalette, CRGBPalette16& nextPalette, uint8_t blendFactor) {
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t colorIndex = map(i, 0, NUM_LEDS, 0, 255);
        leds[i] = getBlendedColor(currentPalette, nextPalette, colorIndex, blendFactor);
    }
}
void setup(void)
{
  Wire.begin();
  rtc.begin();
  //rtc.adjust(DateTime(2024, 8, 1, 14, 17, 0));
  // For debugging output to serial monitor
  Serial.begin(115200); // Set baud rate to 115200 in serial monitor
  delay(2000);
  //*************************************
  DateTime now = rtc.now(); // Get current time from clock
  currMinute = now.minute(); // Store current minute value
  printTime(now);  // Call printTime function to print date/time to serial
  Serial.println("Calculating tides for: ");
  Serial.print(myTideCalc.returnStationID());
  Serial.print(" ");
  Serial.println(myTideCalc.returnStationIDnumber());

  // Calculate new tide height based on current time
  results = myTideCalc.currentTide(now);

  //*****************************************
  // For debugging
  Serial.print("Tide height: ");
  Serial.print(results, 3);
  Serial.println(" ft.");
  Serial.println(); // blank line
  delay(2000);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(255);
    startTime = millis();
    transitionStartTime = millis();
  
}  // End of setup loop

//********************************************
// Welcome to the main loop
void loop(void){
EVERY_N_SECONDS(300){
        hilow();
      }
  // Get the current time in milliseconds
    unsigned long currentMillis = millis();

    // Determine the current and next palette indices for blue and red palettes
    int currentBluePaletteIndex = (currentMillis / paletteChangeInterval) % numBluePalettes;
    int nextBluePaletteIndex = (currentBluePaletteIndex + 1) % numBluePalettes;
    int currentRedPaletteIndex = (currentMillis / paletteChangeInterval) % numRedPalettes;
    int nextRedPaletteIndex = (currentRedPaletteIndex + 1) % numRedPalettes;

    // Determine the blend factor based on the time within the current interval
    uint8_t blendFactor = (currentMillis % paletteChangeInterval) * 255 / paletteChangeInterval;

    // Apply the blended blue palette to all LEDs initially
    applyBlendedPalette(bluePalettes[currentBluePaletteIndex], bluePalettes[nextBluePaletteIndex], blendFactor);

    // Calculate the position of the first green LED in the group
    unsigned long elapsedTime = currentMillis - startTime;
    int greenLedStartPos = map(elapsedTime % travelTime, 0, travelTime, 0, NUM_LEDS - 1);

    // Set the green LEDs in the group
    

    // Handle the transition from blue to red
    /*
    if (currentMillis - transitionStartTime >= 60000) { // 1 minute interval
        if (currentRedIndex < NUM_LEDS) {
            currentRedIndex++;
            transitionStartTime = currentMillis;
        } else {
            currentRedIndex = 0;
            transitionStartTime = currentMillis;
        }
    }
*/
currentRedIndex = map(hoursDiff, 0,12,NUM_LEDS,0);
    // Apply the red palette based on the current red index
   // Serial.print("currentRedIndex:");
    //Serial.println(currentRedIndex);
    //currentRedIndex = 0;
    for (int i = 0; i < currentRedIndex; i++) {
        uint8_t colorIndex = map(i, 0, NUM_LEDS, 0, 255);
        leds[i] = getBlendedColor(redPalettes[currentRedPaletteIndex], redPalettes[nextRedPaletteIndex], colorIndex, blendFactor);
    }
    for (int i = 0; i < groupSize; i++) {
        int pos = (greenLedStartPos + i * groupSpacing) % NUM_LEDS;
        leds[pos] = CRGB::Green;
    }

    // Update the display
    FastLED.show();

    delay(30); // Adjust the delay to control the update speed
} 
  



//*******************************************
// Function for printing the current date/time to the
// serial port in a nicely formatted layout.
void printTime(DateTime now) {
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC); 
  Serial.print("/");
  Serial.print(now.day(), DEC); 
  Serial.print("  ");
   Serial.print(now.hour(), DEC); 
  Serial.print(":");
  if (now.minute() < 10) {
    Serial.print("0");
    Serial.print(now.minute());
   }
  else if (now.minute() >= 10) {
    Serial.print(now.minute());
  }
  Serial.print(":");
  if (now.second() < 10) {
    Serial.print("0");
    Serial.println(now.second());
  }
  else if (now.second() >= 10) {
    Serial.println(now.second());
  }
} // End of printTime function
//*************************************
void hilow(){
  changer = 1;
  lowKeep = 0;
  highKeep = 0;
  lowTideTime = 0;
  oldSlope = 0;
  oldTime = rtc.now();
  while(changer){
   
   newTime = oldTime + TimeSpan(0,0,5,0);
   float tideNow =  myTideCalc.currentTide(oldTime);
   float tideFuture =  myTideCalc.currentTide(newTime);
  if( (tideFuture - tideNow) > 0 ) {
    slope = 1;
  }
  else {slope = -1;}
  //Serial.print("slope:");
  //Serial.print(slope);
  if(oldSlope == 0)oldSlope = slope;
  
  
  if (oldSlope != slope) {
   if (oldSlope == -1) {
    Serial.print("Low Tide :");
    printTime (oldTime);
    Serial.print("Ht:");
    Serial.println(tideNow);
    Serial.print("unixTime");
    Serial.println(oldTime.unixtime());
    lowTideTime = oldTime.unixtime();  
    lowKeep = tideNow;
    delay(5000);
   }
    else {Serial.print("High Tide :");
    printTime (oldTime);
    Serial.print("Ht:");
    Serial.println(tideNow);
    highKeep = tideNow;
    delay (5000);
    //boreTide();
  }
  
  }
  if (lowKeep == 0 && highKeep != 0) highKeep = 0;
  if(lowKeep != 0 && highKeep != 0) {
    boreTide();
    changer = 0;
  }
 oldSlope = slope;
 oldTime = newTime;
  
  }
  
  }
void boreTide(){
  parseTime();
  uint32_t keyTime = lowTideTime - rtc.now().unixtime();
   hoursDiff = keyTime/3600;
  Serial.print("hours to Bore:");
  Serial.println( hoursDiff );
long tidalRange = highKeep - lowKeep;
Serial.print("range:");
Serial.println(tidalRange);
if (tidalRange < 28){
  Serial.println("BAD BORE");
} else if(tidalRange < 30){
  Serial.println("GOOD BORE");
} else if(tidalRange >= 30){
  Serial.println("BIG BORE");
}
constrain(tidalRange, 25, 30);
long q = map(tidalRange, 25, 30, 30, 190);
pwm.writeServo(servoPin, q, speed, ke);

highKeep = 0.0;
lowKeep = 0.0;
}
void parseTime(){
  DateTime now = rtc.now();
  if((now.month() >= 3 && now.day() >= 10) && (now.month() <= 11 && now.day() <= 3) ) {
    lowTideTime = lowTideTime + 7200;
  } else lowTideTime = lowTideTime + 3600;
}