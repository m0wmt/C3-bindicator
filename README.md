# ESP32-C3 with WS2812B LEDs

My version of 'bindicator' originally written by Darren Tarbard. 

This project was written using Visual Studio Code and the PlatformIO extension to manage the project.

This project has 2 methods for retrieving what bin type is due for emptying in the current week. The first is a very simple node.js application I wrote on our website (I can not do what I want due to limited access) which returns the bin type and the second method is to download a .json file and do the logic on the ESP32-C3.  Due to our local council only releasing a picture of the bin collections at the biginning of each year I have to manually enter the values in to the .csv and .json files.  Due to the (current) limited access to our website because of the hosting company I am unable to install the libraries I would like to so I just used the .json file for both methods.

I am looking at storing the information in weekly segments to reduce the size of the file(s) but went for the quick and dirty method of one line per day to start with.

ESP32-C3 information: Chip Model ESP32-C3, ChipRevision 4, Cpu Freq 160, SDK Version v4.4.2

ESP32-C3 and LEDs purchased via Aliexpress.

Libraries used: NeoPixel, ArduinoJson.

3D model of bin courtesy of Printables: https://www.printables.com/model/189899-wheelie-bin-for-the-bindicatorbindaycator, printed by my son-in-law

The Bin will light up a specific colour which will correspond to the colour of the bin that needs taking out that week. E.g. Red for general waste, Green and Blue for garden waste and recycling, and just Blue for the week our local council only collects recycling.

Currently in development.

Bin
![alt text width="500"](/pictures/dev.jpg)

ESP32-C3
![alt text width="250"](/pictures/c3.jpg)

