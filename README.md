# ESP32-C3 with WS2812B LEDs

My version of 'bindicator' originally written by Darren Tarbard. 

This project was written using Visual Studio Code and the PlatformIO extension to manage the project.

This project has 2 methods for retrieving what bin type is due for emptying in the current week. The first is a very simple node.js application I wrote on our website (I can not do what I want due to limited access because of the hosting company) which returns the bin type and the second method is to download a .json file and do the logic on the ESP32-C3.  Due to our local council only releasing a pdf of the bin collections at the biginning of each year I have to manually enter the values in to the .json file.  Due to the (current) limited access to our website I am unable to install the node.js libraries I would like to so I can just use the .json file for both methods.

Update: I have now created a simple python script (createjsonfile.py) to create the .json file that this program requires.  It's specific to this program and requires modification to set up the start date.  It expects the first week it generates to be an 'rgf' week.  I might revisit this in the future and give it command line options so save modifying every time I need to run it.  Note: No more than 1 years of data should be created as that's as much space I've allocated in the program if it has to download the .json file and work out the bin type on the micro processor.

I am looking at storing the information in weekly segments to reduce the size of the file(s) but went for the quick and dirty method of one line per day to start with.

ESP32-C3 information: Chip Model ESP32-C3, ChipRevision 4, Cpu Freq 160, SDK Version v4.4.2

ESP32-C3 and LEDs purchased via Aliexpress.

Libraries used: NeoPixel, ArduinoJson.

3D model of bin courtesy of Printables: https://www.printables.com/model/189899-wheelie-bin-for-the-bindicatorbindaycator, printed by my son-in-law

The Bin will light up a specific colour which will correspond to the colour of the bin that needs taking out that week. E.g. Red for general waste, Green and Blue for garden waste and recycling, and just Blue for the week our local council only collects recycling.

Now deployed at our house and my daughters, who requested this project as she kept forgetting which bin to put out each week!

Before assembly
![alt text width="100"](/pictures/assembly.jpg)

Inside the bin!
![alt text width="100"](/pictures/inside.jpg)

Recycling week (Garden and Recycling waste)
![alt text width="100"](/pictures/recyclingweek.jpg)

Development - Non recylcing week
![alt text width="100"](/pictures/dev.jpg)

ESP32-C3
![alt text width="100"](/pictures/c3.jpg)

