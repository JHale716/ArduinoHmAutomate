//
//
//   This is code for WEMOS D1 with SHT30 port 0x45 using Blynk cloud solution for data representation
//   Temp high, return, and low, for web hooks
//   Time keeping for management.
//
//   Configuration Data to consider
//   Using Multiple temp/time keepers you need to have multiple assignments of Blynk V Pin's to seperate data
//   Serial terminal redirect to Blynk (Need to jumper Tx-Rx and remove for uploads)
//
//   If using the SHT30 as a shield diret connected to the D1 mini the following offsets need to be applied.
//   const float offsetTemp = 1.75;   Adjusted to 1.8035555 7/4/19   //   temp in deg C that the device creates (quick and dirty)
//   const float offsetHumid = 6.1147625; Adjusted to 5.9766513 7/4/19   //   Humidity in % that the device reduces (quick and dirty)
//
//   Home
//   D1 Mini 1 MAC: <xx:xx:xx:xx:xx:xx> - MAC Address is reversed
//   D1 Mini 2 MAC: 
//   D1 Mini 3 MAC: 
//   D1 Mini 4 MAC: 
//   D1 Mini 5 MAC: 
//
//   D1 2 MAC: 
//
//   Office
//   D1 Mini 1 MAC: 
//   D1 Mini 2 MAC: 
//   D1 Mini 3 MAC: 
//   D1 Mini 4 MAC: 
//   D1 5 MAC: 
//
//   char auth1[] = "1";   //   Blynk API Code Work
//   char auth2[] = "";   //   Blynk API Code Home
//
int Version = 100;
//   by JP Hale jhale716@gmail.com
//
#include <ESP8266WiFi.h>   //   load the library for WiFi
#include <WiFiUdp.h>   //   load the UPD protocol
#include <NTPClient.h>   //   load the Library for NTP Time
#include <Time.h>   //   load the Library for Time
#include <TimeLib.h>   //   Blynk Time Library
#include <Wire.h>   //   load the library for I2C
#include <BlynkSimpleEsp8266.h>   //   load the library for Blynk
#include <WidgetRTC.h>   //   Blynk RTC Library
#include <ESP8266WebServer.h>   //   Load the Library for Web Server Services
#include <sunMoon.h>   //   SunMoon Library for sunrise and Sunset
//
#define BLYNK_PRINT Serial   //   you can comment out, its for terminal window only
#define Addr 0x45   //   WEMOS SHT30 I2C address is 0x45
#define turn_On 0   //   Addressing the inverted Wemos logic for the LED
#define turn_Off 1   //   Addressing the inverted Wemos logic for the LED
//
//   User Configurable Data, User Data Required
//
//   Critial Personalisation Data
//
//   Webhooks API Key
char iftttKey[] = "";   //   FILL in your Webhooks code API Key
//
//   Blynk API Key
char auth[] = "";   //   FILL in your BLYNK code
//
//   Username and password details, 4 Authorised WiFi networks possible, device will connect to the strongest signal
const String wifi1ssid = "";   //   Authorised WiFi Network 1 SSID
const String wifi1pswd = "";   //   Authorised WiFi Network 1 Password
const String wifi2ssid = "";   //   Authorised WiFi Network 2 SSID
const String wifi2pswd = "";   //   Authorised WiFi Network 2 Password
const String wifi3ssid = "";   //   Authorised WiFi Network 3 SSID
const String wifi3pswd = "";   //   Authorised WiFi Network 3 Password
const String wifi4ssid = "";   //   Authorised WiFi Network 4 SSID
const String wifi4pswd = "";   //   Authorised WiFi Network 4 Password
//
//   Enable Blinking of the LED, 1 on or 0 off you may want to switch it off in sleeping areas.
byte blinkLEDA[] = { 0, 1 , 1 , 1 , 0 , 0 };   //   Blink LED Array Position 0 - 5 1 = on 0 = off during the night, first position is 0 and is ignored
//
//   Time Zone Setup
const char timeZone[] = "NZST";   //   Time Zone Abreviation https://en.wikipedia.org/wiki/List_of_time_zone_abbreviations
const byte utcOffset = 12;   //   UTC Offset of Time Zone -12 - +12
const byte dstInTimeZone = true;   //   Does the timezone have DST
const byte monthDSTStarts = 9;   //   Month number for Daylight Saving Start
const byte dstWeekStarts = 4;   //   Week that DST starts in the month 1 = 2st, 2 = 2nd, 3 = 3rd, 4 = last
const byte monthDSTEnds = 4;   //   Month number for Daylight Saving End
const byte dstWeekEnds = 1;   //   Week that DST ends in the month 1 = 2st, 2 = 2nd, 3 = 3rd, 4 = last
//   Define location
#define latitude    -36.8485   //   Auckland, NZ cordinates
#define longtitude  174.7633
//
//   Timer Periods Control
byte homeMorningStartHr = 6;   //   Hour of the day the Morning Part off the Day Starts
byte homeMorningFinishHr = 8;   //   Hour of the day the Morning Part off the Day Finishes
byte homeEveningStartHr = 17;   //   Hour of the day the Evening Part off the Day Starts
byte homeEveningFinishHr = 22;   //   Hour of the day the Evening Part off the Day Finishes
byte homeNightStartHr = 22;   //   Hour of the day the Night Part off the Day Starts
byte homeNightFinishHr = 5;   //   Hour of the day the Night Part off the Day Finishes
byte nightShiftTemp = 2; // Number of degrees to reduce temperature while sleeping
byte workStartHr = 9;   //   Hour of the day the Work Day Starts
byte workFinishHr = 17;   //   Hour of the day the Work Day Finishes
//
//   Offset to account for heat and dryness created by having the sensor built into and connected to the shield
float offsetTemp = 1.8035555;   //   temp in deg C that the device creates (quick and dirty)
float offsetHumid = 5.9766513;   //   Humidity in % that the device reduces (quick and dirty)
//
//   Enable Lamp Control, 1 Yes or 0 No.
byte lampControl[] = { 0, 1, 1, 1, 0, 0 };   //   Blink LED Array Position 0 - 5, first position is 0 and is ignored
//  Lamp Timer Control
byte lampGapMinsSet = 60 ;   //   The number of minutes before or after sunrise/sunset that the lamp operates
byte lampWinterExt = -15;   //   Winter extension of morning turn off and evening turn on
// MagicMirror Setup Reporting
char* mmHost = "";   //   Magic Mirror Web Hook URL/ip Address for the MMM-system plugin
//
//   24 Hour Triggers used for extremes and when a timer program is not in use
//   All Temperature notation is 2 digit decimal ie. 24 degrees is 2400 and 24.5 degrees is 2450
//
//   Temperature triggers for timed use, should operate inside the 24 hour triggers
//
//   Device Configuration MAC Address, Temp Sensor connected direct or at distance Timer Control Situation,
//   Type of Device Control; The D1 controls both on/off and temperature or Device Controls once it is on or off
//   Situation 1 = Living Area used during the day every day, Situation 2 = Living Area used during the day weekends only,
//   Situation 3 = Bedroom Heated mornign, Eveing and over night, Situation 4 Work Environment, work hours only
//
//   Device 1 Configuration
const String d1MAC = "";   //   lower case MAC address as presented in monitor from load and first run
const String d1Location = "";
byte d1Situation = 2;   //   Situation for Hours, 1 Home daytime morning & evening, 2 home morning & night, 3 home morning, evening and night, 4 work.
const byte d1Control = 1;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
const byte d1Shield = 1;   //   Shield connected to processor 1 or remote to processor 0; manages temperature / humidity offset
//
const String iftttEvent1High = "";   //   trigger Event name for High Temperature
const String iftttEvent1HNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent1Low = "";   //   trigger Event name for Low Temperature
const String iftttEvent1LNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent1OnW = "";   //   trigger Event name to turn on for Winter leave "" if not used
const String iftttEvent1OnS = "";   //   trigger Event name to turn on for Summer leave "" if not used
const String iftttEvent1On = "";   //   trigger Event name to turn on
const String iftttEvent1Off = "";   //   trigger Event name to turn off
//
const String iftttEvent1LampOn = "";   //   trigger Event name to turn on
const String iftttEvent1LampOff = "";   //   trigger Event name to turn off
int temp24Hr1HighTrigger = 2700;   //   deg C High Temperature where the webhook fires
int temp24Hr1HighNormalTrigger = 2500;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr1LowNormalTrigger = 1800;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr1LowTrigger = 1600;   //   deg C Low Temperature where the webhook fires
int tempTimed1HighTrigger = 2600;   //   deg C High Temperature where the webhook fires
int tempTimed1HighNormalTrigger = 2400;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed1LowNormalTrigger = 2350;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed1LowTrigger = 2250;   //   deg C Low Temperature where the webhook fires
const byte blynkPort1ControlLED = 21;   //   Blynk Port for D1 Control LED Widget
const byte blynkPort1ControlOutput = 26;   //   Blynk Port for D1 Control Switch Widget
const byte blynkPort1Lamp = 31;   //   Blynk Port for D1 Lamp LED Widget
const byte blynkPort1lampCtl = 240;   //   Blynk Port for D1 Lamp Control Switch Widget
const byte blynkPort1Timed = 36;   //   Blynk Port for D1 Timed LED Widget
const byte blynkPort1WKNDOR = 41;   //   Blynk Port for D1 Timed Switch Widget
const byte blynkPort1Reset = 245;   //   Blynk Port for D1 Push Switch Widget
const byte blynkPort1blueLED = 250;   //   Blynk Port for D1 Blue LED Widget
byte lampMorning1On = 4;
byte lampEvening1Off = 22;
byte lampEvening1OnLatest = 18;
WidgetLED led11(blynkPort1blueLED);   //   Define Blynk Port for widget LED
WidgetLED led12(blynkPort1ControlLED);   //   Define Blynk Port for widget Controlled Device LED
WidgetLED led13(blynkPort1Timed);   //   Define Blynk Port for widget Timed Operation LED
WidgetLED led14(blynkPort1Lamp);   //   Define Blynk Port for widget Lamp Operation LED

//
//    Device 2 Configuration
const String d2MAC = "";   //  lower case MAC address as presented in monitor from load and first run
const String d2Location = "";
byte d2Situation = 2;   //   Situation for Hours, 1 Home daytime only, 2 home morning and evening, 3 home morning, evening and night, 4 work.
const byte d2Control = 1;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
const byte d2Shield = 1;   //   Shield connected to processor 1 or remote to processor 0; manages temperature / humidity offset
//
const String iftttEvent2High = "";   //   trigger Event name for High Temperature
const String iftttEvent2HNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent2Low = "";   //   trigger Event name for Low Temperature
const String iftttEvent2LNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent2OnW = "";   //   trigger Event name to turn on for Winter leave "" if not used
const String iftttEvent2OnS = "";   //   trigger Event name to turn on for Summer leave "" if not used
const String iftttEvent2On = "";   //   trigger Event name to turn on
const String iftttEvent2Off = "";   //   trigger Event name to turn off
//
const String iftttEvent2LampOn = "";   //   trigger Event name to turn on
const String iftttEvent2LampOff = "";   //   trigger Event name to turn off
int temp24Hr2HighTrigger = 2700;   //   deg C High Temperature where the webhook fires
int temp24Hr2HighNormalTrigger = 2500;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr2LowNormalTrigger = 1850;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr2LowTrigger = 1600;   //   deg C Low Temperature where the webhook fires
int tempTimed2HighTrigger = 2600;   //   deg C High Temperature where the webhook fires
int tempTimed2HighNormalTrigger = 2400;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed2LowNormalTrigger = 2350;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed2LowTrigger = 2250;   //   deg C Low Temperature where the webhook fires
const byte blynkPort2ControlLED = 22;   //   Blynk Port for D2 Control LED Widget
const byte blynkPort2ControlOutput = 27;   //   Blynk Port for D2 Control Switch Widget
const byte blynkPort2Lamp = 32;   //   Blynk Port for D2 Lamp LED Widget
const byte blynkPort2lampCtl = 241;   //   Blynk Port for D2 Lamp Control Switch Widget
const byte blynkPort2Timed = 37;   //   Blynk Port for D2 Timed LED Widget
const byte blynkPort2WKNDOR = 42;   //   Blynk Port for D2 Timed Switch Widget
const byte blynkPort2Reset = 246;   //   Blynk Port for D2 Push Switch Widget
const byte blynkPort2blueLED = 251;   //   Blynk Port for D2 Blue LED Widget
byte lampMorning2On = 4;
byte lampEvening2Off = 23;
byte lampEvening2OnLatest = 18;
WidgetLED led21(blynkPort2blueLED);   //   Define Blynk Port for widget LED
WidgetLED led22(blynkPort2ControlLED);   //   Define Blynk Port for widget Controlled Device LED
WidgetLED led23(blynkPort2Timed);   //   Define Blynk Port for widget Timed Operation LED
WidgetLED led24(blynkPort2Lamp);   //   Define Blynk Port for widget Lamp Operation LED
//
//    Device 3 Configuration
const String d3MAC = "";   //   lower case MAC address as presented in monitor from load and first run
const String d3Location = "";
byte d3Situation = 2;   //   Situation for Hours, 1 Home daytime only, 2 home morning and evening, 3 home morning, evening and night, 4 work.
const byte d3Control = 1;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
const byte d3Shield = 1;   //   Shield connected to processor 1 or remote to processor 0; manages temperature / humidity offset
//
const String iftttEvent3High = "";   //   trigger Event name for High Temperature
const String iftttEvent3HNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent3Low = "";   //   trigger Event name for Low Temperature
const String iftttEvent3LNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent3OnW = "";   //   trigger Event name to turn on for Winter leave "" if not used
const String iftttEvent3OnS = "";   //   trigger Event name to turn on for Summer leave "" if not used
const String iftttEvent3On = "";   //   trigger Event name to turn on
const String iftttEvent3Off = "";   //   trigger Event name to turn off
//
const String iftttEvent3LampOn = "";   //   trigger Event name to turn on
const String iftttEvent3LampOff = "";   //   trigger Event name to turn off
int temp24Hr3HighTrigger = 2700;   //   deg C High Temperature where the webhook fires
int temp24Hr3HighNormalTrigger = 2450;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr3LowNormalTrigger = 1800;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr3LowTrigger = 1600;   //   deg C Low Temperature where the webhook fires
int tempTimed3HighTrigger = 2600;   //   deg C High Temperature where the webhook fires
int tempTimed3HighNormalTrigger = 2400;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed3LowNormalTrigger = 2350;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed3LowTrigger = 2250;   //   deg C Low Temperature where the webhook fires
const byte blynkPort3ControlOutput = 28;   //   Blynk Port for D3 Control Switch Widget
const byte blynkPort3ControlLED = 23;   //   Blynk Port for D3 Control LED Widget
const byte blynkPort3Lamp = 33;   //   Blynk Port for D3 Lamp LED Widget
const byte blynkPort3lampCtl = 242;   //   Blynk Port for D3 Lamp Control Switch Widget
const byte blynkPort3Timed = 38;   //   Blynk Port for D3 Timed LED Widget
const byte blynkPort3WKNDOR = 43;   //   Blynk Port for D3 Timed Switch Widget
const byte blynkPort3Reset = 247;   //   Blynk Port for D3 Push Switch Widget
const byte blynkPort3blueLED = 252;   //   Blynk Port for D3 Blue LED Widget
byte lampMorning3On = 4;
byte lampEvening3Off = 23;
byte lampEvening3OnLatest = 18;
WidgetLED led31(blynkPort3blueLED);   //   Define Blynk Port for widget LED
WidgetLED led32(blynkPort3ControlLED);   //   Define Blynk Port for widget Controlled Device LED
WidgetLED led33(blynkPort3Timed);   //   Define Blynk Port for widget Timed Operation LED
WidgetLED led34(blynkPort3Lamp);   //   Define Blynk Port for widget Lamp Operation LED
//
//    Device 4 Configuration
const String d4MAC = "";   //   lower case MAC address as presented in monitor from load and first run
const String d4Location = "";
byte d4Situation = 3;   //   Situation for Hours, 1 Home daytime only, 2 home morning and evening, 3 home morning, evening and night, 4 work.
const byte d4Control = 1;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
const byte d4Shield = 1;   //   Shield connected to processor 1 or remote to processor 0; manages temperature / humidity offset
//
const String iftttEvent4High = "";   //   trigger Event name for High Temperature
const String iftttEvent4HNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent4Low = "";   //   trigger Event name for Low Temperature
const String iftttEvent4LNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent4OnW = "";   //   trigger Event name to turn on for Winter leave "" if not used
const String iftttEvent4OnS = "";   //   trigger Event name to turn on for Summer leave "" if not used
const String iftttEvent4On = "";   //   trigger Event name to turn on
const String iftttEvent4Off = "";   //   trigger Event name to turn off
//
const String iftttEvent4LampOn = "";   //   trigger Event name to turn on
const String iftttEvent4LampOff = "";   //   trigger Event name to turn off
int temp24Hr4HighTrigger = 2700;   //   deg C High Temperature where the webhook fires
int temp24Hr4HighNormalTrigger = 2500;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr4LowNormalTrigger = 1800;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr4LowTrigger = 1600;   //   deg C Low Temperature where the webhook fires
int tempTimed4HighTrigger = 2600;   //   deg C High Temperature where the webhook fires
int tempTimed4HighNormalTrigger = 2400;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed4LowNormalTrigger = 2350;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed4LowTrigger = 2250;   //   deg C Low Temperature where the webhook fires
const byte blynkPort4ControlOutput = 29;   //   Blynk Port for D4 Control Switch Widget
const byte blynkPort4ControlLED = 24;   //   Blynk Port for D4 Control LED Widget
const byte blynkPort4Lamp = 34;   //   Blynk Port for D4 Lamp LED Widget
const byte blynkPort4lampCtl = 243;   //   Blynk Port for D4 Lamp Control Switch Widget
const byte blynkPort4Timed = 39;   //   Blynk Port for D4 Timed LED Widget
const byte blynkPort4WKNDOR = 44;   //   Blynk Port for D4 Timed Switch Widget
const byte blynkPort4Reset = 248;   //   Blynk Port for D4 Push Switch Widget
const byte blynkPort4blueLED = 253;   //   Blynk Port for D4 Blue LED Widget
byte lampMorning4On = 4;
byte lampEvening4Off = 23;
byte lampEvening4OnLatest = 18;
WidgetLED led41(blynkPort4blueLED);   //   Define Blynk Port for widget LED
WidgetLED led42(blynkPort4ControlLED);   //   Define Blynk Port for widget Controlled Device LED
WidgetLED led43(blynkPort4Timed);   //   Define Blynk Port for widget Timed Operation LED
WidgetLED led44(blynkPort4Lamp);   //   Define Blynk Port for widget Lamp Operation LED
//
//    Device 5 Configuration
const String d5MAC = "";   //   lower case MAC address as presented in monitor from load and first run
const String d5Location = "";
byte d5Situation = 2;   //   Situation for Hours, 1 Home daytime only, 2 home morning and evening, 3 home morning, evening and night, 4 work.
const byte d5Control = 1;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
const byte d5Shield = 1;   //   Shield connected to processor 1 or remote to processor 0; manages temperature / humidity offset
//
const String iftttEvent5High = "";   //   trigger Event name for High Temperature
const String iftttEvent5HNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent5Low = "";   //   trigger Event name for Low Temperature
const String iftttEvent5LNormal = "";   //   trigger Event name for Return to Normal
const String iftttEvent5OnW = "";   //   trigger Event name to turn on for Winter leave "" if not used
const String iftttEvent5OnS = "";   //   trigger Event name to turn on for Summer leave "" if not used
const String iftttEvent5On = "";   //   trigger Event name to turn on
const String iftttEvent5Off = "";   //   trigger Event name to turn off
//
const String iftttEvent5LampOn = "";   //   trigger Event name to turn on
const String iftttEvent5LampOff = "";   //   trigger Event name to turn off
int temp24Hr5HighTrigger = 2700;   //   deg C High Temperature where the webhook fires
int temp24Hr5HighNormalTrigger = 2500;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr5LowNormalTrigger = 1800;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24Hr5LowTrigger = 1600;   //   deg C Low Temperature where the webhook fires
int tempTimed5HighTrigger = 2600;   //   deg C High Temperature where the webhook fires
int tempTimed5HighNormalTrigger = 2400;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed5LowNormalTrigger = 2350;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimed5LowTrigger = 2250;   //   deg C Low Temperature where the webhook fires
const byte blynkPort5ControlOutput = 30;   //   Blynk Port for D5 Control Switch Widget
const byte blynkPort5ControlLED = 25;   //   Blynk Port for D5 Control LED Widget
const byte blynkPort5Lamp = 35;   //   Blynk Port for D5 Lamp LED Widget
const byte blynkPort5lampCtl = 244;   //   Blynk Port for D5 Lamp Control Switch Widget
const byte blynkPort5Timed = 40;   //   Blynk Port for D5 Timed LED Widget
const byte blynkPort5WKNDOR = 45;   //   Blynk Port for D5 Timed Switch Widget
const byte blynkPort5Reset = 249;   //   Blynk Port for D5 Push Switch Widget
const byte blynkPort5blueLED = 254;   //   Blynk Port for D5 Blue LED Widget
byte lampMorning5On = 4;
byte lampEvening5Off = 23;
byte lampEvening5OnLatest = 18;
WidgetLED led51(blynkPort5blueLED);   //   Define Blynk Port for widget LED
WidgetLED led52(blynkPort5ControlLED);   //   Define Blynk Port for widget Controlled Device LED
WidgetLED led53(blynkPort5Timed);   //   Define Blynk Port for widget Timed Operation LED
WidgetLED led54(blynkPort5Lamp);   //   Define Blynk Port for widget Lamp Operation LED
//
//   Soft Reset for all devices
const byte blynkPortResetAll = 255;   //   Blynk Port for D5 Blue LED Widget
//
//   Variable Setup for timers Adjustable by user
const int intervalLED = 5000;   //   interval between blink LED (milliseconds) 1000 = 1 second
const int intervalWebHook = 5000;   //   interval between webhook (milliseconds) 1000 = 1 second
const int intervalProg = 300000;   //   interval at which to check sensor (milliseconds) 1000 = 1 second
const int intervalTimed = 300000;   //   interval at which to Manage Devices (milliseconds) 1000 = 1 second
const int delayReadTime = 1800000;   //   interval between wait for read NTP time (milliseconds) 1000 = 1 second
//   ReadTimeDelay Should be greater than 30 minutes as per NTP T&C's
const int delayUpdateATime = 1000;   //   interval between wait update of Arduino time (milliseconds) 1000 = 1 second
const int displayConfigDelay = 5000;   //   check for double tap from Blynk to display config summary
const int delayDisplayLocalUTC = 300;   //   Seconds delay between each display of current time
const int blynkSendSerialTimer = 50;   //   Timer duration for interrupt to run sent_serial function to send data to Blynk Terminal
//
//   External Communications
const byte httpPort = 80;   //   HTTP webserver port
const char* host = "maker.ifttt.com";   //   Web Service URL
const char *appassword = "";   //   Defaul t Password for ESP Access point
//
//   Variable Setup - Leave this alone
//
//   Loop constant timer variables
const long delayHalfSecond = 500;   //   interval for 1/2 second delay loop
const long delayOneSecond = 1000;   //   interval for 1 second delay loop
//   Loops millisecond timer variables
unsigned long timeElapsedLED = 0;   //   Create an Instance for the timer function for the LED
unsigned long timeElapsedBlynk = 0;   //   Create an Instance for the timer function for the Blynk Keep Alive
unsigned long timeElapsedTimed = 0;   //   Create an Instance for the timer function for the Timed Management
unsigned long timeElapsedProg = 0;   //   Create an Instance for the timer function for the Program
unsigned long timeElapsedWebHook = 0;   //   Create an Instance for the timer function for the WebHook
unsigned long timeElapsedReadTimeDelay = 0;   //   Create an Instance for the timer function for the WebHook
unsigned long currentMillis = 0;   //   Define timer variable for Blynk loop
unsigned long displayconfigtimer = 0;   //   Blynk Timer for determining to display config. Double Temp request within 10 seconds will display configuration
unsigned long delayLoopLEDTurnedOn = 0;   //   Define timer variable for Delay loop 1 (LED Blink)
unsigned long delayLoopArduinoTime = 0;   //   Define timer variable for Delay loop 2 (Timer for Queued multiple LED Blink)
unsigned long delayloopLEDTurnedOff = 0;   //   Define timer variable for Delay loop 3 (Local Timer for Arduino Time)
unsigned long watchdogLoop = 0;   //   Define timer variable for Delay loop 3 (Local Timer for Arduino Time)
unsigned long timepassed;   //   Load Timer
//
//   Loops Control variables
bool delayLEDTurnedOn = false;   //   Define test variable for Delay loop 1 (LED Blink)
bool delayLEDTurnedOff = false;   //   Define test variable for Delay loop 3 (Local Timer for Arduino Time)
bool watchdogRunning = false;   //   Define test variable for Delay loop 2 (Timer for Queued multiple LED Blink)
byte watchdogCounter = 0;
byte delayDisplayLocalTime = 0;   //   Arduino Time delay variable for calculate time
byte secondsElapsedTimeStamp = 0;   //   Arduino Time delay variable for display local time
//
//   Device Setup Variables
int temp24HrHighTrigger = 0;   //   deg C High Temperature where the webhook fires
int temp24HrHighNormalTrigger = 0;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24HrLowNormalTrigger = 0;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int temp24HrLowTrigger = 0;   //   deg C Low Temperature where the webhook fires
int tempTimedHighTrigger = 0;   //   deg C High Temperature where the webhook fires
int tempTimedHighNormalTrigger = 0;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimedLowNormalTrigger = 0;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
int tempTimedLowTrigger = 0;   //   deg C Low Temperature where the webhook fires
byte mac[6];   //   Variable to hold MAC address for sensor
String deviceMac;   //   Variable to hold MAC address for triggers
byte device;   //   Track the device in use value 1 - 5
byte d1MiniSituation = 0;   //   Situation for Hours, 1 Home daytime only, 2 home morning and evening, 3 home day time, evening and night, 4 work.
byte d1MiniControl = 0;   //   Type of web hook control 1 On/Off Switch 2 AirCon/Self Managed On Off required
String d1MiniLocation;   //   Location for reporting
char iftttEventHigh[25];   //   trigger Event name for High Temperature
char iftttEventHNormal[25];   //   trigger Event name for Return to Normal
char iftttEventLow[25];   //   trigger Event name for Low Temperature
char iftttEventLNormal[25];   //   trigger Event name for Return to Normal
char iftttEventOn[25];   //   trigger Event name to turn on
char iftttEventOff[25];   //   trigger Event name to turn off
char iftttEventLampOn[25];   //   trigger Event name to turn on
char iftttEventLampOff[25];   //   trigger Event name to turn off
bool blinkLED = true;   //   Variable to control the LED
//
//   System Control Variables
//
//   External Output Control
byte externalOutputOn = 0;   //   0 = no change, 1 = On, 2 = off
byte lampControlOn = 1;   //   0 = off, 1 = On
byte externalCTLOverride = 0;   //   Floating Variable for external interference;
byte externalTMOverride = 0;   //   Floating Variable for external interference;
//
//   System Status Tracking
byte currentMode = 0;   //   Tracking of mode to limit change triggers 0 = heating 1 = cooling
byte webHookLoaded = 0;   //   Test if webhook loaded to queue commands
byte externalDeviceState = 0;   //   External Device State Tracking (webhooks) -1=low on, 0=off, 1=high on
byte timedOperation = 0;   //   Variable to suspend 24hour return to normal triggers
byte lastADay;   //   Arduino Time variable for track change of Day
byte Season;
byte resetCountdown = 49;   //   Countdown for reboot / reset cycle in days
byte switchStateChangeCTL = 2;   //   Did the Blynk switch change?
byte switchStateChangeOR = 2;   //   Did the Blynk switch change?
byte beenHereOR1 = 0;   //   Weekend Override rest controls
byte beenHereOR2 = 0;   //   Weekend Override rest controls
byte homeEveningFinishHrORG;   //   Hour of the day the Evening Part off the Day Finishes
byte homeEveningStartHrORG;   //   Hour of the day the Evening Part off the Day Finishes
byte d1MiniSituationORG;   //   Weekend Override Holding Variable
//
//   System Setup Controls
byte firstRun = 1;   //   First Run variable
byte setupATime = 1;   //   Control variable for initial setup of Arduino Time
byte keepAlivePrintLN = 0;   //   Line control for print alive
//
//   Maker Service Variables
char post_rqst[256];   //   hand-calculated to be big enough
char *p;   //   Variable for building json string
char usewifissid[25];   //   WiFi SSID variable used for connection
char usewifipswd[25];   //   Wifi Password variable used for connection
char *apssid = "";   //   Define Access Point SSID for wifi Access point
//
//   Blynk
byte blynkPortT = 0;   //   Blynk Virtual Pin for the Temp data
byte blynkPortH = 0;   //   Blynk Virtual Pin for the Humidity data
byte blynkPortU = 0;   //   Blynk Virtual Pin to trigger sensor read and update
byte blynkPortS = 0;   //   Blynk Virtual Pin for Serial Terminal Widget
byte blynkPortReset = 0;   //   input port for reset
byte blynkPortOutput = 0;   //   input port for on/off of controlled output
byte blynkPortlampCtl = 0;   //   Load control variable when enabled
byte blynkPortTimed = 0;   //   Timed Control Switch
byte blynkPortWKNDOR = 0;   //   Weekend Override switch
byte bUpdate = 0;   //   Blynk Update Container
//
//   LED
bool ledOn = false;   //   Keep track of the led state
//
//   Time
unsigned long epoch;   //   epoch current time in seconds since 00:00:00 01/01/1970
unsigned long aSecsUpdate;   //   Arduino Time variable holding seconds since last update
unsigned long aMilli;   //   Arduino Time variable holding last milli at last update
int utcOffsetSeconds = 0;   //   0 seconds if UTC or GMT position, updated in void config()
byte daylightSavings = 0;   //   Daylight Savings indicator, default off
byte dstChanged = 0;   //   Daylight Savings changed indicator, default off
byte aDay;   //   Arduino Time variable for Day
byte aMth;   //   Arduino Time variable for Month
int aYr;   //   Arduino Time variable for Year
byte aHours;   //   Arduino Time variable for Hours
byte aMins;   //   Arduino Time variable for Minutes
byte aSecs;   //   Arduino Time variable for Seconds
byte aWDay;   //   Arduino Time variable for Day of Week
String weekdays[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
byte ledSync = 0;   //   Control Variable for LED Sync Control
byte weekendOverride = 0;   //   Override Control for holiday's and days off
//
//   Temp Sensor
float rCTemp;   //   Define cTemp Variable globally
int cTemp;   //   Integer for temperature, multiplied by 100 for floating point and rounding.
float rFTemp;   //   Define fTemp Variable Globally
int fTemp;   //   Integer for temperature, multiplied by 100 for floating point and rounding.
float humidity;   //   Define Humidity Variable Globally
//
//   Temp Control
byte highTempTriggered = 0;   //   integer variable initilisation for tracking High Temp Triggers
byte lowTempTriggered = 0;   //   integer variable initilisation for tracking Low Temp Triggers
byte timedOnTriggered = 0;   //   integer variable initilisation for tracking Timed Activity On Triggers
byte timedOffTriggered = 0;   //   integer variable initilisation for tracking Timed Activity Off Triggers
byte highTempReset = 0;   //   integer variable initilisation for tracking return to High Temp range
byte lowTempReset = 0;   //   integer variable initilisation for tracking return to Low Temp range
byte highTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of High Temp Trigger
byte lowTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of Low Temp Trigger
byte highTempResetSent = 0;   //   integer variable initilisation for tracking sending of High Temp Reset
byte lowTempResetSent = 0;   //   integer variable initilisation for tracking sending of Low Temp Reset
byte timedOnTriggerSent = 0;   //   integer variable initilisation for tracking sending of Timed Activities On
byte timedOffTriggerSent = 0;   //   integer variable initilisation for tracking sending of timed Activities Off
byte timedOnControlTriggerSent = 0;   //   integer variable initilisation for tracking sending of Timed Activities On
byte timedOffControlTriggerSent = 0;   //   integer variable initilisation for tracking sending of timed Activities Off
byte timedOnControlTriggered = 0;   //   integer variable initilisation for tracking Timed Activity On Triggers
byte timedOffControlTriggered = 0;   //   integer variable initilisation for tracking Timed Activity Off Triggers
byte timedHighTempTriggered = 0;   //   integer variable initilisation for tracking Timed High Temp Triggers
byte timedLowTempTriggered = 0;   //   integer variable initilisation for tracking Timed Low Temp Triggers
byte timedHighTempReset = 0;   //   integer variable initilisation for tracking return to Timed High Temp range
byte timedLowTempReset = 0;   //   integer variable initilisation for tracking return to Timed Low Temp range
byte timedHighTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of Timed High Temp Trigger
byte timedLowTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of Timed Low Temp Trigger
byte timedHighTempResetSent = 0;   //   integer variable initilisation for tracking sending of Timed High Temp Reset
byte timedLowTempResetSent = 0;   //   integer variable initilisation for tracking sending of Timed Low Temp Reset
byte nightShiftActive = 0;  // track Temp managment movement +/- 2 deg for night time
byte safetyHeating = 0;  // For tracking when Heating/Cooling has been active
byte safetyCooling = 0;  // Used to first an off for heating when Normal Cooling Temp hit and opposite for Cooling
byte mDay;   //   MoonDay Variable
time_t sRise;   //   Sunrise Time
time_t sSet;   //   Sunset Time
char forecast;   //   Forecast from mDay
//
//   Lamp Control
int lampOverRide = 0; // variable to manage lampControl Override
byte lampControlOnTrigger = 0;   //   integer variable initilisation for tracking Lamp on trigger
byte lampControlOffTrigger = 0;   //   integer variable initilisation for tracking Lamp off trigger
byte lampMorningOn;   //   Device Lamp Time Turn On Hour
byte lampEveningOff;   // Device Lamp Time Turn Off Hour
byte lampEveningOnLatest;  // Latest to turn lamp on
byte lampGapMins;   //   The number of minutes before or after sunrise/sunset that the lamp operates
byte shiftedlamptimehr;   //   Device Next Hour On or Off
byte shiftedlamptimemin;   //   Device Next Minutes On or Off
byte confSRiseH;
byte confSRiseM;
byte confSSetH;
byte confSSetM;
//
//   PreProcessing
WiFiUDP udp;   //   Define upd to be WiFiUDP
NTPClient timeClient(udp);   //   Setup NTP time Client
WidgetTerminal terminal(blynkPortS);   //   Define widget terminal Blynk Port/Pin
WidgetRTC rtc;   //   Define RTC to be Widget RTC
BlynkTimer timer;   //   Timer
ESP8266WebServer server(80);   //   Define WebServer
sunMoon  sm;   //   SunMoon Library Reference
//
//   Loops and Delays Section
//
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);   //   Setup still alive indicator (onboard LED)
  startupPause();
  Serial.begin(9600);   //   Setup serial monitor
  Serial.println();   //   print config to Terminal Screen
  printScreenLine(); Serial.println();
  Serial.println(F("Startup"));
  startupSparkle();
  Serial.println();
  Serial.println(F("Initialisation:"));
  WiFi.macAddress(mac);
  startupSparkle();
  individualiseSensor();
  Serial.print(F("Version")); printColon(); printSpace(); serialPrintDecimal(Version); Serial.println();
  printScreenLine(); Serial.println();
  listNetworks();
  setupESPAP();
  ConnectWifi();
  InitialiseTime();
  initialiseI2C();
  InitialiseBlynk();
  send_serial();
  printScreenLine(); Serial.println();
  startupSparkle();
  Serial.println();
  Serial.println(F("WEMOS Temp/Humidity IoT Started:"));   //   send to serial control messsage
  printScreenLine(); Serial.println();
  send_serial();
  Serial.println();
  Serial.println(F("Initial data check & send:"));   //   send to serial control messsage
  sendTemperature();   //   Initial data send
  startupSparkle();
  setupLamp();
  send_serial();
  InitialiseTimers();
  send_serial();
  blynkclockDisplay();
  startupSparkle();
  Serial.println();
  Serial.println(F("End of setup:"));   //   send to serial control messsage
  printScreenLine(); Serial.println();
  send_serial();
  currentMillis = millis();   //   Current Millis for measuring time from start of loop
  timerBlynk();   //   Contact/update Blynk
  startupSparkle();
  displayConfig();
  startupSparkle();
}

void loop() {
  currentMillis = millis();   //   Current Millis for measuring time from start of loop
  timerBlynk();   //   Contact/update Blynk
  controlCheckTime();
  checkRunTemperatureDelay();
  runTemperature();
  delayCheckTempTriggers();
  controlTheLamp();
  delayUpdateTime();
  blinkTheLEDCheck();
  server.handleClient();
  watchdogWebH();
  watchdogSafety();
  runTasks();
  checkReset();   //   Check for Reset
}

void runTasks() {
  //  Serial.println("runTasks");
  if (weekendOverride) {
    if (!beenHereOR1) {
      beenHereOR1 = 1;
      homeEveningFinishHrORG = homeEveningFinishHr;
      homeEveningStartHrORG = homeEveningStartHr;
      homeEveningFinishHr = 24;
      homeEveningStartHr = 1;
    }
    weekendTask();
  } else {
    if (aWDay >= 1 && aWDay <= 5) {
      weekdayTask();   //   Program for weekday
    } else {
      weekendTask();   //   Program for weekend
    }
  }
}

void delayUpdateTime() {   //   was DelayOne
  //  Serial.println("delayUpdateTime");
  if ((millis() - delayLoopArduinoTime) >= delayUpdateATime) {   //   update the time every second from elapsed miliseconds
    updateArduinoTime();
    if (aSecs != delayDisplayLocalTime) {   //   increment the display counter every 1 second
      secondsElapsedTimeStamp++;
      delayDisplayLocalTime = aSecs;
    }
    if (secondsElapsedTimeStamp >= delayDisplayLocalUTC) {   //   Every 30 seconds display the time
      secondsElapsedTimeStamp = 0;
      localUTCTime();
    }
  }
}

void watchdogWebH() {   //  Watchdog Timer - too many webhooks and it's a reset
  //  Serial.println("watchdogWebH");
  if (watchdogRunning) {
    if ((millis() - watchdogLoop) >= 60000) {
      watchdogRunning = false;
      watchdogCounter = 0;
    } else {
      if (watchdogCounter >= 5) {
        softResetD1();
      }
    }
  } else {
    if (watchdogCounter > 0) {
      watchdogLoop = millis();
      watchdogRunning = true;
    }
  }
}

void setupLamp() {   //  Assumption lamp and start state will be in on or off periods so set to off and test/control from there.
  //  Serial.println("SetupLamp");
  if (lampControl[device]) {
    lampControlOffTrigger = 1;
    lampControlOnTrigger = 0;
    sendWebhook();
    delay(500); // delay 1/2 second for next step to allow webhook to clear
    aMilli = aMilli + 500; // Add delay to time timer
    controlTheLamp();
    Serial.print(F("Lamp Control")); tabindent(3); variableIndent(); Serial.println(F("is Enabled"));
    Serial.print(F("Lamp Should Be")); tabindent(3); variableIndent();
    if (lampControlOn) {
      Serial.println(F("On"));
    } else {
      Serial.println(F("Off"));
    }
  } else {
    Serial.print(F("Lamp Control:")); tabindent(3); Serial.print(F("is Disabled "));
  }
  startupSparkle();
}

void controlTheLamp() {   //   lamp control
  //  Serial.println("controlTheLamp");
  if (lampControl[device]) {
    if (!lampOverRide) {
      if (firstRun) {
        if ((minute(sRise) + lampGapMins) < 60) {
          shiftedlamptimehr = hour(sRise);
        } else {
          shiftedlamptimehr = hour(sRise) + 1;
        }
        shiftedlamptimemin = (minute(sRise) + lampGapMins) % 60;
        confSRiseH = shiftedlamptimehr;
        confSRiseM = shiftedlamptimemin;
      }
      if (lampControlOn) {  // If the lamp is on then do the first half, if it is off then do the second half. Off by default at start up.
        if ((minute(sRise) + lampGapMins) < 60) {
          shiftedlamptimehr = hour(sRise);
        } else {
          shiftedlamptimehr = hour(sRise) + 1;
        }
        shiftedlamptimemin = (minute(sRise) + lampGapMins) % 60;
        if (((aHours >= shiftedlamptimehr && aMins >= shiftedlamptimemin) or aHours > shiftedlamptimehr) && aHours < (hour(sSet) - 1) && aHours < lampEveningOnLatest) {   //   Turn it off after sunrise
          lampControlOffTrigger = 1;
          lampControlOnTrigger = 0;
          sendWebhook();
        }
        if (aHours > lampEveningOff) { // Turn it off at night (Gives an hour later than end of evening)
          lampControlOffTrigger = 1;
          lampControlOnTrigger = 0;
          sendWebhook();
        }
        confSRiseH = shiftedlamptimehr;  // Save the Sunrise Data
        confSRiseM = shiftedlamptimemin;
      } else {
        if (minute(sSet) > lampGapMins) { // Calculate Sunset
          shiftedlamptimehr = hour(sSet);
        } else {
          shiftedlamptimehr = hour(sSet) - 1;
        }
        shiftedlamptimemin = (60 + minute(sSet) - lampGapMins) % 60;
        if ((aHours > shiftedlamptimehr or (aHours >= shiftedlamptimehr && aMins >= shiftedlamptimemin) or aHours >= lampEveningOnLatest) && (aHours < lampEveningOff)) {   //   turn it on before sunset
          lampControlOffTrigger = 0;
          lampControlOnTrigger = 1;
          sendWebhook();
        }
        if (aHours >= lampMorningOn ) { // Turn it on in the morning
          if (aHours < (hour(sRise) + 1)) {
            lampControlOffTrigger = 0;
            lampControlOnTrigger = 1;
            sendWebhook();
          }
        }
        confSSetH = shiftedlamptimehr;
        confSSetM = shiftedlamptimemin;
      }
    } else {
      if (lampControlOn) {
        if (aHours == 0) {
          lampControlOffTrigger = 1;
          lampControlOnTrigger = 0;
          lampOverRide = 0;
          sendWebhook();
        }
      }
    }
  }
}

void delayCheckTempTriggers() {
  //  Serial.println("delayCheckTriggers");
  if (millis() - timeElapsedWebHook >= intervalWebHook) {   //   If routine for posting the Webhook
    timeElapsedWebHook =  millis();   //   save the last time you check the webhooks
    goCheckTempTriggers();
  }
}

void controlCheckTime() {
  //  Serial.println("controlCheckTime");
  if (millis() - timeElapsedReadTimeDelay >= delayReadTime) {   //   Check NTP Time every 30 minutes
    timeElapsedReadTimeDelay = millis();
    goGetNTPTime();
  }
  if (millis() - timeElapsedReadTimeDelay <= 10 * 1000) {  // Sync Blynk LED's in the first 10 seconds of NTP time updated
    if (aSecs == (device) && !ledSync) { // Try and Sync Blynk LED's
      timeElapsedLED = millis();
      ledSync = 1;
      Serial.print (F("="));
      keepAlivePrintLN++;
    }
  }
  if ((aSecs == (1 + device)) && ledSync) { // reset Sync Blynk LED's
    ledSync = 0;
  }
  if (aHours >= homeNightStartHr or (aHours <= (homeNightFinishHr - 1))) { // Apply Night shifted lower temp
    if (!nightShiftActive) {
      moveNightTimeTempStart();
      nightShiftActive = 1;
    }
  } else {
    if (nightShiftActive) { // Reverse Night shifted lower temp
      moveNightTimeTempFinish();
      nightShiftActive = 0;
    }
  }
}

void moveNightTimeTempStart() {
  //  Serial.println("moveNightTimeTempStart");
  tempTimedHighTrigger = tempTimedHighTrigger - (nightShiftTemp * 100);
  tempTimedHighNormalTrigger = tempTimedHighNormalTrigger - (nightShiftTemp * 100);
  tempTimedLowNormalTrigger = tempTimedLowNormalTrigger - (nightShiftTemp * 100);
  tempTimedLowTrigger = tempTimedLowTrigger - (nightShiftTemp * 100);
  timedtriggers();
}

void moveNightTimeTempFinish() {
  //  Serial.println("moveNightTimeTempFinish");
  tempTimedHighTrigger = tempTimedHighTrigger + (nightShiftTemp * 100);
  tempTimedHighNormalTrigger = tempTimedHighNormalTrigger + (nightShiftTemp * 100);
  tempTimedLowNormalTrigger = tempTimedLowNormalTrigger + (nightShiftTemp * 100);
  tempTimedLowTrigger = tempTimedLowTrigger + (nightShiftTemp * 100);
  timedtriggers();
}

void checkRunTemperatureDelay() {
  //  Serial.println("checkRunTemperatureDelay");
  if (millis() - timeElapsedProg >= intervalProg) {   //   If routine for check/send temp & humidity data
    timeElapsedProg = millis();   //   save the last time you checked and sent temp/humidity data
    bUpdate = 2;   //   sets measure temperature function
  }
}
//
//   Control Section
//
void seasonSetup() {
  //  Serial.println("seasonSetup");
  setupHooks();
  if (!daylightSavings or aMth == monthDSTEnds) {
    Season = 0; // 0 Winter 1 Summer
    if (firstRun) {
      tabindent(4); variableIndent();
    }
    Serial.println(F("Season is Winter"));
  } else {
    Season = 1; // 0 Winter 1 Summer
    if (firstRun) {
      tabindent(4); variableIndent();
    }
    Serial.println(F("Season is Summer"));
  }
}

void weekdayTask() {
  //  Serial.println("weekdayTask");
  if (beenHereOR1) {
    Serial.println(F("Turning off timer override"));
    weekendOverride = 0;
    d1MiniSituation = d1MiniSituationORG;
    beenHereOR1 = 0;
    homeEveningFinishHr = homeEveningFinishHrORG;
    homeEveningStartHr = homeEveningStartHrORG;
    resetControlParameters();
    Blynk.virtualWrite(blynkPortWKNDOR, 0);
  }
  if (d1MiniSituation == 1 ) {   //   Weekday Morning through Daytime off in the evening and night
    if (d1MiniControl == 1) {   //   D1 Mini Controlled
      if (aHours >= (homeMorningStartHr - 1) && aHours < homeEveningStartHr ) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineDaytime();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineDaytime();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if (aHours >= (homeMorningStartHr - 1) && aHours < homeEveningStartHr ) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineDaytime();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnTriggered = 1;
          sendWebhook();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   Timer routine to test if switching mode, heating or cooling, is needed
          timeElapsedTimed = millis();
          goCheckModeSwitch();
        }
      } else {
        if (!timedOffTriggerSent) {
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineDaytime();
          resetControlParameters();
          timedOperation = 0;
          timedOffTriggered = 1;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 2 ) {   //   Weekday Morning and Evening
    if (d1MiniControl == 1) {
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineMDandE();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineMDandE();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr)) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineMDandE();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1 ;
          timedOnTriggered = 1;
          sendWebhook();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   Timer routine to test if switching mode, heating or cooling, is needed
          timeElapsedTimed = millis();
          goCheckModeSwitch();
        }
      } else {
        if (!timedOffTriggerSent) {
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineMDandE();
          resetControlParameters();
          timedOperation = 0;
          timedOffTriggered = 1;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 3 ) {   //   Weekday Morning, Evening & Night
    if (d1MiniControl == 1) {
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr) or (aHours  >= (homeNightStartHr - 1)) or (aHours < homeNightFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineMEandN();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineMEandN();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr) or (aHours  >= (homeNightStartHr - 1)) or (aHours < homeNightFinishHr)) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekdaySpace(); printLineMEandN();
          timedtriggers();
          resetControlParameters();
          timedOnTriggered = 1;
          timedOperation = 1;
          sendWebhook();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   Timer routine to test if switching mode, heating or cooling, is needed
          timeElapsedTimed = millis();
          goCheckModeSwitch();
        }
      } else {
        if (!timedOffTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekdaySpace(); printLineMEandN();
          resetControlParameters();
          timedOperation = 0;
          timedOffTriggered = 1;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 4 ) {   //   Work Time
    if (d1MiniControl == 1) {
      if ((aHours >= workStartHr) && (aHours < workFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printLineWorkDay();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printLineWorkDay();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= workStartHr - 1) && (aHours < workFinishHr)) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printLineWorkDay();
          timedtriggers();
          resetControlParameters();
          timedOnTriggered = 1;
          timedOperation = 1;
          sendWebhook();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   Timer routine to test if switching mode, heating or cooling, is needed
          timeElapsedTimed = millis();
          goCheckModeSwitch();
        }
      } else {
        if (!timedOffTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printLineWorkDay();
          resetControlParameters();
          timedOperation = 0;
          timedOffTriggered = 1;
          sendWebhook();
        }
      }
    }
  }
}

void weekendTask() {
  //  Serial.println("weekendTask");
  byte TimedCheck = timedOperation;
  if (d1MiniSituation == 1 ) {   //   Weekend Morning through Daytime to Evening
    if (d1MiniControl == 1) {
      if ((aHours >= (homeMorningStartHr - 1)) && (aHours < homeEveningFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMTE();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMandE();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= (homeMorningStartHr - 1)) && (aHours < homeEveningFinishHr)) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMTE();
          timedtriggers();
          resetControlParameters();
          timedOnTriggered = 1;
          timedOperation = 1;
          sendWebhook();
        }
      } else {
        if (!timedOffTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMTE();
          resetControlParameters();
          timedOffTriggered = 1;
          timedOperation = 0;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 2 ) {   //   Weekend Morning through Daytime to Evening
    if (d1MiniControl == 1) {
      if ((aHours >= (homeMorningStartHr - 1)) && (aHours < homeEveningFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMTE();
          timedtriggers();
          resetControlParameters();
          timedOperation = 1;
          timedOnControlTriggerSent = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMTE();
          resetControlParameters();
          timedOperation = 0;
          timedOffControlTriggered = 1;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= (homeMorningStartHr - 1)) && (aHours < homeEveningFinishHr)) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMTE();
          timedtriggers();
          resetControlParameters();
          timedOnTriggered = 1;
          timedOperation = 1;
          sendWebhook();
        }
      } else {
        if (!timedOffTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMTE();
          resetControlParameters();
          timedOffTriggered = 1;
          timedOperation = 0;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 3 ) {   //   Weekend Morning through Daytime to Evening & Night
    if (d1MiniControl == 1) {
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr) or (aHours  >= (homeNightStartHr - 1)) or (aHours < homeNightFinishHr)) {
        if (!timedOnControlTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMEandN();
          timedtriggers();
          resetControlParameters();
          timedOnControlTriggerSent = 1;
          timedOperation = 1;
          goCheckTimedTempTriggers();
        }
        if (millis() - timeElapsedTimed >= intervalTimed) {   //   If routine in link with check/send temp & humidity data
          timeElapsedTimed = millis();
          goCheckTimedTempTriggers();
        }
      } else {
        if (!timedOffControlTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMEandN();
          resetControlParameters();
          timedOffControlTriggered = 1;
          timedOperation = 0;
          sendWebhook();
        }
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if ((aHours >= (homeMorningStartHr - 1) && aHours < homeMorningFinishHr) or (aHours >= (homeEveningStartHr - 1) && aHours < homeEveningFinishHr) or aHours  >= (homeNightStartHr - 1) or aHours < homeNightFinishHr) {
        if (!timedOnTriggerSent) {
          Serial.println();
          printStartingSpace(); printWeekendSpace(); printLineMEandN();
          timedtriggers();
          resetControlParameters();
          timedOnTriggered = 1;
          timedOperation = 1;
          sendWebhook();
        }
      } else {
        if (!timedOffTriggerSent) {   //   Else - Turn it off
          Serial.println();
          printFinishingSpace(); printWeekendSpace(); printLineMEandN();
          resetControlParameters();
          timedOffTriggered = 1;
          timedOperation = 0;
          sendWebhook();
        }
      }
    }
  }
  if (d1MiniSituation == 4 ) {   //   Work Time
    if (d1MiniControl == 1) {
      if (!timedOffControlTriggerSent) {   //   Else - Turn it off
        Serial.println();
        printFinishingSpace(); printWork(); printSpace(); printDay(); Serial.println(F("; It's the Weekend"));
        resetControlParameters();
        timedOffControlTriggered = 1;
        timedOperation = 0;
        sendWebhook();
      }
    }
    if (d1MiniControl == 2) {   //   Device Controlled
      if (!timedOffTriggerSent) {
        Serial.println();
        printFinishingSpace(); printWork(); printSpace(); printDay(); Serial.println(F("; It's the Weekend"));
        resetControlParameters();
        timedOffTriggered = 1;
        timedOperation = 0;
        sendWebhook();
      }
    }
  }
  if (weekendOverride) {
    if (aHours == 0) {
      Serial.println(F("Turning off timer override"));
      weekendOverride = 0;
      d1MiniSituation = d1MiniSituationORG;
      beenHereOR1 = 0;
      timedOperation = 0;
      homeEveningFinishHr = homeEveningFinishHrORG;
      homeEveningStartHr = homeEveningStartHrORG;
      resetControlParameters();
      Blynk.virtualWrite(blynkPortWKNDOR, 0);
    }
  }
}
//
//   Send & Receive Blynk
//
void InitialiseBlynk() {
  Serial.println();
  Blynk.config(auth);   //   set Blynk authentication string
  Serial.print(F("Connecting to Blynk..."));   //   send to serial control messsage
  Blynk.connect();   //   start connection to Blynk server, this command has 30 seconds timeout
  terminal.println(F("------terminal-------"));
  terminal.flush();
  timer.setInterval(blynkSendSerialTimer, send_serial);
  tabindent(2);
  Serial.println(F("Connected to Blynk Success"));   //   send to serial control messsage
  blynkLED1off();
  blynkLED2off();
  blynkLED3off();
  blynkLED4off();
  blynkclockDisplay();
  startupSparkle();
}

void send_serial() {   //   Sent serial data to Blynk terminal - Unlimited string readed
  //  Serial.println("send_Serial");
  String content = "";   //   null string constant ( an empty string )
  char character;
  while (Serial.available()) {
    character = Serial.read();
    content.concat(character);
  }
  if (content != "") {
    Blynk.virtualWrite(blynkPortS, content);
  }
}

BLYNK_WRITE_DEFAULT() {   //   Blynk Function to read data from Blynk.
  //  Serial.println("Blynk_Write_Default");
  byte bRequestPin = request.pin;   //   Blynk update pin reported container
  for (auto i = param.begin(); i < param.end(); ++i) {   //   print all parameter values
    if (bRequestPin == blynkPortU) {
      if (i != 0) {
        bUpdate = i;
        Serial.println();
        printInputfromBlynk();   //   Called Write as you write data to the device from the Blynk platform
        Serial.println(request.pin);
        if (millis() - displayconfigtimer <= displayConfigDelay) {
          Serial.println(F("Config Summary Requested"));
          displaySummary();
          displayconfigtimer = 0;
        }
        displayconfigtimer = millis();
      }
    }
    if (bRequestPin == blynkPortResetAll or bRequestPin == blynkPortReset) {
      if (i != 0) {
        Serial.println();
        printInputfromBlynk();   //   Called Write as you write data to the device from the Blynk platform
        Serial.print(request.pin);
        Serial.println();
        softResetD1();
      }
    }
    if (bRequestPin == blynkPortOutput) {
      int state = param.asInt();
      if (!state) {
        externalOutputOn = 2;
      }
      if (state) {
        externalOutputOn = 1;
      }
      if (switchStateChangeCTL != externalOutputOn) {
        switchStateChangeCTL = externalOutputOn;
        if (externalOutputOn == 1) {
          externalCTLOverride = 1;
        } else {
          if (externalOutputOn == 2) {
            externalCTLOverride = 1;
          } else {
            externalOutputOn = 0;
          }
        }
        if (!firstRun) {
          Serial.println();
          printInputfromBlynk();   //   Called Write as you write data to the device from the Blynk platform
          Serial.print(blynkPortOutput); printData();
          Serial.print(externalOutputOn); printSpace();
          sendWebhook();
        }
      }
    }
    if (bRequestPin == blynkPortWKNDOR) {
      int state = param.asInt();
      if (!state) {
        weekendOverride = 0;
        d1MiniSituation = d1MiniSituationORG;
      }
      if (state) {
        weekendOverride = 1;
        d1MiniSituationORG = d1MiniSituation;
        d1MiniSituation = 2;
      }
      Serial.println();
      printInputfromBlynk();   //   Called Write as you write data to the device from the Blynk platform
      Serial.print(request.pin); printData(); Serial.print(weekendOverride); printSpace();
    }
    if (bRequestPin == blynkPortlampCtl) {
      int state = param.asInt();
      int currentlamp = lampControlOn;
      Serial.println();
      printInputfromBlynk();   //   Called Write as you write data to the device from the Blynk platform
      Serial.print(request.pin); printData(); Serial.print(state); printSpace();
      if (!state) {
        lampControlOffTrigger = 0;
        sendWebhook();
        lampOverRide = 0;
      }
      if (state) {
        lampControlOnTrigger = 1;
        sendWebhook();
      }
      if (currentlamp != lampControlOn) {
        lampOverRide = 1;
      }
    }
  }
}

void timerBlynk() {
  //  Serial.println("timerBlynk");
  timer.run();
  Blynk.run();
}

//
//   Send & Receive Web
//
void sendMMWebhook(String Event) {
  //  Serial.println("sendMMWebhook");
  WiFiClient client;
  if (client.connect(mmHost, 8080)) {
    String mm = "GET /syslog?type=INFO&message=" +  Event + " HTTP/1.0";
    client.println(mm);
    //  client.println("Host: mm1");
    //  client.println("Connection: close");
    client.println();
    Serial.println("Magic Mirror Webhook Sent");
  } else {
    Serial.println("Magic Mirror Webhook Not Connected");
  }
  client.stop();
}

void sendMMUpdate() {
  //  Serial.println("SendMMUpdate");
  WiFiClient client;
  if (client.connect(mmHost, 8080)) {
    String mm = "GET /templog?type=";
    if (cTemp >= temp24HrHighTrigger) {
      mm = mm + "ERROR&message=" +  d1MiniLocation + "%20High%20Temp%20Problem%20";
    }
    else if (cTemp <= temp24HrLowTrigger) {
      mm = mm + "ERROR&message=" +  d1MiniLocation + "%20Low%20Temp%20Problem%20";
    }
    else if (cTemp >= tempTimedHighTrigger) {
      mm = mm + "WARNING&message=" +  d1MiniLocation + "%20High%20Temp%20Detected%20";
    }
    else if (cTemp <= tempTimedLowTrigger) {
      mm = mm + "WARNING&message=" +  d1MiniLocation + "%20Low%20Temp%20Detected%20";
    }
    else {
      mm = mm + "INFO&message=" +  d1MiniLocation + "%20Normal%20Temp%20Report%20";
    }
    mm = mm + cTemp / 100 + "." + cTemp % 100 + ((char) 176) + "C%20%20 HTTP/1.0";
    client.println(mm);
    //  client.println("Host: mm1");
    //  client.println("Connection: close");
    client.println();
    Serial.println("Magic Mirror Update Sent");
  } else {
    Serial.println("Magic Mirror Update Not Connected");
  }
  client.stop();
}

void sendWebhook() {
  //  Serial.println("sendWebhook");
  WiFiClient client;
  client.connect(host, httpPort);
  p = post_rqst;
  p = append_str(p, "POST /trigger/");
  if (lampControlOnTrigger) {
    lampControlOn = 1;
    lampControlOnTrigger = 0;
    p = append_str(p, iftttEventLampOn);
    if (!firstRun) {
      Serial.print(F("Turn Lamp Switch On Sent")); printColonSpace(); Serial.println(iftttEventLampOn);
    }
    webHookLoaded = 1;
    blynkLED4on();
  }
  if (!webHookLoaded) {
    if (lampControlOffTrigger) {
      lampControlOn = 0;
      lampControlOffTrigger = 0;
      lampOverRide = 0;
      p = append_str(p, iftttEventLampOff);
      if (!firstRun) {
        Serial.print(F("Turn Lamp Switch Off Sent")); printColonSpace(); Serial.println(iftttEventLampOff);
      }
      webHookLoaded = 1;
      blynkLED4off();
    }
  }
  if (!webHookLoaded) {
    if (externalOutputOn == 1) {
      p = append_str(p, iftttEventOn);
      Serial.print(F("External Switch On Sent")); printColonSpace(); Serial.println(iftttEventOn);
      externalDeviceState = 1;
      externalOutputOn = 0;
      webHookLoaded = 1;
      if (d1MiniControl == 1) {
        currentMode = 0;
      }
      safetyHeating = 1;
      safetyCooling = 1;
      sendMMWebhook(iftttEventOn);
      blynkLED2on();
    }
  }
  if (!webHookLoaded) {
    if (externalOutputOn == 2) {
      p = append_str(p, iftttEventOff);
      Serial.print(F("External Switch Off Sent")); printColonSpace(); Serial.println(iftttEventOff);
      externalDeviceState = 0;
      externalOutputOn = 0;
      webHookLoaded = 1;
      safetyHeating = 1;
      safetyCooling = 1;
      sendMMWebhook(iftttEventOff);
      blynkLED2off();
    }
  }
  if (!webHookLoaded) {
    if (timedOffControlTriggered) {
      p = append_str(p, iftttEventOff);
      printTimedSpace(); printOff(); printSpace(); Serial.print(F("Control")); printSpace(); printTriggeredSpace();
      printWebhookSent(); printColonSpace(); Serial.println(iftttEventOff);
      timedOffControlTriggerSent = 1;
      timedOffControlTriggered = 0;
      externalDeviceState = 0;
      webHookLoaded = 1;
      sendMMWebhook(iftttEventOff);
      blynkLED2off();
    }
  }
  if (!webHookLoaded) {
    if (timedOffTriggered) {
      p = append_str(p, iftttEventOff);
      printTimedSpace(); printOff(); printSpace(); printTrigger(); printSpace();
      printWebhookSent(); printColonSpace(); Serial.println(iftttEventOff);
      timedOffTriggerSent = 1;
      timedOffTriggered = 0;
      externalDeviceState = 0;
      webHookLoaded = 1;
      sendMMWebhook(iftttEventOff);
      blynkLED2off();
    }
  }
  if (!webHookLoaded) {
    if (highTempReset or timedHighTempReset) {
      p = append_str(p, iftttEventHNormal);
      if (highTempReset) {
        printHighSpace(); printTemperature(); printSpace(); Serial.print(F("Reset")); printSpace();
        printWebhookSent(); printColonSpace(); Serial.println(iftttEventHNormal);
        highTempResetSent = 1;
        highTempReset = 0;
        externalDeviceState = 0;
      }
      if (timedHighTempReset) {
        printTimedSpace(); printHighSpace(); printTemperature(); Serial.print(F(" Reset "));
        printWebhookSent(); printColonSpace(); Serial.println(iftttEventHNormal);
        timedHighTempResetSent = 1;
        timedHighTempReset = 0;
        externalDeviceState = 0;
      }
      webHookLoaded = 1;
      sendMMWebhook(iftttEventHNormal);
      blynkLED2off();
    }
  }
  if (!webHookLoaded) {
    if (lowTempReset or timedLowTempReset) {
      p = append_str(p, iftttEventLNormal);
      if (lowTempReset) {
        Serial.print(F("Low Temp Reset "));
        printWebhookSent(); printColonSpace(); Serial.println(iftttEventLNormal);
        lowTempResetSent = 1;
        lowTempReset = 0;
        externalDeviceState = 0;
      }
      if (timedLowTempReset) {
        Serial.print(F("Timed Low Temp Reset "));
        printWebhookSent(); printColonSpace(); Serial.println(iftttEventLNormal);
        timedLowTempResetSent = 1;
        timedLowTempReset = 0;
        externalDeviceState = 0;
      }
      webHookLoaded = 1;
      sendMMWebhook(iftttEventLNormal);
      blynkLED2off();
    }
  }
  if (!webHookLoaded) {
    if (timedOnTriggered) {
      p = append_str(p, iftttEventOn);
      Serial.print(F("Timed On Trigger "));
      printWebhookSent(); printColonSpace(); Serial.println(iftttEventOn);
      timedOnTriggerSent = 1;
      timedOnTriggered = 0;
      externalDeviceState = 1;
      webHookLoaded = 1;
      if (d1MiniControl == 1) {
        currentMode = 0;
      }
      sendMMWebhook(iftttEventOn);
      blynkLED2on();
    }
  }
  if (!webHookLoaded) {
    if (highTempTriggered or timedHighTempTriggered) {
      p = append_str(p, iftttEventHigh);
      if (highTempTriggered) {
        printHighSpace(); printTTWS(); printColonSpace(); Serial.println(iftttEventHigh);
        highTempTriggerSent = 1;
        highTempTriggered = 0;
      }
      if (timedHighTempTriggered) {
        printTimedSpace(); printHighSpace(); printTTWS(); printColonSpace(); Serial.println(iftttEventHigh);
        timedHighTempTriggerSent = 1;
        timedHighTempTriggered = 0;
      }
      currentMode = 1;
      externalDeviceState = 1;
      safetyCooling = 1; // turn on the secondary off for cooling
      safetyHeating = 0; // turn off the secondary off for heating
      webHookLoaded = 1;
      sendMMWebhook(iftttEventHigh);
      blynkLED2on();
    }
  }
  if (!webHookLoaded) {
    if (lowTempTriggered or timedLowTempTriggered) {
      p = append_str(p, iftttEventLow);
      if (lowTempTriggered) {
        printLowSpace(); printTTWS(); printColonSpace(); Serial.println(iftttEventLow);
        lowTempTriggerSent = 1;
        lowTempTriggered = 0;
      }
      if (timedLowTempTriggered) {
        Serial.print(F("Timed Low ")); printTTWS(); printColonSpace(); Serial.println(iftttEventLow);
        timedLowTempTriggerSent = 1;
        timedLowTempTriggered = 0;
      }
      currentMode = 0;
      externalDeviceState = 1;
      safetyCooling = 0; // turn off the secondary off for cooling
      safetyHeating = 1; // turn on the secondary off for heating
      webHookLoaded = 1;
      sendMMWebhook(iftttEventLow);
      blynkLED2on();
    }
  }
  p = append_str(p, "/with/key/");
  p = append_str(p, iftttKey);
  p = append_str(p, " HTTP/1.1\r\n");
  p = append_str(p, "Host: maker.ifttt.com\r\n");
  p = append_str(p, "Content-Type: application/json\r\n");
  p = append_str(p, "Content-Length: ");
  char *content_length_here = p;   //   we need to remember where the content length will go, which is:
  p = append_str(p, "NN\r\n");   //   it's always two digits, so reserve space for them (the NN)
  p = append_str(p, "\r\n");   //   end of headers
  char *json_start = p;   //   construct the JSON; remember where we started so we will know length
  p = append_str(p, "{\"value1\":\"");   //   As described - this example reports a cTemp, Humidity, and fTemp
  p = append_ul(p, cTemp / 100);
  p = append_str(p, "\",\"value2\":\"");
  p = append_ul(p, humidity);
  p = append_str(p, "\",\"value3\":\"");
  p = append_ul(p, fTemp);
  p = append_str(p, "\"}");
  int i = strlen(json_start);   //   go back and fill in the JSON length
  content_length_here[0] = '0' + (i / 10);   //   we just know this is at most 2 digits (and need to fill in both)
  content_length_here[1] = '0' + (i % 10);
  client.print(post_rqst);   //   finally we are ready to send the POST to the server!
  client.stop();
  webHookLoaded = 0;
  watchdogCounter++;
  keepAlivePrintLN = 0;
  Serial.println();
}

//   Webserver

void handleRoot() {
  //  Serial.println("Webserver");
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

//
//   LED Section
//

void blinkTheLEDCheck() {
  //  Serial.println("blinkTheLEDCheck");
  if (millis() - timeElapsedLED >= intervalLED) {   //   If routine for blinking the still alive LED
    timeElapsedLED = millis();   //   save the last time you blinked the LED
    delayLoopLEDTurnedOn = millis();   //   Set delay loop timer 1 time
    delayLEDTurnedOn = true;   //   Set delay loop timer 1 on
    flashLED();   //   turn the LED on
    if (!setupATime) {
      if (timedOperation) {
        Serial.print(F("%"));   //   Serial monitor still alive pulse timed
        keepAlivePrintLN++;
        if (!blynkPortTimed) {   //   Manage the timed/override switch on
          blynkPortTimed = 1;
          blynkLED3on();
        }
      } else {
        Serial.print(F("*"));   //   Serial monitor still alive pulse not timed
        keepAlivePrintLN++;
        if (blynkPortTimed) {   //   Manage the timed/override switch off
          blynkPortTimed = 0;
          blynkLED3off();
        }
      }
    }
  }
  if (delayLEDTurnedOn) {
    if ((millis() - delayLoopLEDTurnedOn) >= delayHalfSecond) {
      delayLEDTurnedOn = false;
      ledOn = 1;
      flashLED();
    }
  }
}

void flashLED() {
  //  Serial.println("flashLED");
  ledOn = !ledOn;   //   toggle the led
  if (ledOn) {
    if (blinkLED or (!blinkLED && aHours >= 7 && aHours <= 18)) {
      digitalWrite(LED_BUILTIN, turn_On);   //   turn led on
    }
    blynkLED1on();   //   send to Blynk virtual LED On
  } else {
    digitalWrite(LED_BUILTIN, turn_Off);   //   turn led off
    blynkLED1off();   //   send to Blynk virtual LED Off
    if (!delayLEDTurnedOff) {
      delayloopLEDTurnedOff = millis();   //   Set delay loop timer to turn off
      delayLEDTurnedOff = true;   //   Set delay loop timer on
    }
  }
}

void blynkLED1on() {   //  Keep Alive LED On
  if (device == 1) led11.on();
  if (device == 2) led21.on();
  if (device == 3) led31.on();
  if (device == 4) led41.on();
  if (device == 5) led51.on();
}

void blynkLED1off() {   //  Keep Alive LED off
  if (device == 1) led11.off();
  if (device == 2) led21.off();
  if (device == 3) led31.off();
  if (device == 4) led41.off();
  if (device == 5) led51.off();
}

void blynkLED2on() {   //  Output On LED
  Blynk.virtualWrite(blynkPortOutput, 1);   //   Toggle the on/off button
  if (device == 1) {
    if (currentMode == 0) {
      Blynk.setProperty(blynkPort1ControlLED, "color", "#D3435C"); // Red for Heating
    } else {
      Blynk.setProperty(blynkPort1ControlLED, "color", "#42CEF5"); // Blue for cooling
    }
    led12.on();
  }
  if (device == 2) {
    if (currentMode == 0) {
      Blynk.setProperty(blynkPort2ControlLED, "color", "#D3435C"); // Red for Heating
    } else {
      Blynk.setProperty(blynkPort2ControlLED, "color", "#42CEF5"); // Blue for cooling
    }
    led22.on();
  }
  if (device == 3) {
    if (currentMode == 0) {
      Blynk.setProperty(blynkPort3ControlLED, "color", "#D3435C"); // Red for Heating
    } else {
      Blynk.setProperty(blynkPort3ControlLED, "color", "#42CEF5"); // Blue for cooling
    }
    led32.on();
  }
  if (device == 4) {
    if (currentMode == 0) {
      Blynk.setProperty(blynkPort4ControlLED, "color", "#D3435C"); // Red for Heating
    } else {
      Blynk.setProperty(blynkPort4ControlLED, "color", "#42CEF5"); // Blue for cooling
    }
    led42.on();
  }
  if (device == 5) {
    if (currentMode == 0) {
      Blynk.setProperty(blynkPort5ControlLED, "color", "#D3435C"); // Red for Heating
    } else {
      Blynk.setProperty(blynkPort5ControlLED, "color", "#42CEF5"); // Blue for cooling
    }
    led52.on();
  }
}

void blynkLED2off() {   //  Output Off LED
  Blynk.virtualWrite(blynkPortOutput, 0);   //   Toggle the on/off button
  if (device == 1) led12.off();
  if (device == 2) led22.off();
  if (device == 3) led32.off();
  if (device == 4) led42.off();
  if (device == 5) led52.off();
}

void blynkLED3on() {   //  Timer On LED
  Blynk.virtualWrite(blynkPortWKNDOR, 1);   //   Toggle the on/off button
  if (device == 1) led13.on();
  if (device == 2) led23.on();
  if (device == 3) led33.on();
  if (device == 4) led43.on();
  if (device == 5) led53.on();
}

void blynkLED3off() {   //  Timer Off LED
  Blynk.virtualWrite(blynkPortWKNDOR, 0);   //   Toggle the on/off button
  if (device == 1) led13.off();
  if (device == 2) led23.off();
  if (device == 3) led33.off();
  if (device == 4) led43.off();
  if (device == 5) led53.off();
}

void blynkLED4on() {   //  Lamp On LED
  Blynk.virtualWrite(blynkPortlampCtl, 1);   //   Toggle the on/off button
  if (device == 1) led14.on();
  if (device == 2) led24.on();
  if (device == 3) led34.on();
  if (device == 4) led44.on();
  if (device == 5) led54.on();
}

void blynkLED4off() {   //  Lamp Off LED
  Blynk.virtualWrite(blynkPortlampCtl, 0);   //   Toggle the on/off button
  if (device == 1) led14.off();
  if (device == 2) led24.off();
  if (device == 3) led34.off();
  if (device == 4) led44.off();
  if (device == 5) led54.off();
}
//
//   Time Section
//
void InitialiseTime() {
  //  Serial.println("initialiseTime");
  byte utcOSHours;   //   Variable for UTC Offset Hours for location
  DefineInitialUTCOffset();
  timeClient.begin();   //   Start Time Client
  goGetNTPTime();
  getSunRiseSet();
  lastADay = aDay;
  Serial.println();
  utcOSHours = (utcOffsetSeconds / 60 / 60);
  Serial.print(F("UTC Offset Being Used"));
  tabindent(2); variableIndent(); Serial.print(utcOSHours); printSpace(); Serial.println(F("Hours"));
  delayDisplayLocalTime = aSecs;
  startupSparkle();
}

void InitialiseTimers() {
  //  Serial.println("initialiseTimers");
  currentMillis = millis();
  timeElapsedTimed = currentMillis - (intervalTimed - 30000);
  timeElapsedBlynk = currentMillis;   //   save the last time you checked Blynk
  timeElapsedProg = currentMillis - (intervalProg - 15000);   //   save the last time you checked and sent temp/humidity data
  timeElapsedLED = currentMillis; // Set timer for Blynk LED's
  setupATime = 0;
  timer.setInterval(300000L, blynkclockDisplay);
  startupSparkle();
}

void getSunRiseSet() {
  //  Serial.println("getSunRiseSet");
  int offset;
  tmElements_t  tm;   //   specific time
  tm.Second = aSecs;
  tm.Minute = aMins;
  tm.Hour   = aHours;
  tm.Day    = aDay;
  tm.Month  = aMth;
  tm.Year   = aYr - 1970;
  time_t s_date = makeTime(tm);
  if (daylightSavings) {
    offset = (utcOffsetSeconds - (60 * 60)) / 60;
  } else {
    offset = utcOffsetSeconds / 60;
  }
  sm.init(offset, latitude, longtitude);
  mDay = sm.moonDay(s_date);
  sRise = sm.sunRise(s_date);
  sSet  = sm.sunSet(s_date);
  forecast = sm.dayForecast(s_date);
  if (daylightSavings) {
    breakTime(sRise, tm);
    tm.Hour = tm.Hour + 1;
    sRise = makeTime(tm);
    breakTime(sSet, tm);
    tm.Hour = tm.Hour + 1;
    sSet = makeTime(tm);
  }
}

void goGetNTPTime() {
  //  Serial.println("goGetNTPTime");
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  if (!setupATime) {
    Serial.println();
    Serial.print(F("Local Time Before Update Sync:"));
  }
  localUTCTime();
  Serial.println();
  Serial.print(F("Syncing Time with NTP"));
  epoch = timeClient.getEpochTime();
  aMilli = millis();
  tabindent(2);
  Serial.println(F("Done"));
  updateArduinoTime();
  if (setupATime) {
    dstYorN();   //   Now we have time, test for DST
    if (daylightSavings) {
      utcOffsetSeconds = (utcOffsetSeconds + (60 * 60));
      timeClient.setTimeOffset(utcOffsetSeconds);
      Serial.print(F("Daylight Saving Applied"));
      tabindent(2); variableIndent(); Serial.println(F("Startup Daylight Saving Time Enabled"));
      lampGapMins = lampGapMinsSet; // Lamp on off at sunrise and on at sunset
      epoch = timeClient.getEpochTime();
      updateArduinoTime();
    } else {
      printNormal(); printSpace(); Serial.print(F("Time Applied"));
      tabindent(2); variableIndent(); Serial.println(F("Startup Daylight Saving Time Disabled"));
      lampGapMins = lampGapMinsSet + lampWinterExt; // Extend Lamp on off at sunrise and on at sunset by 15 minutes during winter
    }
    seasonSetup();
  } else {
    if (lastADay != aDay) {
      dstYorN();   //   Now we have time, test for DST
      getSunRiseSet();
      lastADay = aDay;
      if (dstChanged) {
        if (daylightSavings) {
          utcOffsetSeconds = (utcOffsetSeconds + (60 * 60));
          timeClient.setTimeOffset(utcOffsetSeconds);
          Serial.print(F("Daylight Saving")); printlineTriggered();
          variableIndent(); Serial.println(F("Daylight Saving Time Enabled"));
          lampGapMins = lampGapMinsSet; // Lamp on off at sunrise and on at sunset
          updateArduinoTime();
          printDeviceLeader(); Serial.print(device); printlineColon();
        }
      } else {
        if (!daylightSavings) {
          if (setupATime) {
            utcOffsetSeconds = (utcOffsetSeconds - (60 * 60));
            timeClient.setTimeOffset(utcOffsetSeconds);
            printNormal(); Serial.println(F("Time Resumes"));
            variableIndent(); Serial.println(F("Daylight Saving Time Disabled"));
            lampGapMins = lampGapMinsSet + lampWinterExt; // Extend Lamp on off at sunrise and on at sunset by 15 minutes during winter
          }
          updateArduinoTime();
          printDeviceLeader(); Serial.print(device); printlineColon();
        }
      }
    }
    seasonSetup();
  }
  localUTCDate();
  localUTCTime();
}

void dstYorN() {
  //  Serial.println("dstYorN");
  if (dstInTimeZone) {
    dstChanged = daylightSavings;   //   Last week for September and First week for April
    int checkDST = isDST();
    if (checkDST) {
      daylightSavings = 1;
    } else {
      daylightSavings = 0;
    }
    if (dstChanged != daylightSavings) {
      dstChanged = 1;
    } else {
      dstChanged = 0;
    }
  } else {
    dstChanged = 0;
  }
}

bool isDST() {
  //  Serial.println("isDST");
  if (aMth < 4 or aMth > 9) {
    return true;
  }
  if (aMth > 4 && aMth < 9) {
    return false;
  }
  int previousSunday = aDay - aWDay;
  if (aMth == 9) {
    return previousSunday >= 28;
  }
  return previousSunday <= 0;
}

void localUTCTime() {
  Serial.println();
  Serial.print(F("The Local UTC time is "));   //   UTC is the time at Greenwich Meridian (GMT)
  if (firstRun) {
    tabindent(2); variableIndent();
  }
  Serial.print((epoch % 86400L) / 3600);   //   print the hour (86400 equals secs per day)
  Serial.print(':');
  if (((epoch % 3600) / 60) < 10 ) {
    Serial.print('0');   //   In the first 10 minutes of each hour, we'll want a leading '0'
  }
  Serial.print((epoch % 3600) / 60);   //   print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ((epoch % 60) < 10 ) {
    Serial.print('0');   //   In the first 10 seconds of each minute, we'll want a leading '0'
  }
  Serial.print(epoch % 60);   //   print the second
  printSpace();
  keepAlivePrintLN = 0;
  Serial.println();
}

void tabindent(int tabs) {
  for (int i = 0; i < tabs; i++) {
    printTAB();
  }
}

void localUTCDate() {
  Serial.print(F("The Local UTC Date is "));
  if (firstRun) {
    tabindent(2); variableIndent();
  }
  Serial.print(weekdays[aWDay]);
  printSpace();
  if ((aDay) < 10 ) {
    Serial.print('0');
  }
  Serial.print(aDay); Serial.print(F("/"));
  if ((aMth) < 10 ) {
    Serial.print('0');
  }
  Serial.print(aMth); Serial.print(F("/")); Serial.print(aYr); printSpace();
}

BLYNK_CONNECTED() {   //   Synchronize time on connection
  rtc.begin();
}

void blynkclockDisplay() {
  //  Serial.println("blynkClockDisplay");
  String currentTime = String(hour() < 10 ? "0" + String(hour()) : String(hour())) + ":" + (minute() < 10 ? "0" + String(minute()) : String(minute())) + ":" + (second() < 10 ? "0" + String(second()) : String(second()));
  String currentDate = String(day() < 10 ? "0" + String(day()) : String(day())) + "/" + String(month() < 10 ? "0" + String(month()) : String(month())) + "/" + year();
  if (!setupATime) {
    Serial.println();
    Serial.print(F("Current Blynk time")); printColonSpace();
    if (firstRun) {
      tabindent(2);
    }
    Serial.print(currentTime); printSpace(); Serial.println(currentDate);
    keepAlivePrintLN = 0;
  }
}

void updateArduinoTime() {
  //  Serial.println("updateArduinoTime");
  String inString = "";   //   String used for Blynk terminal
  if (firstRun) {
    aMilli = millis();
  }
  aSecsUpdate = (millis() - aMilli) / 1000UL;
  aMilli = millis();
  delayLoopArduinoTime = aMilli;   //   Set delay loop timer 2 time
  epoch = epoch + aSecsUpdate;
  time_t rawtime = epoch;
  struct tm * ti;
  ti = localtime (&rawtime);
  uint16_t year = ti ->tm_year + 1900;
  inString = String(year);
  aYr = inString.toInt();
  uint8_t month = ti->tm_mon + 1;
  inString = month < 10 ? "0" + String(month) : String(month);
  aMth = inString.toInt();
  uint8_t day = ti->tm_mday;
  inString = day < 10 ? "0" + String(day) : String(day);
  aDay = inString.toInt();
  uint8_t hours = ti->tm_hour;
  inString = hours < 10 ? "0" + String(hours) : String(hours);
  aHours = inString.toInt();
  uint8_t minutes = ti->tm_min;
  inString = minutes < 10 ? "0" + String(minutes) : String(minutes);
  aMins = inString.toInt();
  uint8_t seconds = ti->tm_sec;
  inString = seconds < 10 ? "0" + String(seconds) : String(seconds);
  aSecs = inString.toInt();
  aWDay = ((epoch / 86400) + 4) % 7;
}

void DefineInitialUTCOffset() {
  if (utcOffset > 0) {   //   Add offset for timezones before UTC
    utcOffsetSeconds = (utcOffset * 60 * 60);
  }
  if (utcOffset < 0) {   //   Add offset for timezones after UTC
    utcOffsetSeconds = (-utcOffset * 60 * 60);
  }
  timeClient.setTimeOffset(utcOffsetSeconds);
}
//
//   Temperature Section
//
void initialiseI2C() { //   send to serial control messsage
  Serial.println();
  Serial.print(F("Initialise I2C")); printColon();
  Wire.begin();   //   Initialise I2C communication as MASTER
  tabindent(3); variableIndent(); Serial.println(F("I2C communication Initialised as MASTER"));   //   send to serial control messsage
  startupSparkle();
}

void runTemperature() {
  if (bUpdate >= 1) {   //   External request for check/send temp & humidity data
    Serial.println();
    Serial.println();
    Serial.print(F("Update Status")); printColonSpace();
    if (bUpdate == 1) {
      Serial.print(F("External")); printSpace(); printTemperature(); printSpace(); Serial.println(F("Check Requested"));
      timeElapsedTimed = millis() - intervalTimed;
      //   delayLoopLEDTurnedOn = millis();   //   Set delay loop timer 1 time
    }
    if (bUpdate == 2) {
      printNormal(); printSpace(); Serial.print(F("Timer")); printSpace(); printTriggeredSpace();
      printTemperature(); printSpace(); Serial.println(F("Check"));
    }
    sendTemperature();
    bUpdate = 0;
    keepAlivePrintLN = 0;
  }
}

void goCheckModeSwitch() {   //   Routine for switching aircon from heating to cooling or cooling to heating
  //  Serial.println("goCheckModeSwitch");
  String holdString;
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println(F("Connection failed connecting to IFTTT"));
    Serial.println();
  } else {
    if (cTemp <= (tempTimedLowNormalTrigger - 100)) {
      if (currentMode != 0) {
        Serial.println();
        Serial.println(F("Switching to Heating"));
        currentMode = 0;
        holdString = iftttEventLow;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
        timedOnTriggered = 1;
        sendWebhook();
      }
    }
    if (cTemp >= (tempTimedHighNormalTrigger + 100)) {
      if (currentMode != 1) {
        Serial.println();
        Serial.println(F("Switching to Cooling"));
        currentMode = 1;
        holdString = iftttEventHigh;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
        timedOnTriggered = 1;
        sendWebhook();
      }
    }
  }
}

void goCheckTempTriggers() {
  //  Serial.println("goCheckTempTriggers");
  if (!firstRun) {
    if (externalDeviceState) {
      Serial.print(F(">#"));   //   Serial monitor still alive pulse with controlled equipment on
    } else {
      Serial.print(F("^#"));   //   Serial monitor still alive pulse with controlled equipment off
    }
    if (keepAlivePrintLN >= 77) {
      Serial.println();
      keepAlivePrintLN = 0;
    }
    for (int i = 0 ; i < 2 ; i++ ) {
      keepAlivePrintLN++;
    }
  }
  if (!timedOperation) {   //   Exclude 24 hour return to normal triggers when timed operation in progress
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
      Serial.println(F("Connection failed connecting to IFTTT"));
      Serial.println();
    } else {
      if (cTemp >= temp24HrHighTrigger) {
        if (!highTempTriggerSent or (externalDeviceState && externalCTLOverride)) {
          Serial.println();
          printHighSpace(); printTemperature(); printlineTriggered();
          highTempTriggered = 1;
          externalCTLOverride = 0;
          sendWebhook();
        }
      }
      if (cTemp <= temp24HrLowTrigger) {
        if (!lowTempTriggerSent) {
          Serial.println();
          printLowSpace(); printTemperature(); printlineTriggered();
          lowTempTriggered = 1;
          sendWebhook();
        }
      }
      if (highTempTriggerSent) {
        if (cTemp <= temp24HrHighNormalTrigger) {
          Serial.println();
          Serial.println(F("Return from")); printSpace(); printHighSpace(); printTemperature(); printlineTriggered();
          highTempReset = 1;
          sendWebhook();
        }
      }
      if (lowTempTriggerSent) {
        if (cTemp >= temp24HrLowNormalTrigger) {
          Serial.println();
          Serial.print(F("Return from")); printSpace(); printLowSpace(); printTemperature(); printlineTriggered();
          lowTempReset = 1;
          sendWebhook();
        }
      }
      if (externalOutputOn == 1) {
        Serial.println();
        Serial.println(F("External Turn Device")); printSpace(); printOn(); Serial.println();
        sendWebhook();
      }
      if (externalOutputOn == 2) {
        Serial.println();
        Serial.println(F("External Turn Device")); printSpace(); printOff(); Serial.println();
        sendWebhook();
      }
    }
  }
}

void goCheckTimedTempTriggers() {
  //  Serial.println("goCheckTimedTempTriggers");
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println(F("Connection failed connecting to IFTTT"));
    Serial.println();
  } else {
    if (cTemp >= tempTimedHighTrigger) {
      if (!timedHighTempTriggerSent or (externalDeviceState && externalCTLOverride)) {
        Serial.println();
        printTimedSpace(); printHighSpace(); printTemperature(); printlineTriggered();
        timedHighTempTriggered = 1;
        externalCTLOverride = 0;
        sendWebhook();
      }
    }
    if (cTemp <= tempTimedLowTrigger) {
      if (!timedLowTempTriggerSent) {
        Serial.println();
        printTimedSpace(); printLowSpace(); printTemperature(); printlineTriggered();
        timedLowTempTriggered = 1;
        sendWebhook();
      }
    }
    if (cTemp <= tempTimedHighNormalTrigger) {
      if (timedHighTempTriggerSent) {
        Serial.println();
        Serial.print(F("Return from")); printSpace(); printTimedSpace(); printHighSpace(); printTemperature(); printlineTriggered();
        timedHighTempTriggerSent = 0;
        timedHighTempReset = 1;
        sendWebhook();
      }
    }
    if (cTemp >= tempTimedLowNormalTrigger) {
      if (timedLowTempTriggerSent) {
        Serial.println();
        Serial.print(F("Return from")); printSpace(); printTimedSpace(); printLowSpace(); printTemperature(); printlineTriggered();
        timedLowTempTriggerSent = 0;
        timedLowTempReset = 1;
        sendWebhook();
      }
    }
  }
}

void watchdogSafety() {
  //  Serial.println("watchdogSafety");
  if (safetyHeating) {
    if (cTemp >= tempTimedHighNormalTrigger) {
      timedLowTempTriggerSent = 0;
      timedLowTempReset = 1;
      sendWebhook();
      safetyHeating = 0;
    }
  }
  if (safetyCooling) {
    if (cTemp >= tempTimedLowNormalTrigger) {
      timedHighTempTriggerSent = 0;
      timedHighTempReset = 1;
      sendWebhook();
      safetyCooling = 0;
    }
  }
}

void sendTemperature() {   //   temperature measuring function
  //  Serial.println("sendTemperature");
  unsigned int data[6];
  Wire.beginTransmission(Addr);   //   Start I2C Transmission
  Wire.write(0x2C);   //   Send measurement command
  Wire.write(0x06);
  Wire.endTransmission();   //   Stop I2C transmission
  delay(500);
  aMilli = aMilli + 500;   //   Account for the delay
  Wire.requestFrom(Addr, 6);   //   Request 6 bytes of data
  if (Wire.available() == 6) {//   Read 6 bytes of data
    data[0] = Wire.read();   //   cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }
  rCTemp = ((((data[0] * 256.000) + data[1]) * 175.000) / 65535.000) - 45.000 - offsetTemp;   //   Convert the data deg C
  rFTemp = (rCTemp * 1.8) + 32;   //   Convert the data deg F
  humidity = (((((data[3] * 256.000) + data[4]) * 100.000) / 65535.000) + offsetHumid);   //   Convert the data %RH
  cTemp = rCTemp * 100.0;
  fTemp = rFTemp * 100.0;
  rCTemp = cTemp / 100.0;
  rFTemp = fTemp / 100.0;
  if (!firstRun) {
    Serial.println();
  }
  if (rCTemp >= 100) {
    Serial.println(F("Problem with Sensor"));
    Blynk.virtualWrite(blynkPortT, 0);
    Blynk.virtualWrite(blynkPortH, 0);
  } else {
    if (firstRun) {
      Serial.print(F("Raw Data")); tabindent(3); variableIndent(); printTemperature(); printColonSpace();
      Serial.print(data[0]); printSpace(); printAndSpace(); Serial.println(data[1]);
      tabindent(4); variableIndent(); printHumidity(); printColonSpace();
      Serial.print(data[3]); printSpace(); printAndSpace(); Serial.println(data[4]);
    }
    printTemperature();
    printSpace();
    Serial.print(F("in Celsius")); printColonSpace(); tabindent(1); serialPrintDecimal(cTemp); Serial.println(F(" C"));
    Serial.print(F("Relative Humidity")); printColonSpace(); tabindent(2); Serial.print(humidity); Serial.println(F(" %RH"));
    Serial.print(F("Send Blynk Temp"));
    Blynk.virtualWrite(blynkPortT, rCTemp);   //   send to Blynk virtual pin temperature value
    tabindent(3); Serial.println(F("Done"));
    Serial.print(F("Send Blynk Humidity"));
    Blynk.virtualWrite(blynkPortH, humidity);   //   send to Blynk virtual pin 1 humidity value
    tabindent(2); Serial.println(F("Done"));
    Serial.println(F("Update Blynk Successful"));   //   send to serial control messsage
    if (!firstRun) {
      Serial.println();
    }
    sendMMUpdate();
  }
}
//
//   Setup Section
//
//
//   Wifi Section
//
void ConnectWifi() {
  Serial.print(F("Connecting to WiFi Network"));
  tabindent(1); variableIndent(); Serial.println(usewifissid);
  WiFi.begin(usewifissid, usewifipswd);   //   Setup WiFi
  Serial.print(F("Connecting to WiFi "));
  while (WiFi.status() != WL_CONNECTED) {   //   Wait until connected
    delay(500);   //   5 Second delay (loop timer not applicable in void setup ())
    Serial.print(F("*"));   //   Still alive pulse
  }
  Serial.println(); //   Report connection and IP Address
  Serial.print(F("Connected, IP address")); tabindent(2); variableIndent(); Serial.println(WiFi.localIP());
  startupSparkle();
}

void listNetworks() {   //   scan for nearby networks:
  Serial.println(F("Setting Up WiFi"));
  Serial.println();
  Serial.println(F("Scanning available networks..."));
  Serial.println(F("** Scan Networks **"));
  Serial.println();
  int numSsid = WiFi.scanNetworks();
  String holdStringssid = "";
  String holdStringpswd = "";
  if (numSsid == -1) {
    Serial.println(F("Couldn't get a wifi connection"));
    while (true);
  }
  Serial.print(F("number of available networks:"));   //   print the list of networks seen:
  Serial.println(numSsid);
  int o = numSsid;   //   sort by RSSI
  int n = numSsid;
  int loops = 0;
  int indices[n];
  int skip[n];
  String ssid;
  for (int i = 0; i < n; i++) {
    indices[i] = i;
  }
  bool sortRSSI = true;   //   CONFIG   //   sort aps by RSSI
  bool removeDups = false;   //   remove dup aps ( forces sort )
  bool printAPs = true;   //   print found aps
  if (removeDups or sortRSSI) {
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          loops++;
          //   int temp = indices[j];
          //   indices[j] = indices[i];
          //   indices[i] = temp;
          std::swap(indices[i], indices[j]);
          std::swap(skip[i], skip[j]);
        }
      }
    }
  }
  if (removeDups) {
    for (int i = 0; i < n; i++) {
      if (indices[i] == -1) {
        --o;
        continue;
      }
      ssid = WiFi.SSID(indices[i]);
      for (int j = i + 1; j < n; j++) {
        loops++;
        if (ssid == WiFi.SSID(indices[j])) {
          indices[j] = -1;
        }
      }
    }
  }
  int APSelected = 0;
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {   //   print the network number and name for each network found:
    if ((indices[thisNet]) < 10 ) {
      Serial.print('0');
    }
    Serial.print(indices[thisNet]); Serial.print(F(") ")); Serial.print(F("Signal")); printColonSpace();
    Serial.print(WiFi.RSSI(indices[thisNet])); Serial.print(F(" dBm"));
    tabindent(1); Serial.print(F("Encryption")); printColonSpace();
    printEncryptionType(WiFi.encryptionType(indices[thisNet]));
    tabindent(2); Serial.print(F("SSID")); printColonSpace(); Serial.println(WiFi.SSID(indices[thisNet]));
    if (!APSelected) {
      if (WiFi.SSID(indices[thisNet]) == wifi1ssid) {
        holdStringssid = wifi1ssid;
        holdStringssid.toCharArray(usewifissid, holdStringssid.length() + 1);
        holdStringpswd = wifi1pswd;
        holdStringpswd.toCharArray(usewifipswd, holdStringpswd.length() + 1);
        APSelected = 1;
      }
      if (WiFi.SSID(indices[thisNet]) == wifi2ssid) {
        holdStringssid = wifi2ssid;
        holdStringssid.toCharArray(usewifissid, holdStringssid.length() + 1);
        holdStringpswd = wifi2pswd;
        holdStringpswd.toCharArray(usewifipswd, holdStringpswd.length() + 1);
        APSelected = 1;
      }
      if (WiFi.SSID(indices[thisNet]) == wifi3ssid) {
        holdStringssid = wifi3ssid;
        holdStringssid.toCharArray(usewifissid, holdStringssid.length() + 1);
        holdStringpswd = wifi3pswd;
        holdStringpswd.toCharArray(usewifipswd, holdStringpswd.length() + 1);
        APSelected = 1;
      }
      if (WiFi.SSID(indices[thisNet]) == wifi4ssid) {
        holdStringssid = wifi4ssid;
        holdStringssid.toCharArray(usewifissid, holdStringssid.length() + 1);
        holdStringpswd = wifi4pswd;
        holdStringpswd.toCharArray(usewifipswd, holdStringpswd.length() + 1);
        APSelected = 1;
      }
    }
  }
  Serial.println();
  Serial.println(F("Selecting WiFi Network"));
  Serial.print(F("SSID")); printColon(); tabindent(4); variableIndent(); Serial.println(usewifissid);
  startupSparkle();
}

void printEncryptionType(int thisType) {
  switch (thisType) {   //   read the encryption type and print out the name:
    case ENC_TYPE_WEP:
      Serial.print(F("WEP"));
      break;
    case ENC_TYPE_TKIP:
      Serial.print(F("WPA"));
      break;
    case ENC_TYPE_CCMP:
      Serial.print(F("WPA2"));
      break;
    case ENC_TYPE_NONE:
      Serial.print(F("None"));
      break;
    case ENC_TYPE_AUTO:
      Serial.print(F("Auto"));
      break;
  }
}

void setupESPAP() {
  Serial.println();
  if (!usewifissid) {
    Serial.println(F("Configuring access point..."));
    WiFi.softAP(apssid, appassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(F("AP SSID")); tabindent(4); variableIndent(); Serial.println(apssid);
    Serial.print(F("AP IP address")); tabindent(3); variableIndent(); Serial.println(myIP);
  } else {
    WiFi.mode(WIFI_STA);
  }
  server.on("/", handleRoot);
  server.begin();
  Serial.print(F("HTTP server started...")); tabindent(2); variableIndent(); Serial.println(F("Ok"));
  Serial.println();
  startupSparkle();
}

void individualiseSensor() {
  //  Serial.println("individualiseSensor");
  String DMAC5 = String(mac[5], HEX);
  String DMAC4 = String(mac[4], HEX);
  String DMAC3 = String(mac[3], HEX);
  String DMAC2 = String(mac[2], HEX);
  String DMAC1 = String(mac[1], HEX);
  String DMAC0 = String(mac[0], HEX);
  String holdString;
  deviceMac = String(DMAC5 + ":" + DMAC4 + ":" + DMAC3 + ":" + DMAC2 + ":" + DMAC1 + ":" + DMAC0);
  if (d1MAC == deviceMac) {
    printDeviceLeader();
    device = 1;
    Serial.print(device);
    d1MiniSituation = d1Situation;
    d1MiniSituationORG = d1Situation;
    d1MiniControl = d1Control;
    blynkPortT = 1;   //   Blynk Virtual Pin for the Temp data
    blynkPortH = 2;   //   Blynk Virtual Pin for the Humidity data
    blynkPortU = 11;   //   Blynk Virtual Pin to trigger sensor read and update
    blynkPortS = 16;   //   Blynk Virtual Pin for Serial Terminal Widget
    blynkPortReset = blynkPort1Reset;
    blynkPortOutput = blynkPort1ControlOutput;
    blynkPortlampCtl = blynkPort1lampCtl;
    blynkPortWKNDOR = blynkPort1WKNDOR;
    d1MiniLocation = d1Location;
    holdString = iftttEvent1LampOn;
    holdString.toCharArray(iftttEventLampOn, holdString.length() + 1);
    holdString = iftttEvent1LampOff;
    holdString.toCharArray(iftttEventLampOff, holdString.length() + 1);
    lampMorningOn = lampMorning1On;
    lampEveningOff = lampEvening1Off;
    lampEveningOnLatest = lampEvening1OnLatest;
    temp24HrHighTrigger = temp24Hr1HighTrigger;   //   deg C High Temperature where the webhook fires
    temp24HrHighNormalTrigger = temp24Hr1HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowNormalTrigger = temp24Hr1LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowTrigger = temp24Hr1LowTrigger;
    tempTimedHighTrigger = tempTimed1HighTrigger;   //   deg C High Temperature where the webhook fires
    tempTimedHighNormalTrigger = tempTimed1HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowNormalTrigger = tempTimed1LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowTrigger = tempTimed1LowTrigger;
    if (!d1Shield) {
      offsetTemp = 0;
      offsetHumid = 0;
    }
    apssid = "ESPD1";
    blinkLED = blinkLEDA[1];
  }
  if (d2MAC == deviceMac) {
    printDeviceLeader();
    device = 2;
    Serial.print(device);
    d1MiniSituation = d2Situation;
    d1MiniSituationORG = d2Situation;
    d1MiniControl = d2Control;
    blynkPortT = 3;   //   Blynk Virtual Pin for the Temp data
    blynkPortH = 4;   //   Blynk Virtual Pin for the Humidity data
    blynkPortU = 12;   //   Blynk Virtual Pin to trigger sensor read and update
    blynkPortS = 17;   //   Blynk Virtual Pin for Serial Terminal Widget
    blynkPortReset = blynkPort2Reset;
    blynkPortOutput = blynkPort2ControlOutput;
    blynkPortlampCtl = blynkPort2lampCtl;
    blynkPortWKNDOR = blynkPort2WKNDOR;
    d1MiniLocation = d2Location;
    holdString = iftttEvent2LampOn;
    holdString.toCharArray(iftttEventLampOn, holdString.length() + 1);
    holdString = iftttEvent2LampOff;
    holdString.toCharArray(iftttEventLampOff, holdString.length() + 1);
    lampMorningOn = lampMorning2On;
    lampEveningOff = lampEvening2Off;
    lampEveningOnLatest = lampEvening2OnLatest = 18;
    temp24HrHighTrigger = temp24Hr2HighTrigger;   //   deg C High Temperature where the webhook fires
    temp24HrHighNormalTrigger = temp24Hr2HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowNormalTrigger = temp24Hr2LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowTrigger = temp24Hr2LowTrigger;
    tempTimedHighTrigger = tempTimed2HighTrigger;   //   deg C High Temperature where the webhook fires
    tempTimedHighNormalTrigger = tempTimed2HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowNormalTrigger = tempTimed2LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowTrigger = tempTimed2LowTrigger;
    if (!d2Shield) {
      offsetTemp = 0;
      offsetHumid = 0;
    }
    apssid = "ESPD2";
    blinkLED = blinkLEDA[2];
  }
  if (d3MAC == deviceMac) {
    printDeviceLeader();
    device = 3;
    Serial.print(device);
    d1MiniSituation = d3Situation;
    d1MiniSituationORG = d3Situation;
    d1MiniControl = d3Control;
    blynkPortT = 5;   //   Blynk Virtual Pin for the Temp data
    blynkPortH = 6;   //   Blynk Virtual Pin for the Humidity data
    blynkPortU = 13;   //   Blynk Virtual Pin to trigger sensor read and update
    blynkPortS = 18;   //   Blynk Virtual Pin for Serial Terminal Widget
    blynkPortReset = blynkPort3Reset;
    blynkPortOutput = blynkPort3ControlOutput;
    blynkPortlampCtl = blynkPort3lampCtl;
    blynkPortWKNDOR = blynkPort3WKNDOR;
    d1MiniLocation = d3Location;
    holdString = iftttEvent3LampOn;
    holdString.toCharArray(iftttEventLampOn, holdString.length() + 1);
    holdString = iftttEvent3LampOff;
    holdString.toCharArray(iftttEventLampOff, holdString.length() + 1);
    lampMorningOn = lampMorning3On;
    lampEveningOff = lampEvening3Off;
    lampEveningOnLatest = lampEvening3OnLatest;
    temp24HrHighTrigger = temp24Hr3HighTrigger;   //   deg C High Temperature where the webhook fires
    temp24HrHighNormalTrigger = temp24Hr3HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowNormalTrigger = temp24Hr3LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowTrigger = temp24Hr3LowTrigger;
    tempTimedHighTrigger = tempTimed3HighTrigger;   //   deg C High Temperature where the webhook fires
    tempTimedHighNormalTrigger = tempTimed3HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowNormalTrigger = tempTimed3LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowTrigger = tempTimed3LowTrigger;
    if (!d3Shield) {
      offsetTemp = 0;
      offsetHumid = 0;
    }
    apssid = "ESPD3";
    blinkLED = blinkLEDA[3];
  }
  if (d4MAC == deviceMac) {
    printDeviceLeader();
    device = 4;
    Serial.print(device);
    d1MiniSituation = d4Situation;
    d1MiniSituationORG = d4Situation;
    d1MiniControl = d4Control;
    blynkPortT = 7;   //   Blynk Virtual Pin for the Temp data
    blynkPortH = 8;   //   Blynk Virtual Pin for the Humidity data
    blynkPortU = 14;   //   Blynk Virtual Pin to trigger sensor read and update
    blynkPortS = 19;   //   Blynk Virtual Pin for Serial Terminal Widget
    blynkPortReset = blynkPort4Reset;
    blynkPortOutput = blynkPort4ControlOutput;
    blynkPortlampCtl = blynkPort4lampCtl;
    blynkPortWKNDOR = blynkPort4WKNDOR;
    d1MiniLocation = d4Location;
    holdString = iftttEvent4LampOn;
    holdString.toCharArray(iftttEventLampOn, holdString.length() + 1);
    holdString = iftttEvent4LampOff;
    holdString.toCharArray(iftttEventLampOff, holdString.length() + 1);
    lampMorningOn = lampMorning4On;
    lampEveningOff = lampEvening4Off;
    lampEveningOnLatest = lampEvening4OnLatest;
    temp24HrHighTrigger = temp24Hr4HighTrigger;   //   deg C High Temperature where the webhook fires
    temp24HrHighNormalTrigger = temp24Hr4HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowNormalTrigger = temp24Hr4LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowTrigger = temp24Hr4LowTrigger;
    tempTimedHighTrigger = tempTimed4HighTrigger;   //   deg C High Temperature where the webhook fires
    tempTimedHighNormalTrigger = tempTimed4HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowNormalTrigger = tempTimed4LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowTrigger = tempTimed4LowTrigger;
    if (!d4Shield) {
      offsetTemp = 0;
      offsetHumid = 0;
    }
    apssid = "ESPD4";
    blinkLED = blinkLEDA[4];
  }
  if (d5MAC == deviceMac) {
    printDeviceLeader();
    device = 5;
    Serial.print(device);
    d1MiniSituation = d5Situation;
    d1MiniSituationORG = d5Situation;
    d1MiniControl = d5Control;
    blynkPortT = 9;   //   Blynk Virtual Pin for the Temp data
    blynkPortH = 10;   //   Blynk Virtual Pin for the Humidity data
    blynkPortU = 15;   //   Blynk Virtual Pin to trigger sensor read and update
    blynkPortS = 20;   //   Blynk Virtual Pin for Serial Terminal Widget
    blynkPortReset = blynkPort5Reset;
    blynkPortOutput = blynkPort5ControlOutput;
    blynkPortlampCtl = blynkPort5lampCtl;
    blynkPortWKNDOR = blynkPort5WKNDOR;
    d1MiniLocation = d5Location;
    holdString = iftttEvent5LampOn;
    holdString.toCharArray(iftttEventLampOn, holdString.length() + 1);
    holdString = iftttEvent5LampOff;
    holdString.toCharArray(iftttEventLampOff, holdString.length() + 1);
    lampMorningOn = lampMorning5On;
    lampEveningOff = lampEvening5Off;
    lampEveningOnLatest = lampEvening5OnLatest;
    temp24HrHighTrigger = temp24Hr5HighTrigger;   //   deg C High Temperature where the webhook fires
    temp24HrHighNormalTrigger = temp24Hr5HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowNormalTrigger = temp24Hr5LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    temp24HrLowTrigger = temp24Hr5LowTrigger;
    tempTimedHighTrigger = tempTimed5HighTrigger;   //   deg C High Temperature where the webhook fires
    tempTimedHighNormalTrigger = tempTimed5HighNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowNormalTrigger = tempTimed5LowNormalTrigger;   //   deg C Normal Temperature where the webhook fires to reverse High or Low trigger
    tempTimedLowTrigger = tempTimed5LowTrigger;
    if (!d5Shield) {
      offsetTemp = 0;
      offsetHumid = 0;
    }
    apssid = "ESPD5";
    blinkLED = blinkLEDA[5];
  }
  tabindent(2); variableIndent();
  Serial.print(F("MAC Address")); printSpace(); Serial.println(deviceMac);
  Serial.println();
  startupSparkle();
}

void setupHooks() {
  //  Serial.println("setupHooks");
  String holdString;
  if (device == 1) {
    holdString = iftttEvent1High;
    holdString.toCharArray(iftttEventHigh, holdString.length() + 1);
    holdString = iftttEvent1HNormal;
    holdString.toCharArray(iftttEventHNormal, holdString.length() + 1);
    holdString = iftttEvent1Low;
    holdString.toCharArray(iftttEventLow, holdString.length() + 1);
    holdString = iftttEvent1LNormal;
    holdString.toCharArray(iftttEventLNormal, holdString.length() + 1);
    if (iftttEvent1OnS != "" && iftttEvent1OnW != "") {
      if (daylightSavings) {
        holdString = iftttEvent1OnS;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
      if (!daylightSavings) {
        holdString = iftttEvent1OnW;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
    } else {
      holdString = iftttEvent1On;
      holdString.toCharArray(iftttEventOn, holdString.length() + 1);
    }
    holdString = iftttEvent1Off;
    holdString.toCharArray(iftttEventOff, holdString.length() + 1);
  }
  if (device == 2) {
    holdString = iftttEvent2High;
    holdString.toCharArray(iftttEventHigh, holdString.length() + 1);
    holdString = iftttEvent2HNormal;
    holdString.toCharArray(iftttEventHNormal, holdString.length() + 1);
    holdString = iftttEvent2Low;
    holdString.toCharArray(iftttEventLow, holdString.length() + 1);
    holdString = iftttEvent2LNormal;
    holdString.toCharArray(iftttEventLNormal, holdString.length() + 1);
    if (iftttEvent2OnS != "" && iftttEvent2OnW != "") {
      if (daylightSavings) {
        holdString = iftttEvent2OnS;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
      if (!daylightSavings) {
        holdString = iftttEvent2OnW;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
    } else {
      holdString = iftttEvent2On;
      holdString.toCharArray(iftttEventOn, holdString.length() + 1);
    }
    holdString = iftttEvent2Off;
    holdString.toCharArray(iftttEventOff, holdString.length() + 1);
  }
  if (device == 3) {
    holdString = iftttEvent3High;
    holdString.toCharArray(iftttEventHigh, holdString.length() + 1);
    holdString = iftttEvent3HNormal;
    holdString.toCharArray(iftttEventHNormal, holdString.length() + 1);
    holdString = iftttEvent3Low;
    holdString.toCharArray(iftttEventLow, holdString.length() + 1);
    holdString = iftttEvent3LNormal;
    holdString.toCharArray(iftttEventLNormal, holdString.length() + 1);
    if (iftttEvent3OnS != "" && iftttEvent3OnW != "") {
      if (daylightSavings) {
        holdString = iftttEvent3OnS;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
      if (!daylightSavings) {
        holdString = iftttEvent3OnW;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
    } else {
      holdString = iftttEvent3On;
      holdString.toCharArray(iftttEventOn, holdString.length() + 1);
    }
    holdString = iftttEvent3Off;
    holdString.toCharArray(iftttEventOff, holdString.length() + 1);
  }
  if (device == 4) {
    holdString = iftttEvent4High;
    holdString.toCharArray(iftttEventHigh, holdString.length() + 1);
    holdString = iftttEvent4Low;
    holdString.toCharArray(iftttEventLow, holdString.length() + 1);
    holdString = iftttEvent4LNormal;
    holdString.toCharArray(iftttEventLNormal, holdString.length() + 1);
    if (iftttEvent4OnS != "" && iftttEvent4OnW != "") {
      if (daylightSavings) {
        holdString = iftttEvent4OnS;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
      if (!daylightSavings) {
        holdString = iftttEvent4OnW;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
    } else {
      holdString = iftttEvent4On;
      holdString.toCharArray(iftttEventOn, holdString.length() + 1);
    }
    holdString = iftttEvent4Off;
    holdString.toCharArray(iftttEventOff, holdString.length() + 1);
  }
  if (device == 5) {
    holdString = iftttEvent5High;
    holdString.toCharArray(iftttEventHigh, holdString.length() + 1);
    holdString = iftttEvent5HNormal;
    holdString.toCharArray(iftttEventHNormal, holdString.length() + 1);
    holdString = iftttEvent5Low;
    holdString.toCharArray(iftttEventLow, holdString.length() + 1);
    holdString = iftttEvent5LNormal;
    holdString.toCharArray(iftttEventLNormal, holdString.length() + 1);
    if (iftttEvent5OnS != "" && iftttEvent5OnW != "") {
      if (daylightSavings) {
        holdString = iftttEvent5OnS;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
      if (!daylightSavings) {
        holdString = iftttEvent5OnW;
        holdString.toCharArray(iftttEventOn, holdString.length() + 1);
      }
    } else {
      holdString = iftttEvent5On;
      holdString.toCharArray(iftttEventOn, holdString.length() + 1);
    }
    holdString = iftttEvent5Off;
    holdString.toCharArray(iftttEventOff, holdString.length() + 1);
  }
}
//
//   Reset Things Section
//
void checkReset() {
  //  Serial.println("resetCheck");
  resetCountdown = (4294967295 - millis()) / (24 * 60 * 60 * 1000);
  if (aHours == 23 && aMins == 45 && aSecs == 32) {
    aSecs = 33;
    Serial.print(F("Reset Countdown: Reset in")); printSpace(); Serial.print(resetCountdown); printSpace(); Serial.println(F("days"));
  }
  if (aHours == 10 && aMins == 10 && aSecs == 10) {
    aSecs = 11;
    Serial.print(F("Reset Countdown: Reset in")); printSpace(); Serial.print(resetCountdown); printSpace(); Serial.println(F("days"));
  }
  if (resetCountdown <= 5 && aMins == 10 && aSecs == 10) {
    aSecs = 11;
    Serial.println(F("Reboot Me Soon!"));
  }
  if (resetCountdown == 1 && aMins == 10 && aSecs == 10) {
    aSecs = 11;
    Serial.println(F("Rebooting Tomorrow!"));
  }
  if (firstRun) {
    firstRun = 0;
  }
  if (millis() >= (4294967295 - 30000)) {
    softResetD1();
  }
  timepassed = millis() - currentMillis;
}

void resetControlParameters() {
  highTempTriggered = 0;
  highTempTriggerSent = 0;
  highTempReset = 0;
  highTempResetSent = 0;
  lowTempTriggered = 0;
  lowTempTriggerSent = 0;
  lowTempReset = 0;
  lowTempResetSent = 0;
  timedHighTempTriggered = 0;
  timedHighTempTriggerSent = 0;
  timedHighTempReset = 0;
  timedHighTempResetSent = 0;
  timedLowTempTriggered = 0;
  timedLowTempTriggerSent = 0;
  timedLowTempReset = 0;
  timedLowTempResetSent = 0;
  timedOnControlTriggerSent = 0;
  timedOffControlTriggerSent = 0;
  timedOnControlTriggered = 0;
  timedOffControlTriggered = 0;
  timedOnTriggered = 0;
  timedOnTriggerSent = 0;
  timedOffTriggered = 0;
  timedOffTriggerSent = 0;
}

void softResetD1() {
  externalOutputOn = 2;
  sendWebhook();
  aMilli = 0;   //   Arduino Time variable holding last milli at last update
  aSecsUpdate = 0;   //   Arduino Time variable holding seconds since last update
  bUpdate = 0;   //   Blynk Update Container
  currentMillis = 0;   //   Define timer variable for Blynk loop
  currentMode = 0;   //   Tracking of mode to limit change triggers 0 = heating 1 = cooling
  daylightSavings = 0;   //   Daylight Savings indicator, default off
  delayLoopLEDTurnedOn = 0;   //   Define timer variable for Delay loop 1 (LED Blink)
  delayloopLEDTurnedOff = 0;   //   Define timer variable for Delay loop 3 (Local Timer for Arduino Time)
  delayLEDTurnedOn = false;   //   Define test variable for Delay loop 1 (LED Blink)
  delayLEDTurnedOff = false;   //   Define test variable for Delay loop 3 (Local Timer for Arduino Time)
  delayLoopArduinoTime = 0;   //   Define timer variable for Delay loop 2 (Timer for Queued multiple LED Blink)
  displayconfigtimer = 0;   //   Blynk Timer for determining to display config. Double Temp request within 10 seconds will display configuration
  delayDisplayLocalTime = 0;   //   Arduino Time delay variable for calculate time
  dstChanged = 0;   //   Daylight Savings changed indicator, default off
  epoch = 0;   //   epoch current time in seconds since 00:00:00 01/01/1970
  externalDeviceState = 0;   //   External deviceState Tracking (webhooks) -1=low on, 0=off, 1=high on
  externalOutputOn = 0;   //   0 = no change, 1 = On, 2 = off
  lampControlOn = 0;   //   0 = off, 1 = On
  externalCTLOverride = 0;   //   Floating Variable for external interference;
  externalTMOverride = 0;   //   Floating Variable for external interference;
  firstRun = 1;   //   First Run variable
  highTempReset = 0;   //   integer variable initilisation for tracking return to High Temp range
  highTempResetSent = 0;   //   integer variable initilisation for tracking sending of High Temp Reset
  highTempTriggered = 0;   //   integer variable initilisation for tracking High Temp Triggers
  highTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of High Temp Trigger
  keepAlivePrintLN = 0;   //   Line control for pralive
  ledOn = false;   //   Keep track of the led state
  ledSync = 0;   //   Control Variable for LED Sync Control
  lowTempReset = 0;   //   integer variable initilisation for tracking return to Low Temp range
  lowTempResetSent = 0;   //   integer variable initilisation for tracking sending of Low Temp Reset
  lowTempTriggered = 0;   //   integer variable initilisation for tracking Low Temp Triggers
  lowTempTriggerSent = 0;   //   integer variable initilisation for tracking sending of Low Temp Trigger
  resetCountdown = 49;   //   Countdown for reboot / reset cycle in days
  secondsElapsedTimeStamp = 0;   //   Arduino Time delay variable for display local time
  setupATime = 1;   //   Control variable for initial setup of Arduino Time
  switchStateChangeCTL = 2;   //   Did the Blynk switch change?
  resetControlParameters();   //   Call routine
  timedOperation = 0;   //   Variable to suspend 24hour return to normal triggers
  timeElapsedBlynk = 0;   //   Create an Instance for the timer function for the Blynk Keep Alive
  timeElapsedLED = 0;   //   Create an Instance for the timer function for the LED
  timeElapsedProg = 0;   //   Create an Instance for the timer function for the Program
  timeElapsedReadTimeDelay = 0;   //   Create an Instance for the timer function for the WebHook
  timeElapsedTimed = 0;   //   Create an Instance for the timer function for the Timed Management
  timeElapsedWebHook = 0;   //   Create an Instance for the timer function for the WebHook
  timepassed = 0;   //   Load Timer
  utcOffsetSeconds = 0;   //   0 seconds if UTC or GMT position, updated in void config()
  watchdogCounter = 0;
  watchdogLoop = 0;   //   Define timer variable for ITFFF watchdog loop
  watchdogRunning = false;   //   Define test variable for Watchdog timer
  webHookLoaded = 0;   //   Test if webhook loaded to queue commands
  weekendOverride = 0;
  setup();
}
//
//   Display Config Section
//
void displaySummary() {
  Serial.println();
  Serial.println(F("WEMOS D1 mini with SHT30 Temp / Humidity Sensor"));
  Serial.println();
  Serial.print(F("Version"));
  printColon(); printSpace(); serialPrintDecimal(Version); Serial.println();
  send_serial();
  d1DeviceDetails();
  send_serial();
  timedOperations();
  send_serial();
  r24HrTriggers();
  send_serial();
  timedtriggers();
  Serial.println();
}

void printScreenLine() {
  for (int i = 0 ; i < 80 ; i++) {
    Serial.print('_');
  }
}

void displayConfig() {
  if (!firstRun) {
    printScreenLine();
  }
  Serial.println();
  Serial.println(F("WEMOS D1 mini with SHT30 Temp / Humidity Sensor"));
  printScreenLine();
  Serial.println();
  Serial.println();
  send_serial();
  Serial.println(F("Configured to check temperature and humidity"));
  Serial.println(F("and report data to Blynk IoT platform, external"));
  Serial.println(F("trigger for data check, and activate too high "));
  Serial.println(F("and too low temperature response with webhooks"));
  Serial.println();
  send_serial();
  Serial.print(F("Version"));
  printColon(); printSpace(); serialPrintDecimal(Version); Serial.println();
  printScreenLine();
  Serial.println();
  Serial.println();
  send_serial();
  d1DeviceDetails();
  wifiAndOffsets();
  yield();
  checksAndTimers();
  timedOperations();
  pulses();
  blynkPorts();
  yield();
  r24HrTriggers();
  timedtriggers();
  printScreenLine();
  Serial.println();
}

void d1DeviceDetails() {
  Serial.print(F("Device Mac Address")); tabindent(2); variableIndent(); Serial.println(deviceMac);
  Serial.print(F("Device Number")); tabindent(3); variableIndent(); Serial.println(device);
  Serial.print(F("Device Situation")); tabindent(2); variableIndent();
  send_serial();
  if (d1MiniSituation == 1) {
    Serial.println(F("On Daytime Only"));
  }
  if (d1MiniSituation == 2) {
    printOnSpace(); printLineMandE();
  }
  if (d1MiniSituation == 3) {
    printOnSpace(); printLineMEandN();
  }
  if (d1MiniSituation == 4) {
    Serial.println(F("On for Working Hours"));
  }
  Serial.print(F("Device Control"));
  tabindent(3); send_serial(); variableIndent();
  if (d1MiniControl == 1) {
    Serial.println(F("D1 Controlled"));
  }
  if (d1MiniControl == 2) {
    Serial.println(F("Aircon/Self Managed Controlled"));
  }
  Serial.println();
  send_serial();
}

void wifiAndOffsets() {
  Serial.print(F("Configuration")); printColon(); Serial.println();
  Serial.println();
  Serial.print(F("WiFi SSID")); tabindent(3); variableIndent(); Serial.println(usewifissid);
  printTemperature(); printSpace(); Serial.print(F("Offset")); tabindent(2); variableIndent();
  send_serial();
  Serial.print(F("-")); Serial.print(offsetTemp); printlineDegC();
  Serial.print(F("Humidity Offset")); tabindent(3); variableIndent(); Serial.print(offsetHumid); Serial.println(F(" %RH"));
  send_serial();
}

void checksAndTimers() {
  Serial.print(F("Sensor check every"));
  tabindent(2); variableIndent(); Serial.print(intervalProg / 1000 / 60); printSpace(); Serial.println(F("minutes"));
  printTemperature(); printSpace(); Serial.print(F("Trigger check every"));
  tabindent(1); variableIndent(); Serial.print(intervalWebHook / 1000); printSpace(); Serial.println(F("seconds"));
  Serial.print(F("NTP Time Check/Sync every")); tabindent(1); variableIndent();
  send_serial();
  Serial.print(delayReadTime / 1000 / 60); printSpace(); Serial.println(F("minutes"));
  if (blinkLED) {   //   Report LED Blink on or Off
    Serial.print(F("Blink LED Status")); tabindent(2); variableIndent(); printOn(); Serial.println();
    Serial.print(F("Still alive LED flash every")); tabindent(1); variableIndent();
    Serial.print(intervalLED / 1000); printSpace(); Serial.println(F("seconds"));
  } else {
    Serial.print(F("Blink LED Status")); tabindent(1); variableIndent(); printOff(); Serial.println();
  }
  send_serial();
}

void timedOperations() {
  if (!firstRun) {
    printScreenLine();
  }
  Serial.println();
  printhomeMorning(); printSpaceStartHour(); tabindent(2); variableIndent();
  send_serial();
  Serial.print(homeMorningStartHr); printlineHoursEndSpace();
  printhomeMorning(); printSpaceFinishHour(); tabindent(1); variableIndent();
  Serial.print(homeMorningFinishHr); printlineHoursEndSpace();
  printhomeEvening(); printSpaceStartHour(); tabindent(2); variableIndent();
  send_serial();
  Serial.print(homeEveningStartHr); printlineHoursEndSpace();
  printhomeEvening(); printSpaceFinishHour(); tabindent(1); variableIndent();
  Serial.print(homeEveningFinishHr); printlineHoursEndSpace();
  printHomeNight(); printSpaceStartHour(); tabindent(2); variableIndent();
  send_serial();
  Serial.print(homeNightStartHr); printlineHoursEndSpace();
  printHomeNight(); printSpaceFinishHour(); tabindent(2); variableIndent();
  Serial.print(homeNightFinishHr); printlineHoursEndSpace();
  Serial.print(F("Work")); printSpaceStartHour(); tabindent(3); variableIndent();
  Serial.print(workStartHr); printlineHoursEndSpace();
  Serial.print(F("Work")); printSpaceFinishHour(); tabindent(2); variableIndent();
  Serial.print(workFinishHr); printlineHoursEndSpace();
  Serial.print(F("Sunrise")); printSpace(); tabindent(3); variableIndent();
  Serial.print(String(hour(sRise) < 10 ? "0" + String(hour(sRise)) : String(hour(sRise))));
  printColon();
  Serial.println(String(minute(sRise) < 10 ? "0" + String(minute(sRise)) : String(minute(sRise))));
  Serial.print(F("Sunset")); printSpace(); tabindent(4); variableIndent();
  Serial.print(String(hour(sSet) < 10 ? "0" + String(hour(sSet)) : String(hour(sSet))));
  printColon();
  Serial.println(String(minute(sSet) < 10 ? "0" + String(minute(sSet)) : String(minute(sSet))));
  send_serial();
  Serial.print(F("Lamp Sunrise/Sunset buffer is")); tabindent(1); variableIndent();
  Serial.print(lampGapMins); Serial.println(F(" minutes"));
  Serial.print(F("Sunrise Lamp Off Time")); tabindent(2); variableIndent();
  Serial.print(String(confSRiseH < 10 ? "0" + String(confSRiseH) : String(confSRiseH)));
  printColon();
  Serial.println(String(confSRiseM < 10 ? "0" + String(confSRiseM) : String(confSRiseM)));
  Serial.print(F("Sunset Lamp On Time")); tabindent(2); variableIndent();
  Serial.print(String(confSSetH < 10 ? "0" + String(confSSetH) : String(confSSetH)));
  printColon();
  Serial.println(String(confSSetM < 10 ? "0" + String(confSSetM) : String(confSSetM)));
  Serial.print(F("Sunset Latest Lamp On Time")); tabindent(2); variableIndent();
  Serial.print(lampEveningOnLatest); printColon(); Serial.println("00");
  if (!firstRun) {
    printScreenLine();
  }
  Serial.println();
}

void blynkPorts() {
  Serial.println();
  printBlynkSpace(); Serial.print(F("API Key")); tabindent(3); variableIndent();
  byte len = strlen(auth);
  Serial.println(&auth[len - 5]);
  printBlynkSpace(); printTemperature(); printSpacePort(); tabindent(2); variableIndent();
  printBlynkV(); Serial.println(blynkPortT);
  send_serial();
  printBlynkSpace(); printHumidity(); printSpacePort(); tabindent(2); variableIndent();
  printBlynkV(); Serial.println(blynkPortH);
  printBlynkSpace(); Serial.print(F("Update")); printSpacePort(); tabindent(2); variableIndent();
  printBlynkV(); Serial.println(blynkPortU);
  send_serial();
  printBlynkSpace(); Serial.print(F("Serial Terminal")); printSpacePort(); tabindent(1); variableIndent();
  printBlynkV(); Serial.println(blynkPortS);
  send_serial();
}

void r24HrTriggers() {
  if (!firstRun) {
    printScreenLine();
  }
  Serial.println();
  printWebTriggerSpace(); printHighSpace(); printTemperature(); tabindent(4); printSpace(); variableIndent();
  serialPrintDecimal(temp24HrHighTrigger); printlineDegC();
  //
  printWebTriggerSpace(); printHighSpace(); printTempAction(); tabindent(4); printSpace(); variableIndent();
  Serial.println(iftttEventHigh);
  send_serial();
  //
  printWebTriggerSpace(); printReturntoSpace(); printNormal(); printSpace(); printFromSpace(); printHighSpace();
  printTemperature(); tabindent(1); printSpace(); variableIndent(); serialPrintDecimal(temp24HrHighNormalTrigger);
  printlineDegC();
  //
  printWebTriggerSpace(); printReturntoSpace(); printNormal(); printSpace(); printFromSpace(); printHighSpace();
  printSpace(); printTempAction(); tabindent(1); printSpace(); variableIndent(); Serial.println(iftttEventHNormal);
  //
  printWebTriggerSpace(); printReturntoSpace(); printNormal(); printSpace(); printFromSpace(); printLowSpace();
  printTemperature(); tabindent(1); printSpace(); variableIndent();
  serialPrintDecimal(temp24HrLowNormalTrigger); printlineDegC();
  //
  send_serial();
  printWebTriggerSpace(); printReturntoSpace(); printNormal(); printSpace(); printFromSpace(); printLowSpace();
  printSpace(); printTempAction(); tabindent(1); printSpace(); variableIndent(); Serial.println(iftttEventLNormal);
  //
  printWebTriggerSpace(); printLowSpace(); printTemperature(); tabindent(4); printSpace(); variableIndent();
  serialPrintDecimal(temp24HrLowTrigger); printlineDegC();
  //
  printWebTriggerSpace(); printLowSpace(); printTempAction();
  tabindent(4); printSpace(); variableIndent(); Serial.println(iftttEventLow);
  //
  printWebTriggerSpace(); printTurn(); printSpace(); printOn(); printSpace(); printAction();
  tabindent(4); printSpace(); variableIndent(); Serial.println(iftttEventOn);
  //
  send_serial();
  printWebTriggerSpace(); printTurn(); printSpace(); printOff(); printSpace(); printAction();
  tabindent(4); printSpace(); variableIndent(); Serial.println(iftttEventOff);
  //
  send_serial();
  if (lampControl[device]) {
    Serial.println();
    printWebTriggerSpace(); printTurn(); printSpace(); Serial.print(F("Lamp"));
    printSpace(); printOn(); printSpace(); printAction();
    tabindent(4); printSpace(); variableIndent(); Serial.println(iftttEventLampOn);
    send_serial();
    printWebTriggerSpace(); printTurn(); printSpace(); Serial.print(F("Lamp"));
    printSpace(); printOff(); printSpace(); printAction();
    tabindent(3); printSpace(); variableIndent(); Serial.println(iftttEventLampOff);
  }
  send_serial();
}

void timedtriggers() {
  Serial.println();
  printWebTriggerSpace(); printTimedSpace(); printHighSpace(); printTemperature();
  tabindent(3); printSpace(); variableIndent(); serialPrintDecimal(tempTimedHighTrigger); printlineDegC();
  //
  printWebTriggerSpace(); printTimedSpace(); printHighSpace(); printTempAction();
  tabindent(3); printSpace(); variableIndent(); Serial.println(iftttEventHigh);
  //
  printWebTriggerSpace(); printTimedSpace(); printReturntoSpace(); printNormal(); printSpace();
  printFromSpace(); printHighSpace(); printTemperature(); printSpace(); variableIndent();
  serialPrintDecimal(tempTimedHighNormalTrigger); printlineDegC();
  send_serial();
  //
  printWebTriggerSpace(); printTimedSpace(); printReturntoSpace(); printNormal(); printSpace();
  printFromSpace(); printLowSpace(); printTemperature(); tabindent(1); printSpace(); variableIndent();
  serialPrintDecimal(tempTimedLowNormalTrigger); printlineDegC();
  send_serial();
  //
  printWebTriggerSpace(); printTimedSpace(); printHighSpace(); printNormal(); printSpace(); printTempAction();
  tabindent(2); printSpace(); variableIndent(); Serial.println(iftttEventHNormal);
  //
  printWebTriggerSpace(); printTimedSpace(); printLowSpace(); printNormal(); printSpace(); printTempAction();
  tabindent(2); printSpace(); variableIndent(); Serial.println(iftttEventLNormal);
  //
  printWebTriggerSpace(); printTimedSpace(); printLowSpace(); printTemperature();
  tabindent(3); printSpace(); variableIndent(); serialPrintDecimal(tempTimedLowTrigger); printlineDegC();
  send_serial();
  //
  printWebTriggerSpace(); printTimedSpace(); printLowSpace(); printTempAction();
  tabindent(3); printSpace(); variableIndent(); Serial.println(iftttEventLow);
  //
  printWebTriggerSpace(); printTimedSpace(); printTurn(); printSpace(); printOn(); printSpace(); printAction();
  tabindent(3); printSpace(); variableIndent(); Serial.println(iftttEventOn);
  //
  printWebTriggerSpace(); printTimedSpace(); printTurn(); printSpace(); printOff(); printSpace(); printAction();
  tabindent(3); printSpace(); variableIndent(); Serial.println(iftttEventOff);
  send_serial();
  keepAlivePrintLN = 0;
  if (!firstRun) {
    printScreenLine();
  }
  Serial.println();
}

void pulses() {
  printWork(); Serial.print(F("ing or alive pulse"));
  tabindent(2); variableIndent(); Serial.println(F("* or % when Timed Control Active"));
  printBlynkSpace(); Serial.print(F("keep alive pulse"));
  tabindent(2); variableIndent(); Serial.println(F("#"));
  send_serial();
  printTemperature(); printSpace(); printTrigger(); Serial.print(F("s Checked"));
  tabindent(1); variableIndent(); Serial.println(F("^"));
  printTemperature(); printSpace(); printTrigger(); printSpace(); printAction(); Serial.print(F("ed"));
  tabindent(1); variableIndent(); printCActive(); Serial.println();
  send_serial();
  Serial.print(F("LED Sync")); tabindent(3); variableIndent(); Serial.println(F("="));
  send_serial();
}
//
//   Serial print Consolidation Functions
//
void printSpace() {
  Serial.print(F(" "));
}

void printComma() {
  Serial.print(F(","));
}

void printColon() {
  Serial.print(F(":"));
}

void printCActive() {
  Serial.print(F(">"));
}

void printTAB() {
  Serial.print(F("\t"));
}

void printAction() {
  Serial.print(F("Action"));
}
void printBlynk() {
  Serial.print(F("Blynk"));
}

void printDay() {
  Serial.print(F("Day"));
}

void printDaytime() {
  Serial.print(F("Daytime"));
}

void printEvening() {
  Serial.print(F("Evening"));
}

void printFrom() {
  Serial.print(F("from"));
}

void printHome() {
  Serial.print(F("Home"));
}

void printHumidity() {
  Serial.print(F("Humidity"));
}

void printMorning() {
  Serial.print(F("Morning"));
}

void printNight() {
  Serial.print(F("Night"));
}

void printNormal() {
  Serial.print(F("Normal"));
}

void printOn() {
  Serial.print(F("On"));
}

void printOff() {
  Serial.print(F("Off"));
}

void printSent() {
  Serial.print(F("Sent"));
}

void printTempAction() {
  Serial.print(F("Temp Action"));
}

void printTemperature() {
  Serial.print(F("Temperature"));
}

void printTrigger() {
  Serial.print(F("Trigger"));
}

void printTurn() {
  Serial.print(F("Turn"));
}

void printBlynkV() {
  Serial.print(F("V"));
}

void printWeb() {
  Serial.print(F("Web"));
}

void printWork() {
  Serial.print(F("Work"));
}

void printAndSpace() {
  Serial.print(F("&")); printSpace();
}

void printlineHoursEndSpace() {
  Serial.print(F(":00")); printLineSpace();
}

void printData() {
  printSpace(); Serial.print(F("Data")); printColon();
}

void printlineDegC() {
  printSpace(); Serial.print(F("deg C")); printLineSpace();
}

void printSpaceFinishHour() {
  printSpace(); Serial.print(F("Finish Hour"));
}

void printFinishingSpace() {
  Serial.print(F("Finishing")); printSpace();
}

void printHighSpace() {
  Serial.print(F("High")); printSpace();
}

void printInputfromBlynk() {
  Serial.print(F("Input")); printSpace(); printFrom(); printSpace(); printBlynkSpace(); printColon(); printSpace();
  printBlynkV();
}

void printLowSpace() {
  Serial.print(F("Low")); printSpace();
}

void printSpacePort() {
  printSpace(); Serial.print(F("Port"));
}

void printReturntoSpace() {
  Serial.print(F("Return to")); printSpace();
}

void printSpaceStartHour() {
  printSpace(); Serial.print(F("Start Hour"));
}

void printStartingSpace() {
  Serial.print(F("Starting")); printSpace();
}

void printDeviceLeader() {
  Serial.print(F("This is Device")); printSpace();
}

void printTimedSpace() {
  Serial.print(F("Timed")); printSpace();
}

void printTriggeredSpace() {
  Serial.print(F("Triggered")); printSpace();
}

void printWebhook() {
  printWeb(); Serial.print(F("hook"));
}

void printWebHooksActive() {
  printSpace(); printWebhook(); Serial.println(F("s Active"));
}

void printWeekdaySpace() {
  Serial.print(F("Weekday")); printSpace();
}

void printWeekendSpace() {
  Serial.print(F("Weekend")); printSpace();
}

void printTTWS() {
  printTemperature(); printSpace(); printTrigger(); printSpace(); printWebhookSent();
}

void printhomeEvening() {
  printHome(); printSpace(); printEvening();
}

void printhomeMorning() {
  printHome(); printSpace(); printMorning();
}

void printOnSpace() {
  printOn(); printSpace();
}

void printBlynkSpace() {
  printBlynk(); printSpace();
}

void printHomeNight() {
  printHome(); printSpace(); printNight();
}

void printWebhookSent() {
  printWebhook(); printSpace(); printSent();
}

void printWebTriggerSpace() {
  printWeb(); printSpace(); printTrigger(); printSpace();
}

void printColonSpace() {
  printColon(); printSpace();
}

void printFromSpace() {
  printFrom(); printSpace();
}

void printCommaSpace() {
  printComma(); printSpace();
}

void variableIndent() {
  printCActive(); printColon(); printSpace();
}

void printlineColon() {
  printColon(); Serial.println();
}

void printLineDaytime() {
  printDaytime(); Serial.println();
}

void printLineMandE() {
  printMorning(); printSpace(); printAndSpace(); printEvening(); Serial.println();
}

void printLineMDandE() {
  printMorning(); printCommaSpace(); printDaytime(); printCommaSpace(); printAndSpace(); printEvening(); Serial.println();
}

void printLineMEandN() {
  printMorning(); printCommaSpace(); printEvening(); printCommaSpace(); printAndSpace(); printNight(); Serial.println();
}

void printLineMTE() {
  printMorning(); printSpace(); Serial.print(F("through")); printSpace(); printDaytime(); printSpace();
  printAndSpace(); printEvening(); Serial.println();
}

void printLineSpace() {
  printSpace(); Serial.println();
}

void printlineTemperature() {
  printTemperature(); Serial.println();
}

void printlineTriggered() {
  printSpace(); printTriggeredSpace(); Serial.println();
}

void printLineWorkDay() {
  printWork(); printSpace(); printDay(); Serial.println();
}
//
//   Functions Section
//
char *append_str(char *here, char *s) {   //   string append function for building the json string
  while (*here++ = *s++)
    ;
  return here - 1;
}

char *append_ul(char *here, unsigned long u) {   //   variable append function for building the json string
  char buf[20];   //   we "just know" this is big enough
  return append_str(here, ultoa(u, buf, 10));
}

void serialPrintDecimal(int numberToPrint) {   //   print a number to screen with 2 decimal places stored as an integer. ie 2450 = 24.50
  Serial.print(numberToPrint / 100); Serial.print(".");
  numberToPrint = numberToPrint % 100;
  int decis = 10;
  if (!numberToPrint) {
    Serial.print("00");
  } else {
    while ( decis > 0) {
      if (numberToPrint < decis) {
        Serial.print("0");
        decis /= 10;
      } else {
        decis = 0;
      }
    }
    Serial.print(numberToPrint);
  }
}

void startupSparkle() {
  for (int i = 0; i < 2; i++) {
    delay(50);
    digitalWrite(LED_BUILTIN, turn_On);
    delay(50);
    digitalWrite(LED_BUILTIN, turn_Off);
  }
}

void startupPause() {
  for (int t = 500; t > 0; t = t - 100) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_BUILTIN, turn_On);
      delay(t);
      digitalWrite(LED_BUILTIN, turn_Off);
      delay(t);
    }
  }   //   6.8 second start up delay, settle things down.
}
