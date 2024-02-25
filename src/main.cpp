#include <Arduino.h>
#include <SPI.h>
#include "CC1101_RFx.h"

#define SS_PIN 5
#define MISO_PIN 19
#define GDO0_PIN 2

// SCK_PIN = 18; MISO_PIN = 19; MOSI_PIN = 23; SS_PIN = 5; GDO0 = 2;
CC1101 radio(SS_PIN,  MISO_PIN);   // SS (CSN), MISO

// codes for the various requests and responses
enum { 
    SAVED_TODAY = 0xCA,
    SAVED_YESTERDAY = 0xCB,
    SAVED_LAST_7 = 0xCC,
    SAVED_LAST_28 = 0xCD,
    SAVED_TOTAL = 0xCE
};

long today, yesterday, last7, last28, total;
  
uint32_t pingTimer;         // used for the periodic pings see below
uint32_t ledTimer;          // used for LED blinking when we receive a packet
uint32_t rxTimer;  
uint8_t txBuf[32];
uint8_t request;
bool boostRequest;
uint8_t boostTime;
uint8_t address[2];         // this is the address of the sender
uint8_t addressLQI, rxLQI;  // signal strength test 
bool addressValid;
byte packet[255];

	
void setup()  {
    addressLQI = 255; // set received LQI to lowest value
    addressValid = false;

    SPI.begin();

    Serial.begin(115200);
    Serial.println();
    Serial.println("SPI OK");
    
    radio.reset();
    radio.begin(868.350e6); // Freq, do not forget the "e6"
    radio.setMaxPktSize(61);
    radio.writeRegister(CC1101_FREQ2, 0x21); 
    radio.writeRegister(CC1101_FREQ1, 0x65);
    radio.writeRegister(CC1101_FREQ0, 0xe8);
    radio.writeRegister(CC1101_FSCTRL1, 0x08); // fif=203.125kHz
    radio.writeRegister(CC1101_FSCTRL0, 0x00); // No offset
    radio.writeRegister(CC1101_MDMCFG4, 0x5B); // CHANBW_E = 1 CHANBW_M=1 BWchannel =325kHz   DRATE_E=11
    radio.writeRegister(CC1101_MDMCFG3, 0xF8); // DRATE_M=248 RDATA=99.975kBaud
    radio.writeRegister(CC1101_MDMCFG2, 0x03); // Disable digital DC blocking filter before demodulator enabled. MOD_FORMAT=000 (2-FSK) Manchester Coding disabled Combined sync-word qualifier mode = 30/32 sync word bits detected
    radio.writeRegister(CC1101_MDMCFG1, 0x22); // Forward error correction disabled 4 preamble bytes transmitted CHANSPC_E=2
    radio.writeRegister(CC1101_MDMCFG0, 0xF8); // CHANSPC_M=248 200kHz channel spacing
    radio.writeRegister(CC1101_CHANNR, 0x00); // The 8-bit unsigned channel number, which is multiplied by the channel spacing setting and added to the base frequency.
    radio.writeRegister(CC1101_DEVIATN, 0x47); // DEVIATION_E=4 DEVIATION_M=7 ±47.607 kHz deviation
    radio.writeRegister(CC1101_FREND1, 0xB6); // Adjusts RX RF device
    radio.writeRegister(CC1101_FREND0, 0x10); // Adjusts TX RF device
    radio.writeRegister(CC1101_MCSM0, 0x18); // Calibrates whngoing from IDLE to RX or TX (or FSTXON) PO_TIMEOUT 149-155uS Pin control disabled XOSC off in sleep mode
    //radio.writeRegister(CC1101_MCSM1, 0x00); // Channel clear = always Return to idle after packet reception Return to idle after transmission
    radio.writeRegister(CC1101_FOCCFG, 0x1D); // The frequency compensation loop gain to be used before a sync word is detected = 4K The frequency compensation loop gain to be used after a sync word is Detected = K/2 The saturation point for the frequency offset compensation algorithm = ±BWchannel /8
    radio.writeRegister(CC1101_BSCFG, 0x1C); // The clock recovery feedback loop integral gain to be used before a sync word is detected = KI The clock recovery feedback loop proportional gain to be used before a sync word is detected = 2KP The clock recovery feedback loop integral gain to be used after a sync word is Detected = KI/2 The clock recovery feedback loop proportional gain to be used after a sync word is detected = KP The saturation point for the data rate offset compensation algorithm = ±0 (No data rate offset compensation performed)
    radio.writeRegister(CC1101_AGCCTRL2, 0xC7); // The 3 highest DVGA gain settings can not be used. Maximum allowable LNA + LNA 2 gain relative to the maximum possible gain. Target value for the averaged amplitude from the digital channel filter = 42dB
    radio.writeRegister(CC1101_AGCCTRL1, 0x00); // LNA 2 gain is decreased to minimum before decreasing LNA gain Relative carrier sense threshold disabled Sets the absolute RSSI threshold for asserting carrier sense to MAGN_TARGET
    radio.writeRegister(CC1101_AGCCTRL0, 0xB2); // Sets the level of hysteresis on the magnitude deviation (internal AGC signal that determine gain changes) to Medium hysteresis, medium asymmetric dead zone, medium gain Sets the number of channel filter samples from a gain adjustment has been made until the AGC algorithm starts accumulating new samples to 32 samples AGC gain never frozen
    radio.writeRegister(CC1101_FSCAL3, 0xEA); // Detailed calibration
    radio.writeRegister(CC1101_FSCAL2, 0x2A); //
    radio.writeRegister(CC1101_FSCAL1, 0x00); //
    radio.writeRegister(CC1101_FSCAL0, 0x1F); //
    radio.writeRegister(CC1101_FSTEST, 0x59); // Test register
    radio.writeRegister(CC1101_TEST2, 0x81); // Values to be used from SmartRF software
    radio.writeRegister(CC1101_TEST1, 0x35); //
    radio.writeRegister(CC1101_TEST0, 0x09); //
    radio.writeRegister(CC1101_IOCFG2, 0x0B); // Active High Serial Clock
    radio.writeRegister(CC1101_IOCFG0, 0x46); // Analog temperature sensor disabled Active High Asserts when sync word has been sent / received, and de-asserts at the end of the packet
    radio.writeRegister(CC1101_PKTCTRL1, 0x04); // Sync word is always accepted Automatic flush of RX FIFO when CRC is not OK disabled Two status bytes will be appended to the payload of the packet. The status bytes contain RSSI and LQI values, as well as CRC OK. No address checkof received packages.
    radio.writeRegister(CC1101_PKTCTRL0, 0x05); // Data whitening off Normal mode, use FIFOs for RX and TX CRC calculation in TX and CRC check in RX enabled Variable packet length mode. Packet length configured by the first byte after sync word
    radio.writeRegister(CC1101_ADDR, 0x00); // Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).

    static uint8_t paTable[] = {0xC6, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
    radio.writeBurstRegister(CC1101_PATABLE, paTable, sizeof(paTable));

    radio.strobe(CC1101_SIDLE); 
    radio.strobe(CC1101_SPWD); 

    Serial.println("Radio OK");
    radio.setRXstate();             // Set the current state to RX : listening for RF packets
    
    // LED setup - so we can use the module without serial terminal
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Setup Finished");
  }


void loop(void) {
    // Turn on the LED for 200ms without blocking the loop.
    digitalWrite(LED_BUILTIN, millis() - ledTimer > 200);

    // Receive part. if GDO0 is connected with D1 you can use it to detect incoming packets
    //if (digitalRead(D1)) {
    byte pkt_size = radio.getPacket(packet);
    if (pkt_size > 0 && radio.crcok()) { // We have a valid packet with some data
        short heating;
        long p1, p2;
        char pbuf[32];
        byte boostTime;
        bool waterHeating,cylinderHot;

        Serial.print("Frame: ");
        rxTimer = millis();
        rxLQI = radio.getLQI();
        for (int i = 0; i < pkt_size; i++) {
            sprintf(pbuf, "%02x", packet[i]);
            Serial.print(pbuf); //packet[i], HEX);
            Serial.print(",");
        }

        Serial.print("len=");
        Serial.print(pkt_size);
        Serial.print(" RSSI="); // for field tests to check the signal strength
        Serial.print(radio.getRSSIdbm());
        Serial.print(" LQI="); // for field tests to check the signal quality
        Serial.println(rxLQI);

        //   buddy request                            sender packet
        if ((packet[2] == 0x21 && pkt_size == 29) || (packet[2] == 0x01 && pkt_size == 44)) {
            if(rxLQI < addressLQI) { // is the signal stronger than the previous/none
                addressLQI = rxLQI;
                address[0] = packet[0]; // save the address of the packet	0x1c7b; //
                address[1] = packet[1];
                addressValid = true;
                Serial.print("Updated the address to:");
                sprintf(pbuf, "%02x,%02x",address[0],address[1]);
                Serial.println(pbuf);
            }		
        }

        // main unit (sending info to iBoost Buddy)
        if (packet[2] == 0x22) {    
            heating = (* ( short *) &packet[16]);
            p1 = (* ( long*) &packet[18]);
            p2 = (* ( long*) &packet[25]); // this depends on the request

            if (packet[6])
                waterHeating = false;
            else
                waterHeating = true;

            if (packet[7])
                cylinderHot = true;
            else
                cylinderHot = false;

            boostTime=packet[5]; // boost time remaining (minutes)
            Serial.print("Heating=");
            Serial.print(heating );
            Serial.print(",P1=");
            Serial.print(p1 );
            Serial.print(",Import=");
            Serial.print(p1 / 390 );        // was 360
            Serial.print(",P2=");
            Serial.print(p2 );
            Serial.print(",P3=");
            Serial.print((* (signed long*) &packet[29]) );
            Serial.print(",P4=");
            Serial.println((* (signed long*) &packet[30]) );
            
            // convert p2 to kWh
            // if (p2 > 0)
            //     p2 = p2/1000;

            switch (packet[24]) {
                case   SAVED_TODAY:
                    today = p2;
                    break;
                case   SAVED_YESTERDAY:
                    yesterday = p2;
                    break;
                case   SAVED_LAST_7:
                    last7 = p2;
                    break;
                case   SAVED_LAST_28:
                    last28 = p2;
                    break;
                case   SAVED_TOTAL:
                    total = p2;
                    break;
            }

            if (cylinderHot)
                Serial.println("Water Tank HOT");
            else if (boostTime > 0)
                Serial.println("Manual Boost ON");
            else if (waterHeating) {
                Serial.print("Heating by Solar=");
                Serial.println(heating);
            }
            else
                Serial.println("Water Heating OFF");

            if (packet[12] == 0x01)
                Serial.println("Warning: Sender Battery LOW");
            else
                Serial.println("Sender Battery OK");

            Serial.print("Today:");
            Serial.print(today);
            Serial.print(" Wh   Yesterday:");
            Serial.print(yesterday);
            Serial.print(" Wh   Last 7 Days:");
            Serial.print(last7);
            Serial.print(" Wh   Last 28 Days:");
            Serial.print(last28);
            Serial.print(" Wh   Total:");
            Serial.print(total);
            Serial.print(" Wh   Boost Time:");
            Serial.println(boostTime);
        }
        // Update LED timer
        ledTimer = millis();
    }

    if(addressValid) {
        if ((millis() - pingTimer > 10000) || boostRequest) { // ping every 10sec
            if ( (millis() - rxTimer) > 1000 &&  (millis() - rxTimer) < 2000) {
                memset(txBuf, 0, sizeof(txBuf));

                if ((request < 0xca) || (request > 0xce)) 
                    request = 0xca;

                txBuf[0] = address[0]; //payload
                txBuf[1] = address[1];		  
                txBuf[2] = 0x21;
                txBuf[3] = 0x8;
                txBuf[4] = 0x92;
                txBuf[5] = 0x7;
                txBuf[8] = 0x24;
                txBuf[10] = 0xa0;
                txBuf[11] = 0xa0;
                txBuf[12] = request; // request information (on this topic) from the main unit
                txBuf[14] = 0xa0;
                txBuf[15] = 0xa0;
                txBuf[16] = 0xc8;
                
            //   if(boostRequest){
            //     txBuf[4] = 0x18; // set boost time
            //     txBuf[18] = boostTime;
            //     boostRequest = false;
            //   }

                radio.strobe(CC1101_SIDLE);
                radio.writeRegister(CC1101_TXFIFO, 0x1d);             // packet length
                radio.writeBurstRegister(CC1101_TXFIFO, txBuf, 29);   // write the data to the TX FIFO
                radio.strobe(CC1101_STX);
                delay(5);
                radio.strobe(CC1101_SWOR);
                delay(5);
                radio.strobe(CC1101_SFRX);
                radio.strobe(CC1101_SIDLE);
                radio.strobe(CC1101_SRX);
                Serial.print("Sent request: ");
                switch (request) {
                    case   SAVED_TODAY:
                        Serial.println("Saved Today");
                        break;
                    case   SAVED_YESTERDAY:
                        Serial.println("Saved Yesterday");
                        break;
                    case   SAVED_LAST_7:
                        Serial.println("Saved Last 7 Days");
                        break;
                    case   SAVED_LAST_28:
                        Serial.println("Saved Last 28 Days");
                        break;
                    case   SAVED_TOTAL:
                        Serial.println("Saved In Total");
                        break;
                }

                request++;

                // Update timer for pinging main unit for information
                pingTimer = millis();
            }
        }
    }
}
