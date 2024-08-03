
#include "pwmWrite.h"
#include <Wire.h> // Required for RTClib
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS1307 rtc;  // Uncomment when using this chip
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
int currMinute; // Keep track of current minute value in main loop
float results; // results holds the output from the tide calc. Units = ft.
DateTime oldTime;
DateTime newTime;
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
}  // End of setup loop

//********************************************
// Welcome to the main loop
void loop(void){
EVERY_N_SECONDS(300){
        hilow();
      }
  
  
} // End of main loop 


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
  oldTime = rtc.now();
  while(changer){
   
   newTime = oldTime + TimeSpan(0,0,5,0);
   float tideNow =  myTideCalc.currentTide(oldTime);
   float tideFuture =  myTideCalc.currentTide(newTime);
  if( (tideFuture - tideNow) > 0 ) {
    slope = 1;
  }
  else {slope = -1;}
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
  if(lowKeep != 0 && highKeep != 0) boreTide();
 oldSlope = slope;
 oldTime = newTime;
 
  }
  
  }
void boreTide(){
  parseTime();
  uint32_t keyTime = lowTideTime - rtc.now().unixtime();
  int hoursDiff = keyTime/3600;
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
long q = map(tidalRange, 25, 30, 1, 180);
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