/***
 * ESP32 bindicator using an ESP32-C3.  
 * 
 * Original concept/project written by: Darren Tarbard
 * https://www.youtube.com/channel/UC44BUDSGzAjDnop3DvhAAog
 * 
 * Application was written using VSCode and platformio to manage the project. Upload
 * from VSCode works without any button presses on the board for a change! 
 *
 * ESP32 Information:
 * Internal Total Heap 299656
 * Chip Model ESP32-C3, ChipRevision 4, Cpu Freq 160, SDK Version v4.4.2
 * Flash Size 4194304, Flash Speed 80000000
 * 
 * v1: Initial version 
 * 
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"

// CaptureLog setup
#define CLOG_ENABLE true                        // this must be defined before cLog.h is included 
#include "cLog.h"

#if CLOG_ENABLE
const uint16_t maxEntries = 25;
const uint16_t maxEntryChars = 100;
CLOG_NEW myLog1(maxEntries, maxEntryChars, NO_TRIGGER, NO_WRAP);
#endif

/* Constants, defines and typedefs */
#define PIN_WS2812B 0           // Output pin on ESP32-C3 that controls the LEDs
#define NUM_PIXELS 4            // Number of LEDs (pixels) in the bin

#define DIMMER 12               // hour to set LEDs dimmer
#define BRIGHTER 7              // hour to set LEDs brighter

#define DIM 75                  // dim value for illumination of the LEDs
#define BRIGHT 250              // bright value for illumination of the LEDs

typedef enum {
    BIN_ERROR,
    RECYCLING_GARDEN_FOOD,
    RECYCLING_FOOD,
    WASTE_FOOD
} this_weeks_bins_t;

typedef enum {
    NO_ERROR,
    WIFI_ERROR,
    NODE_ERROR,
    JSON_ERROR
} status_t;

// Hardware timer for updating clock etc.
hw_timer_t *My_timer = NULL;
volatile bool update = false;
static portMUX_TYPE myMux = portMUX_INITIALIZER_UNLOCKED;

/* Function prototypes */
void ntpTime(void);
this_weeks_bins_t getBinColour(void);
this_weeks_bins_t getBinColourFromFile(void);
void illuminateBin(void);
void setBinTypes(void);
void updateBin(void);
int getCurrentHour(void);
void setBrightness(uint8_t brightness);

// timer interrupt routine
void IRAM_ATTR onTimerInterrupt() {
    portENTER_CRITICAL_ISR(&myMux);
    update = true;
    portEXIT_CRITICAL_ISR(&myMux);
}

/* Globals etc. */
WiFiClientSecure wifiClient;
String ipAddress = "0:0:0:0";

this_weeks_bins_t bin_type = BIN_ERROR;
status_t status = NO_ERROR;
uint8_t currentBrightness = 0;

// Set up library to control LEDs
Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

// Define some standard colours for easier reference later
uint32_t redPixel = ws2812b.Color(255, 0, 0);
uint32_t greenPixel = ws2812b.Color(0, 255, 0);
uint32_t bluePixel = ws2812b.Color(0, 0, 255);
uint32_t violetPixel = ws2812b.Color(246, 0, 255);
uint32_t yellowPixel = ws2812b.Color(255, 255, 0);

const uint32_t whitePixel = ws2812b.Color(255, 255, 255);
const uint32_t unlitPixel = ws2812b.Color(0, 0, 0);

/**
 * @brief Set up the application, wifi, time and LEDs
 * 
 */
void setup() {
    int wifi_connect_counter = 0;
    bool wifi_connected = true;

    #if CLOG_ENABLE
    Serial.begin(115200);
    delay(5000); // delay for serial to begin, ESP32 can be slow to start serial output!	
    #endif
 
    // Show 4 lights which will go out as everything starts up correctly.
    
    // Set brightness using our own routine
    setBrightness(BRIGHT);

    // Set up LEDs
    ws2812b.begin();                // Initialise WS2812B strip object (REQUIRED)
   // ws2812b.setBrightness(BRIGHT);  
    ws2812b.show();                 // Initialise all pixels to off

    ws2812b.clear();
    // turn on all pixels, these will be extinguished if there are no 
    // errors as we set up and get bin types.
    for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {          // for each pixel
        ws2812b.setPixelColor(pixel, violetPixel);              // it only takes effect when pixels.show() is called
    }
    ws2812b.show();  // update to the WS2812B Led Strip

    #if CLOG_ENABLE
    Serial.println(F("\n##################################"));
    Serial.println(F("ESP32 Information:"));
    Serial.printf("Internal Total Heap %d, Internal Used Heap %d, Internal Free Heap %d\n", ESP.getHeapSize(), ESP.getHeapSize()-ESP.getFreeHeap(), ESP.getFreeHeap());
    Serial.printf("Sketch Size %d, Free Sketch Space %d\n", ESP.getSketchSize(), ESP.getFreeSketchSpace());
    Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
    Serial.printf("Chip Model %s, ChipRevision %d, Cpu Freq %d, SDK Version %s\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    Serial.printf("Flash Size %d, Flash Speed %d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    Serial.println(F("##################################\n\n"));
    #endif

    // Set up wifi
    WiFi.disconnect();
    WiFi.mode(WIFI_STA); // switch off AP
    WiFi.setAutoReconnect(true);

    WiFi.begin(SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        wifi_connect_counter++;

        // Give the wifi a few seconds to connect
        if (wifi_connect_counter > 30) {
            wifi_connected = false;
            break;
        }
    }
    
    if (wifi_connected) {
        // Turn off first error light
        ws2812b.setPixelColor(0, unlitPixel);                
        ws2812b.show();  // update to the WS2812B Led Strip

        ipAddress = WiFi.localIP().toString();
        CLOG(myLog1.add(), "IP Address: %s", ipAddress);

        ntpTime();

        // Turn off next eror light
        ws2812b.setPixelColor(1, unlitPixel);                
        ws2812b.show();  // update to the WS2812B Led Strip

        CLOG(myLog1.add(), "WiFi, time and LED setup complete...");

        updateBin();
        
        // Turn off wifi to save power
        // WiFi.disconnect();
        // WiFi.mode(WIFI_OFF);
    } else {
        CLOG(myLog1.add(), "Unable to connect to wifi!");
        status = WIFI_ERROR;
    }

    // set brightness depending on time, default is bright at startup
    int currentHour = getCurrentHour();
    if ((currentHour >= DIMMER) && (currentHour < BRIGHTER)) {
        setBrightness(DIM);
    }

    illuminateBin();

    #if CLOG_ENABLE
    Serial.println(F(""));
    Serial.println(F("## CLOG output ##"));
    for (uint16_t i = 0; i < myLog1.numEntries; i++) {
        Serial.println(myLog1.get(i));
    }
    Serial.println(F("\n"));
    //delay(3000);  // serial output seems to be slow!
    #endif

    // Set up hardware timer
    //int one_second = 1000000;
    long one_minute = 1000000 * 60;
    long minutes = one_minute * 5;   // 60=1hr 
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimerInterrupt, true);
    timerAlarmWrite(My_timer, minutes, true);           // every 'n' minutes
    timerAlarmEnable(My_timer);
}

/**
 * @brief Main loop
 * 
 */
void loop(void) {
    // timer for next bin check
    if (update) {
        // reset update flag
        portENTER_CRITICAL(&myMux);
        update = false;
        portEXIT_CRITICAL(&myMux);

        // TESTING ONLY //
        ws2812b.clear();
        ws2812b.setPixelColor(0, yellowPixel);
        ws2812b.setPixelColor(1, yellowPixel);
        ws2812b.setPixelColor(2, yellowPixel);
        ws2812b.setPixelColor(3, yellowPixel);
        ws2812b.show();
        delay(5000);
        // END OF TESTING //

        int currentHour = getCurrentHour();
        // 00:00 to 00:59, update bin - as we are checking every hour this 
        // should only be called once!
        if (currentHour == 0) {   
            //check bin colours and update;
            ntpTime();
            updateBin();
            illuminateBin();
        } else if ( (currentHour == DIMMER) && (currentBrightness != DIM) ) {
            // dim the brightness of the LEDs if required
            setBrightness(DIM);
            illuminateBin();
        } else if ( (currentHour == BRIGHTER) && (currentBrightness != BRIGHTER) ) {
            // increase the brightness of the LEDs if required
            setBrightness(BRIGHT);
            illuminateBin();
        } else {        // TESTING ONLY //
            illuminateBin();
        } // END OF TESTING

        #if CLOG_ENABLE
        for (uint16_t i = 0; i < myLog1.numEntries; i++) {
            Serial.println(myLog1.get(i));
        }
        Serial.println("");
        #endif
    }
}

/**
 * @brief Get the bin type from node.js app or via file from website
 * and update the bin LEDs.
 * 
 */
void updateBin(void) {
    CLOG(myLog1.add(), "updateBin()");

    bin_type = BIN_ERROR;   // reset bin error flag
    if (WiFi.status() == WL_CONNECTED) {
        bin_type = getBinColour();
        if (BIN_ERROR == bin_type) {
            status = NODE_ERROR;
            bin_type = getBinColourFromFile();

            if (BIN_ERROR == bin_type) {
                status = JSON_ERROR;
            } 
        }
    } else {
        status = WIFI_ERROR;
    }
}

/**
 * @brief Connect to NTP time server for accurate time.
 * 
 */
void ntpTime(void) {
    configTime(0, 0, SNTP_TIME_SERVER);

    // Set timezone - London for us
    setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);
    tzset();
}

/**
 * @brief Set the bin types for testing.
 * 
 */
void setBinTypes(void) {
    if (RECYCLING_GARDEN_FOOD == bin_type) {
        bin_type = RECYCLING_FOOD;
    } else if (RECYCLING_FOOD == bin_type) {
        bin_type = WASTE_FOOD;
    } else {
        bin_type = RECYCLING_GARDEN_FOOD;
    }
}

/**
 * @brief Get the colour of the bin for current time/day.  This will be achieved by accessing a node.js 
 * server on the website where all the heavy lifting will be carried out. It will return a JSON object 
 * containing the bin types.
 * 
 * The server gets todays date and then loops through a .csv file to get the bins for this week.
 * 
 * @return THIS_WEEKS_BINS Bin types
 */
this_weeks_bins_t getBinColour(void)
{
    this_weeks_bins_t retcode = BIN_ERROR;

    CLOG(myLog1.add(), "getBinColour()");

    // Connect to the server and get current bin information
    HTTPClient http;
    
    if (! http.begin(BINDICATOR_URL)) {
        CLOG(myLog1.add(), "  Connection failed to: %s", BINDICATOR_URL);

        // If we cant connect, show 1 pixel red to indicate an error
        //pixels.setPixelColor(0, redPixel);
    } else {
        CLOG(myLog1.add(), "  Connection succesful to: %s", BINDICATOR_URL);

        // If we can connect get bin type
        int httpCode = http.GET();
        
        // Check we're getting a HTTP Code 200
        if (HTTP_CODE_OK == httpCode) {
            String payload = http.getString();
            CLOG(myLog1.add(), "  HTTP.get() HTTP_CODE_OK");
            
            // Parse JSON object
            DynamicJsonDocument doc(256);
            DeserializationError err = deserializeJson(doc, payload);
            if (err) {
                CLOG(myLog1.add(), "  DeserializeJson failed: %s", err.c_str());
            } else {
                String bins = doc["collection"].as<String>();

                CLOG(myLog1.add(), "  Bin type: %s", bins);
                if (bins == "rgf") {
                    retcode = RECYCLING_GARDEN_FOOD;
                } else if (bins == "rf") {
                    retcode = RECYCLING_FOOD;
                } else if (bins == "wf") {
                    retcode = WASTE_FOOD;
                } else {
                    retcode = BIN_ERROR;
                }
            }
        } else {
            CLOG(myLog1.add(), "  Unable to get bin type!");
        }
    }

    return retcode;
}

/**
 * @brief Get the colour of the bin for current time/day.  This is an alternative function to above which 
 * will download a .json file containing dates and their associated bin types.  We then loop through the 
 * JSON to find todays date and then extract the bin types for that day/week.
 * 
 * This is a backup function in case the node.js server on the website failes or we move website host and 
 * can't run a node.js program on it.
 *  
 * @return THIS_WEEKS_BINS Bin types
 */
this_weeks_bins_t getBinColourFromFile(void) {
    this_weeks_bins_t retcode = BIN_ERROR;

    CLOG(myLog1.add(), "getBinColourFromFile()");
   
    // Connect to the server and get current bin information
    HTTPClient http;
    
    if (! http.begin(BINDICATOR_FILE_URL)) {
        CLOG(myLog1.add(), "  Connection failed to: %s", BINDICATOR_FILE_URL);
    } else {
        CLOG(myLog1.add(), "  Connection successful to: %s", BINDICATOR_FILE_URL);

        // If we can connect get bin type
        int httpCode = http.GET();
        
        // Check we're getting a HTTP Code 200
        if (HTTP_CODE_OK == httpCode) {
            String payload = http.getString();
            CLOG(myLog1.add(), "  HTTP.get() HTTP_CODE_OK");

            //Serial.println("HTTP.get()...");
            //Serial.println("  " + payload);

            // Parse JSON object
            DynamicJsonDocument doc(25 * 1024);
            DeserializationError err = deserializeJson(doc, payload);
            String bin_type = "error";
            if (err) {
                CLOG(myLog1.add(), "  DeserializeJson failed: %s", err.c_str());
            } else {
                struct tm timeinfo;
                if (!getLocalTime(&timeinfo)) {
                    CLOG(myLog1.add(), "  Failed to obtain time");
                } else {
                    char todaysDate[11]; 
                    strftime(todaysDate, sizeof(todaysDate), "%Y-%m-%d", &timeinfo); // yyyy-mm-dd
                    String bin_type = "";

                    int i = 0;
                    // loop through the deserialized data and find the bins for todays date
                    for (JsonObject item : doc.as<JsonArray>()) {
                        //const char* date = item["date"]; // "2023-01-01", "2023-01-02", "2023-01-03", "2023-01-04", ...
                        //const char* bins = item["bins"]; // "wf", "wf", "wf", "wf", "wf", "wf", "rgf", "rgf", "rgf", "rgf", ...
                        
                        if (strcmp(item["date"], todaysDate) == 0) {
                            bin_type = item["bins"].as<String>();
                            // we have the bins, exit for loop
                            break;
                        }
                    }

                    // now set return code for the type of bins found
                    if (bin_type == "rgf") {
                        retcode = RECYCLING_GARDEN_FOOD;
                    } else if (bin_type =="rf") {
                        retcode = RECYCLING_FOOD;
                    } else if (bin_type == "wf") {
                        retcode = WASTE_FOOD;
                    } else {
                        retcode = BIN_ERROR;
                    }

                    CLOG(myLog1.add(), "  Bin types for %s are: %s", todaysDate, bin_type);
                }
            }
        } else {
            CLOG(myLog1.add(), "  Unable to get bin type!");
        }
    }

    return retcode;
}

/**
 * @brief Light up the LEDs in the bin :-)
 * 
 */
void illuminateBin(void) {
    CLOG(myLog1.add(), "illuminateBin()");
    ws2812b.clear();

    // Get bin types and show appropriate colours
    switch (bin_type) {
        case RECYCLING_GARDEN_FOOD:
            // set colour
            ws2812b.setPixelColor(0, greenPixel);
            ws2812b.setPixelColor(1, greenPixel);
            ws2812b.setPixelColor(2, bluePixel);
            ws2812b.setPixelColor(3, bluePixel);
            CLOG(myLog1.add(), "  Bin Type: Recycling and Garden");
        break;
        case WASTE_FOOD:
            // set colour
            ws2812b.setPixelColor(0, redPixel);
            ws2812b.setPixelColor(1, redPixel);
            ws2812b.setPixelColor(2, redPixel);
            ws2812b.setPixelColor(3, redPixel);
            CLOG(myLog1.add(), "  Bin Type: Normal Waste");
        break;
        case RECYCLING_FOOD:
            // set colour
            ws2812b.setPixelColor(0, bluePixel);
            ws2812b.setPixelColor(1, bluePixel);
            ws2812b.setPixelColor(2, bluePixel);
            ws2812b.setPixelColor(3, bluePixel);
            CLOG(myLog1.add(), "  Bin Type: Recycling only");
        break;
        default:    // BIN_ERROR
            // set error colour
            if (NODE_ERROR == status) {
                ws2812b.setPixelColor(0, unlitPixel);
                ws2812b.setPixelColor(1, unlitPixel);
                ws2812b.setPixelColor(2, violetPixel);
                ws2812b.setPixelColor(3, violetPixel);
                CLOG(myLog1.add(), "  Status: NODE ERROR!");
            } else if (JSON_ERROR == status) {
                ws2812b.setPixelColor(0, unlitPixel);
                ws2812b.setPixelColor(1, unlitPixel);
                ws2812b.setPixelColor(2, unlitPixel);
                ws2812b.setPixelColor(3, violetPixel);
                CLOG(myLog1.add(), "  Status: JSON ERROR!");
            } else if (WIFI_ERROR == status) {
                ws2812b.setPixelColor(0, violetPixel);
                ws2812b.setPixelColor(1, unlitPixel);
                ws2812b.setPixelColor(2, unlitPixel);
                ws2812b.setPixelColor(3, unlitPixel);
                CLOG(myLog1.add(), "  Status: WIFI ERROR!");
            } else {    // status ok, a bin error!
                ws2812b.setPixelColor(0, violetPixel);
                ws2812b.setPixelColor(1, violetPixel);
                ws2812b.setPixelColor(2, violetPixel);
                ws2812b.setPixelColor(3, violetPixel);
                CLOG(myLog1.add(), "  Status: BIN ERROR!");
            }

            // Reset status for next checkup
            status = NO_ERROR;
        break;
    }

    ws2812b.show();  // update to the WS2812B Led Strip   
}

/**
 * @brief Get the current hour.
 * 
 * @return int Current Hour
 */
int getCurrentHour(void) {
    int retcode = -1;

    CLOG(myLog1.add(), "getCurrentHour()");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        CLOG(myLog1.add(), "  Failed to obtain timeinfo");
    } else {
        retcode = timeinfo.tm_hour;
        CLOG(myLog1.add(), "  Current hour is: %d", retcode);
    }

    return retcode;
}

/**
 * @brief Set the Brightness of all the defined pixel colours. Can not 
 * use the neopixel library for this, 'setBrightness() function as 
 * currently implemented operates directly on the pixel array and is 'lossy'. 
 * Since the original brightness value is not retained, once dimmed, you can 
 * never return to the exact original brightness value. The more you dim, the 
 * more information you lose'.
 * 
 * @param unit8_t brightness to set the LED colours we're using.
 */
void setBrightness(uint8_t brightness) {
    CLOG(myLog1.add(), "setBrightness(%d), current brightness is: %d", brightness, currentBrightness);

    if (brightness != currentBrightness) {
        redPixel = ws2812b.Color(brightness*255/255, brightness*0/255, brightness*0/255);
        greenPixel = ws2812b.Color(brightness*0/255, brightness*255/255, brightness*0/255);
        bluePixel = ws2812b.Color(brightness*0/255, brightness*0/255, brightness*255/255);
        violetPixel = ws2812b.Color(brightness*246/255, brightness*0/255, brightness*255/255);
        yellowPixel = ws2812b.Color(brightness*255/255, brightness*255/255, brightness*0/255);
        currentBrightness = brightness;
    }
}