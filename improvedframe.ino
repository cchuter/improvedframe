/*
   This program allows users to set their Inkplate up to their home Wifi 
   and then refreshes the inkplate with the image url provided at a regular
   interval that the user also provides by setting it up using a local web page

   Select "Inkplate 10(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 10(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   28 July 2020 by e-radionica.com
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate 10 in the boards menu."
#endif

#include "EEPROM.h"   // Include ESP32 EEPROM library
#include "Inkplate.h"   //Include Inkplate library to the sketch
#include "htmlCode.h"   //Include .h file where we stored out html code of our web page
#include "driver/rtc_io.h" //ESP32 library used for deep sleep and RTC wake up pins
#include <rom/rtc.h>       // Include ESP32 library for RTC (needed for rtc_get_reset_reason() function)

#include <WebServer.h>  //Include ESP32 library for Web server
#include <WiFi.h>       //Include ESP32 WiFi library
#include <WiFiClient.h> //Include ESP32 WiFi library for AP
#include <uri/UriBraces.h>

#define ssid "MyInkplate"
#define pass "myinkplate"
#define uS_TO_S_FACTOR 1000000 // Conversion factor for micro seconds to seconds
#define EEPROM_START_ADDR 76  // Start EEPROM address for user data. Addresses below address 76 are waveform data!
#define EEPROM_SIZE       128

#define SSID_SIZE 32
#define SSIDPASS_SIZE 16
#define URL_SIZE 70
#define REFRESH_SIZE 5

#define eepromSSID EEPROM.readString(EEPROM_START_ADDR)
#define eepromPass EEPROM.readString(EEPROM_START_ADDR+SSID_SIZE+1)
#define eepromURL EEPROM.readString(EEPROM_START_ADDR+SSID_SIZE+SSIDPASS_SIZE+2)
#define eepromRefresh EEPROM.readString(EEPROM_START_ADDR+SSID_SIZE+SSIDPASS_SIZE+URL_SIZE+3)

Inkplate display(INKPLATE_3BIT);
WebServer server(80);            // Create Web server on port 80 (HTTP port number)

IPAddress serverIP;

void setup() {
  Serial.begin(115200);
  display.begin(); // Init Inkplate library (you should call this function ONLY ONCE)

  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) // Check if ESP32 is reseted by deep sleep or power up / user manual
                                                    // reset (or some other reason)
  {  
    renderWebImage();

    Serial.println("Going to sleep");
    delay(1000);

    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * 60 * eepromRefresh.toInt());
    esp_deep_sleep_start();      
  }
  else
  {
    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display

    display.setTextSize(4);
    display.setTextColor(BLACK);
    display.setTextWrap(true);

    // Start WiFi
    WiFi.begin();            // Init. WiFi library
    WiFi.mode(WIFI_AP);      // Set WiFi to Access point mode
    WiFi.softAP(ssid, pass); // Set SSID (WiFi name) and password for Access point

    serverIP = WiFi.softAPIP(); // Get the server IP address

    server.on("/", handleRoot); // If you open homepage, go to handle root function
    server.on(UriBraces("/string/{}"),
              handleString); // If you send some text to Inkplate, go to handleString function. Note that {} brackets at
                            // the end of address. That means that web address has some arguments (our text!).
    server.begin();          // Start the web server

    clearEEPROM();
  
    // Print Instructions    
    updatePaper();
  }
}

void loop() {
  if (rtc_get_reset_reason(0) != DEEPSLEEP_RESET) // Still waiting on settings
  {
    server.handleClient(); // You have to constantly read if there is any new client connected to web server
  }
  delay(100);
}

void updateHTML()
{ // This function will send response to client and send HTML code of our web page
    server.send(200, "text/html", s);
}

void handleRoot()
{ // This function will send response to client if client open a root (homepage) of our web page
    updateHTML();
}

void handleString()
{ 
  // This function will send response to client, send HTML code of web page, get the text from argument sent in web page
  // address and refresh screen with new text

  String homeSSID, homePass, frameURL, refresh;
  homeSSID = server.arg(0);
  homePass = server.arg(1);
  frameURL = server.arg(2);
  refresh = server.arg(3);
  updateHTML();

  // TODO: add a JoinAP check after receiving values

  if (homeSSID != "" && homePass != "" && frameURL != "" && refresh != "") 
  {
    // Write to EEPROM one variable after another with linefeed - never go below the start address
    int address = EEPROM_START_ADDR;
    EEPROM.writeString(address,homeSSID.c_str());
    address += SSID_SIZE+1;
    EEPROM.writeString(address,homePass.c_str());
    address += SSIDPASS_SIZE+1;
    EEPROM.writeString(address,frameURL.c_str());
    address += URL_SIZE+1;
    EEPROM.writeString(address,refresh.c_str());
    EEPROM.commit();
    delay(500); 

    renderWebImage();
    delay(1000);
    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * 60 * eepromRefresh.toInt());
    esp_deep_sleep_start();  
  }
  else
  {
    display.println("Error rreading values submitted...");
    display.println(homeSSID);
    display.println(homePass);
    display.println(frameURL);
    display.println(refresh);
    display.display();
  }

}

void updatePaper()
{                              // This function updates screen with new data (text)
    display.clearDisplay();    // Clear everything from epaper frame buffer

    display.setCursor(0, 40); // Print out instruction on how to connect to Inkplate WiFi and how to open a web page
    display.print("Connect to SSID: ");
    display.println(ssid);
    display.println();
    display.print("With password: ");
    display.println(pass);
    display.println();
    display.print("Open your web browser to ");
    display.print("http://");
    display.print(serverIP);
    display.println('/');
    display.println();

    display.display(); // Send everything to screen (refresh the screen)
}

void renderWebImage()
{
    // Join wifi
    display.joinAP(eepromSSID.c_str(), eepromPass.c_str());
    
    String url = eepromURL;
    HTTPClient http;
    if (http.begin(url.c_str()) && http.GET() > 0)
    {
      Serial.println(url);
    }
    else
    {
      display.println("HTTP error");
      display.display();
    }
    http.end();

    Serial.println(display.drawImage(url, display.PNG, 0, 0));
    display.display();
}

// Function for clearing EEPROM data (it will NOT clear waveform data)
void clearEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i + EEPROM_START_ADDR,
                     0); // Start writing from address 76 (anything below that address number is waveform data!)
    }
    EEPROM.commit();
}
