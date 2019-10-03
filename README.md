# ArduinoHmAutomate
Arduino distributed Home Automation for the home DIY hacker/builder.

I have takes a bunch of things and pulled them together to make an automation system. The sort of thing you can do block by block and not have a lot of upfront cost. Pick up an arduino and a wifi switch and you have some basic control over a heater or a lamp. 
Or with some slight modification, a jug or a toaster. The idea is to remain flexible with the approach.

The Problem I had:

+ Plug in heating that isn't very efficent on energy and the onboard thermostat is great for telling the temperature of the heater but not the room.
+ Manual Timers and temperature controllers have helped, but don't have enough flexibility for extreme temperature management, especially in the work office where the window rattler aircon units are dumb as a rock and don't have timers or temperature trigger options.

So a twofold issue, manage the devices on and off for the temperature control directly at home, and in the office managing the aircon turning on and off but leave them be, outside of switching between heating and cooling.

This solution I have saved about 30% on our typical electricity heating bill for both Aircon and convection heating/cooling systems and I have added additional functionality with the office of over heating protection. As it can get to 40+ deg C in the summer with no aircon running. 

This project covers off a number of things, so ideal for the Ardunino enthusist who also wants to integrate control with wifi controllers and data collection platforms. 

For managing:

+ Heating and/or cooling connected to normal power socket. Great for when you can't get into the walls, so to speak.
+ Temp and humidity monitoring on the Blynk platform
+ Connectivity to Magic Mirror though a http json API for further reporting/monitoring
+ Control lighting/or lamp for morning and evening on/off control. No tripping over in the dark stuff.
+ Blynk tools for managing data collection and reporting.

Designed and tested to date with the following equipment:

+ Wemos D1 mini ESP8266 WiFi connected Arduino Clones x 5 per configuration
+ SHT30 D1 mini shield for temp and humidity (see code notes for stacked vs seperated temp/humidity considerations)
+ Blynk platform and Blynk on iPhone and Android for monitor and control
+ IFTTT as central tool for interface between D1 mini and the device being controlled.
+ TP-Link WiFi switches, frankly found them unreliable for this implemntation as they dropped off connectivity too often and they have an interanl security challenge with users on the network if they are so inclined.
+ Sonoff S26 Wifi Switches, have been great devices, need a good WiFi signal but more stable on the connectivity. Connect through the AWS platform so less of a security concern. added advantage is they are ESP8266 Arduinos inside too, so there is prospect of flashing your own stuff on these later ;) 
+ Logitech Harmony Remote Hub, this device is the bomb if you are looking for an off the shelf option for IR control that is WiFi connected and has bluetooth control too. Added benefit is it has a 'normal' fully programmable remote controller for those less technical to still drive stuff.

A few design considerations:

+ Those experienced reading the code will note straight off that the code is huge from a singel device control perspective. And yes it is. It was done this way to make it easier to manage the device setup and management, and I didn't forsee it gettign quite this big.
+ The code is designed to accomodate up to 5 controllers and their configuration. Meaning you have one code file to manage, rather than 5 individual ones. This also flows through to the Blynk platform too. And the combinaiton of the two drove my decision to stick to one file for the code.
+ The Bkynk platform has the capacity to have multiple devices loaded, which also comes with multiple access keys, one for each device. However, the Blynk platform has 256 individual ports per device as well, far more than is needed for this project.
+ So to manage the device structure and setup in one file on one screen with one Blynk device key, made a lot of sense. Especially when I can take the file on screen plug in any of the Ardunio's and upload. The code figures out which one is which and how it operates, I just need to put them bakc in the right rooms.
+ I made a call early on to keep the Arduino hardware simple, board and temp sensor, which happened to also come with a humidity one.
+ This meant that I wasn't relying on coding every little piece and I could get something basic working fairly quickly. 
+ Already beign familar with IFTTT adding in a compatible WiFi switch sounded like a good idea. Which also gave me remote/phone control if things got a bit haywire. As they have from time to time. Downside, you're relying on more communicaion points to make it all work. If you have a stable internet connection it's not a big deal. 
+ One of my first cuts of this is this instructable, which also has much of the initial build stuff neede for the physical Arduino including the links to the devices I purchased https://www.instructables.com/id/WEMOS-D1-TempHumidity-IoT/

The D1 mini ESP8266 devices are quite versitile for this application and whic they can be clocked at 160Mhz, running at the default 80Mhz is fine for this purpose. 

A couple of other considerations:

+ I could add an infra-red controller to the Arduino instead of the Logitech remote Hub, though I already had that in place for another reason and it reduced the burden of the build in gettign things working. Maybe an addition for later on.
+ The other is the WiFi switch over adding an Arduino relay to the controlled devices, heaters in my case. This was again about off the shelf working kit, and it also means I have both less modificaiton of the existign equipment to do, and given it is mains voltage, less exposure to me havign a live wire get free or some otehr hazardous stuff like that. Buring down the house isn't part of the plan.

With the design considerations, I found that the D1's have the expected MAC address which enabled me to distinguish which was which and thus use the same code file for all of them.
+ So the trick here is to load the software on the device and have it run while monitoring in the Arduino IDE serial monitor, when they boot, the code will display the MAC address fo the device as it starts up. 
+ Grab that verbatim (case sensitive) and put it into the MAC address area for that device in your setup D1 - D5... 
+ If you are looking at the MAC address and also watching the device monitor on your WiFi router, you will find the MAC address provided with the Ardunio code is in the reverse order to the router. 
+ I based this part on the routine from a code discussion in one of the forums, not sure which and who's code, but atribution for that if someone does spot it, please point it out and I'll link it in. The fact it is backwards is sort of irrelevant for the job at hand, so I've left it be for this reason. I may address it in the future, for now other things are more pressing.

So basic setup:
+ Grab and Arduino and shield, with a WiFi switch. 
+ You'll need an IFTTT account with access to the maker.ifttt.com section. This will give you the API key you need for the code before you flash the Arduino.
+ make sure you have you WiFi network(s) entered in the right area, there is provision for 5 different networks, if you have distance and stuff involved you may need them all... Also keep in mind they need to be 2.4Ghz networks, no 5Ghz on these basic devices.
+ Link up the WiFi switch to IFTTT:
+ Make sure the provider software is working and you can control the switch form the provider software.
+ Find the provider in IFTTT and link it up, as in login and authorise the provider platform.
+ Then find create your own applets and get started with building your webhooks.
+ If This: Find the webhooks service, this will then enable you to define your webhook name to go with your maker API authorisation code from earlier. Give you webhook a name. i.e., if it's the kitchen heater, and you're turning it on, maybe call it Kitchen-Heater-On (this is the name you add to the Code for the Low Temp Trigger as well as the Standard On IFTTT event name)
+ Create that
+ Now come back to the next + That; Find your switch provider, select what you are doing, (Turning the device on), and select yoru device that you setup earlier.
+ And finish.

So this should be enough to create the 'on' aspect for your first controlled device for your first Arduino device.
+ Repeat for the off function, i.e Kitchen-Heater-Off
+ And you can do the same for a lamp in the same room with another WiFi switch too, again an On trigger and an Off trigger.
+ Loading this into the Arduino, and kicking it alight, you should get some idea on what's going on in the serial monitor. CMD/Ctrl + Shift + M in the Arduino IDE.

The next step is to add in the Blynk platform:
+ This is designed for the IoT revolution and is a smart platform which allows you to manage and control things with an app for next to nothing, yes, there is a cost to this. 
+ Grab your mobile and go find the Blynk app. You can also check out Blynk on www.blynk.io too, though it will only work on a mobile device.
+ in the Bkynk folder there are/will be two image files, one for a 4 device setup and one for a 5 device setup. The image files will aloow you to 'copy' across the Blynk IoT project(s) I have setup for this project. The 4 device one is a bit cheaper to get going with. The 5 device one has more stuff as this is the main one I use.
+ The Blynk platform is going to give you both visibility as well as control of your automation.

If you have got to this point, you're probably well on the track to making things work.

A couple of things to note:
This has been built in New Zealand, so the default timezone is likely the opposite side of the world to you, so update it accordingly.
The code uses 

More to come as I can document, the code does have a lot of documentation. And reporting on what is configured and going on can be monitored through the serial monitor in your Arduino IDE or suitable serial interface. 
