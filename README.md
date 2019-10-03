# ArduinoHmAutomate
Arduino distributed Home Automation for the home DIY hacker/builder

Have saved about 30% on our typical electricity heating bill for both Aircon and convection heating/cooling systems.

For managing:
Heating and/or cooling connected to normal power socket. Great for when you can't get into the walls, so to speak.
Temp and humidity monitoring on the Blynk platform
Connectivity to Magic Mirror though a http json API for further reporting/monitoring
Control lighting/or lamp for morning and evening
Blynk tools for managing data collection and reporting

Designed and tested to date with:
Wemos D1 mini ESP8266 WiFi connected Arduino Clones x 5 per configuration
SHT30 D1 mini shield for temp and humidity (see code notes for stacked vs seperated temp/humidity considerations)
Blynk platform and Blynk on iPhone and Android for monitor and control
IFTTT as central tool for interface between D1 mini and the device being controlled.
TP-Link WiFi switches, frankly found them unreliable for this implemntation as they dropped off connectivity too often and they have an interanl security challenge with users on the network if they are so inclined.
Sonoff S26 Wifi Switches, have been great devices, need a good WiFi signal but more stable on the connectivity. Connect through the AWS platform so less of a security concern.

More to come as I can document, the code does have a lot of documentation. And reportign on what is configured and goign on can be monitored through the serial monitor in your Arduino IDE or suitable serial interface. 
