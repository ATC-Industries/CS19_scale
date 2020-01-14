#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <EEPROM.h>           // include library to read and write from flash memory


#include "Scale.h"
#include "Index.h"
#include "Updater.h"


#define EEPROM_SIZE 64         // define the number of bytes you want to access in flash memory

// Set wifi login and password
const char* ssid     = "ProTournamentScales";
String password = "987654321";
const int passAddress = 0;
unsigned long timer = millis();

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(0, 0, 0, 0);

IPAddress subnet(255, 255, 255, 0);


//  Firmware Updates
//  1001 - Initial Design
//  1002 -
//  1003 - 
//  1004 - Add all styles locally via data upload.
//  1005 - Add legacy remote checker, blink LED red for no Legacy, RGB for Xbee found
const int FW_VERSION = 1005;
int remoteMode = 0;
String maxMode = "4";

// A Scale object instance on Pin 25 and 27
Scale scale(25,27);
// Instantiate Updater
//Updater updater;

// Get the current Firmware Version
String getVersion() {
  return String(FW_VERSION);
}

String processStringForRemote(String weight, String oz) {
  String s = "";
  //Serial.println(s);
  //Serial.println(scale.getUnits());
  if (scale.getUnits() == "") {     // if empty string returned in lbs oz mode
    int tempLb = weight.toInt();         // get lbs from scale and convert to int
    float tempOz = oz.toFloat();     // convert oz to a float
    if (tempLb <= 0 && tempOz < 0.1) {          // check if less than 0lb 0.1oz
      s = " 0. 0";                              // if it is print 0. 0
    } else {                                    // if not then print real weight    
      if (tempLb < 10) {
        s = " ";
      } else {
        s = "";
      }
      s += tempLb;                               // make it the String
      if (tempOz < 10.0 ){                      // if oz is less than 10
        s += ". ";                              // we need to add a space between lb and oz
      } else {                                  // if it's greater than 10
        s += ".";                               // no space needed
      }
      s += tempOz;                              // no append the ounces  Remote display should cut off decimal if needed.
    }
  }
  else { 
    s = weight;                                      // must be in Kg or Lb mode 
    int decimalCounter = 0;
    s.replace(" ", "");                         // delete all the spaces
    // loop through weight to see if there are no decimals
    for (int i = 0; i < s.length(); i++) {
      if (s[i] == '.') {
        decimalCounter++;
      } 
    }
    if (decimalCounter > 0) {
      if (s.toFloat() < 0.1) {                    // check if value is less than .01
        s = " 0.00";                              //  if it is just display zeros
      }
    } else {
      if (s.toInt() < 5) {
        s = "   0";
      }
    }
  }                                             // otherwise business as usual

 // Serial.println(s);
  return s;
}

String remoteDisplay(char *mode) {
  switch (*mode)
  {
  case '1':
    /* Mode 1 is Normal Weiging Mode */
    return processStringForRemote(scale.getWeight(), scale.getOz());
    break;
  case '2':
    /* Mode 2 will hold weight until new weight is locked */
    return processStringForRemote(scale.getLastLocked(), scale.getLockedOz());
    break;
  case '3':
    /* Mode 3 will 'biggest loser' style randomization */
    if (scale.getLockStatus() == "Calculating...") {
      float r = (rand() / (float)RAND_MAX * (20));
      return processStringForRemote(String(r), String(rand() % 15));
    } else {
      return processStringForRemote(scale.getWeight(), scale.getOz());
    }
    break;
  case '4':
    /* Mode 4 will display 0.00 when scale is 0, ---- when calculating and the locked weight when locked */
    if (scale.getLockStatus() == "Calculating...") {
      return "----";
    } else {
      return processStringForRemote(scale.getWeight(), scale.getOz());
    }
    break;
  default:
    /* Default back to normal weighing as default catch all */
    return processStringForRemote(scale.getWeight(), scale.getOz());
    break;
  }
}

// Replaces placeholder in HTML with values
String processor(const String& var) {
  //Serial.println(var);
  if(var == "WEIGHT"){
    return scale.getWeight();
  }
  else if(var == "LOCKED"){
    return scale.getLockStatus();
  }
  else if(var == "VERSION"){
    return getVersion();
  }
  else if(var == "UNIT"){
    return scale.getUnits();
  }
  else if (var == "LASTLOCKED") {
    return scale.getLastLocked();
  }
  return String();
}

void setup() {
  
  // TODO uint8_t cardType;
  Serial.begin(115200);                      // start serial port 0 (debug monitor and programming port)
  scale.begin();
  Serial.println("Searching for Update...");
  // //first init and check SD card
  //  if (!SD.begin()) {
  //     updater.rebootEspWithReason("Card Mount Failed");
  //  }

  //  cardType = SD.cardType();

  //  if (cardType == CARD_NONE) {
  //     updater.rebootEspWithReason("No SD card attached");
  //  }else{
  //     updater.updateFromFS(SD);
  // }


  // Configures static IP address
  Serial.println("Configuring access point...");
  EEPROM.begin(EEPROM_SIZE);
  
  if (EEPROM.readByte(passAddress) == 255) {
    EEPROM.writeString(passAddress, password);
  }
  password = EEPROM.readString(passAddress);
 

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password.c_str());
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  
  Serial.println("Set softAPConfig");

  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  // Begin SPIFFS
  if(!SPIFFS.begin()) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    Serial.println("Index Requested");
  });
  server.on("/weight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getWeight().c_str());
    Serial.println(scale.getWeight());
  });

  server.on("/getlegacyweight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLegacyWeight().c_str());
    //Serial.println(scale.getLegacyWeight());
  });

  
  server.on("/getmaxmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", maxMode.c_str());
    Serial.println("maxMode");
  });

  server.on(
    "/remote",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      char mode[len];
      for (size_t i = 0; i < len; i++) {
        //Serial.write(data[i]);
        mode[i] = data[i];
      }
  
      //Serial.println();
 
      request->send(200, "text/plain", remoteDisplay(mode).c_str());
  });

server.on(
    "/remoteMode",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      //char mac[len];
      for (size_t i = 0; i < len; i++) {
        Serial.write(data[i]);
      }
 
      Serial.println();
      request->send_P(200, "text/html", index_html, processor);
      //request->send(200, "text/plain", remoteDisplay(mac).c_str());
  });


  server.on("/getUnits", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getUnits().c_str());
    //Serial.println("Weight Sent");
  });
  server.on("/isLocked", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLockStatus().c_str());
    //Serial.println("locked status Sent");
  });
  server.on("/getLastLocked", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLastLocked().c_str());
    //Serial.println("last locked value Sent");
  });
  // Route to load style.css file
server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
});

// Route to load bootstrap.css file
server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/bootstrap.min.css", "text/css");
});

// Route to load logo file
server.on("/pts.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/pts.png", "image/png");
});

// Route to load logo file
server.on("/pts.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/pts.svg", "image/svg+xml");
});
server.on("/pts-white.svg", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/pts-white.svg", "image/svg+xml");
});

// Route to load favicon.ico file
server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.ico", "image/x-icon");
});

// Route to load jQuery js file
server.on("/jquery-3.3.1.slim.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/jquery-3.3.1.slim.min.js", "application/javascript");
});

// Route to load popper js file
server.on("/popper.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/popper.min.js", "application/javascript");
});

// Route to load bootstrap.js file
server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/bootstrap.min.js", "application/javascript");
});

// Route to load jquery js file
server.on("/jquery-slim.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/jquery-slim.min.js", "application/javascript");
});

server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404);
});


  // Connect to Wi-Fi
  server.begin();               //start server
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (millis() - timer > 300) {
  //   scale.readScale();
  //   timer = millis();
    
  // }
  scale.readScale();
}