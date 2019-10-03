# ArduinoHmAutomate
Arduino distributed Home Automation for the home DIY hacker/builder.

I have takes a bunch of things and pulled them together to make an automation system. The sort of thing you can do block by block and not have a lot of upfront cost. Pick up an arduino and a wifi switch and you have some basic control over a heater or a lamp. 
Or with some slight modification, a jug or a toaster. The idea is to remain flexible with the approach.

The Problem I had:

Plug in heating that isn't very efficent on energy and the onboard thermostat is great for telling the temperature of the heater but not the room.
Manual Timers and temperature controllers have helped, but don't have enough flexibility for extreme temperature management, especially in the work office where the window rattler aircon units are dumb as a rock and don't have timers or temperature trigger options.

So a twofold issue, manage the devices on and off for the temperature control directly at home, and in the office managing the aircon turning on and off but leave them be, outside of switching between heating and cooling.

This solution I have saved about 30% on our typical electricity heating bill for both Aircon and convection heating/cooling systems and I have added additional functionality with the office of over heating protection. As it can get to 40+ deg C in the summer with no aircon running. 

This project covers off a number of things, so ideal for the Ardunino enthusist who also wants to integrate control with wifi controllers and data collection platforms. 

For managing:

Heating and/or cooling connected to normal power socket. Great for when you can't get into the walls, so to speak.
Temp and humidity monitoring on the Blynk platform
Connectivity to Magic Mirror though a http json API for further reporting/monitoring
Control lighting/or lamp for morning and evening on/off control. No tripping over in the dark stuff.
Blynk tools for managing data collection and reporting.

Designed and tested to date with the following equipment:

Wemos D1 mini ESP8266 WiFi connected Arduino Clones x 5 per configuration
SHT30 D1 mini shield for temp and humidity (see code notes for stacked vs seperated temp/humidity considerations)
Blynk platform and Blynk on iPhone and Android for monitor and control
IFTTT as central tool for interface between D1 mini and the device being controlled.
TP-Link WiFi switches, frankly found them unreliable for this implemntation as they dropped off connectivity too often and they have an interanl security challenge with users on the network if they are so inclined.
Sonoff S26 Wifi Switches, have been great devices, need a good WiFi signal but more stable on the connectivity. Connect through the AWS platform so less of a security concern.
Logitech Harmony Remote Hub, this device is the bomb if you are looking for an off the shelf option for IR control that is WiFi connected and has bluetooth control too. Added benefit is it has a 'normal' fully programmable remote controller for those less technical to still drive stuff.

A few design considerations:

Those experienced reading the code will note straight off that the code is huge from a singel device control perspective. And yes it is. It was done this way to make it easier to manage the device setup and management, and I didn't forsee it gettign quite this big.

The code is designed to accomodate up to 5 controllers and their configuration. Meaning you have one code file to manage, rather than 5 individual ones. This also flows through to the Blynk platform too. And the combinaiton of the two drove my decision to stick to one file for the code.

The Bkynk platform has the capacity to have multiple devices loaded, which also comes with multiple access keys, one for each device. However, the Blynk platform has 256 individual ports per device as well, far more than is needed for this project.

So to manage the device structure and setup in one file on one screen with one Blynk device key, made a lot of sense. Especially when I can take the file on screen plug in any of the Ardunio's and upload. The code figures out which one is which and how it operates, I just need to put them bakc in the right rooms.

I made a call early on to keep the Arduino hardware simple, board and temp sensor, which happened to also come with a humidity one.
This meant that I wasn't relying on coding every little piece and I could get something basic working fairly quickly. 

Already beign familar with IFTTT adding in a compatible WiFi switch sounded like a good idea. Which also gave me remote/phone control if things got a bit haywire. As they have from time to time. Downside, you're relying on more communicaion points to make it all work. If you have a stable internet connection it's not a big deal. 

One of my first cuts of this is this instructable, which also has much of the initial build stuff neede for the physical Arduino including the links to the devices I purchased https://www.instructables.com/id/WEMOS-D1-TempHumidity-IoT/

The D1 mini ESP8266 devices are quite versitile for this application and whic they can be clocked at 160Mhz, running at the default 80Mhz is fine for this purpose. 

With the design considerations, I found that the D1's have the expected MAC address which enabled me to distinguish which was which and thus use the same code file for all of them.

So the trick here is to load the software on the device and have it run while monitoring in the Arduino IDE serial monitor, when they boot, the code will display the MAC address fo the device as it starts up. 
Grab that verbatim and put it into the MAC address area for that device in your setup D1 - D5... 
If you are looking at the MAC address and also watchign the device monitor on your WiFi router, you will find the MAC address provided witht he Ardunio code is in the reverse order to the router. 
I based this part on the routine from a code discussion in one of the forums, not sure which and who's code, by atribution for that if someone does spot it, please point it out and I'll link it in. The fact it is backwards is sort of irrelevant for the job at hand, so I've left it be for this reason.





More to come as I can document, the code does have a lot of documentation. And reporting on what is configured and going on can be monitored through the serial monitor in your Arduino IDE or suitable serial interface. 
