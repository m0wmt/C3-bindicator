Currently in development. 

Tasks:
- Recieve/transmit packets and extract iBoost information - Nearly Done...
- Feed data to existing MQTT queue on the Raspberry Pi
- Raspberry Pi - Write information to website (and possible InfluxDb)
- Use a different board/add a display

# iBoost Monitor

[Marlec iBoost] Monitor (https://www.marlec.co.uk/product/solar-iboost/)

This project is based on the original by [JMSwanson / ESP-Home-iBoost](https://github.com/JNSwanson/ESP-Home-iBoost) which integrates with ESPHome which I currently do not use.

This project uses an ESP32 and a [CC1101 TI radio module](https://www.ti.com/lit/ds/symlink/cc1100.pdf).  It was written using 
VSCode and the PlatformIO plug-in. I used the same radio library as JMSwanson as it works.

Currently using an ESP32 Wroom 32 and a CC1101 Module purchased from eBay.

## Wiring 

(images/iBoostMonitor.png)

## Frequency tuning

The CC1101 modules can be a little off with the frequency.  This affects the quality of the received packets and can dramatically decrease the range at which you can receive packets from the iBoost.
You can buy a better Xtal like an Epson X1E0000210666 from Farnell (2471832), or change the frequency in initialisation. For my module I needed to set the frequency to 838.35MHz, I wasn't prepared to do any SMD soldering!

Below are some suggested values.  The default is/should be 868300000 Hz.

|    Freq   | Dec     | HEX    |
|:---------:|---------|--------|
| 868425000 | 2188965 | 2166A5 |
| 868400000 | 2188902 | 216666 |
| 868375000 | 2188839 | 216627 |
| 868350000 | 2188776 | 2165E8 |
| 868325000 | 2188713 | 2165A9 |
| *868300000* | *2188650* | *21656A* |
| 868275000 | 2188587 | 21652B |
| 868250000 | 2188524 | 2164EC |
| 868225000 | 2188461 | 2164AD |
| 868200000 | 2188398 | 21646E |
| 868175000 | 2188335 | 21642F |


To make these changes you will need to change these lines:
```
radio.writeRegister(CC1101_FREQ2, 0x21);
radio.writeRegister(CC1101_FREQ1, 0x65);
radio.writeRegister(CC1101_FREQ0, 0x6a);
```

Look at the LQI value in the debug output for an indication of received packet quality, lower is better.  

When looking at the debug output of the received packets are printed. The third byte represents the source of the packet:
- 01 - Clamp sensor / sender
- 21 - iBoost Buddy sending a request
- 22 - iBoost main unit metrics data

You should optimize receive quality for the iBoost main unit (0x22). I don't have in iBoost Buddy so I never saw any of those packets.

## CC1101 Packet Format

(images/cc1101-packet-format.png

## ESP32 Wroom 32 Pinout

(images/ESP32-pinout-30pins.png)