#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include "Scale.h"
#include "Index.h"
#include "Updater.h"

//  ---- FIRMWARE UPDATES ====  //
//  1001 - Initial Design
//  1002 -
//  1003 - 
//  1004 - Add all styles locally via data upload.
//  1005 - Add legacy remote checker, blink LED red for no Legacy, RGB for Xbee found
//  1006 - SHIP - Bobo Albright 
//  1007 - Add auto print or manual print option, set on printer
//  1008 - Change RX TX to Xbee to 19/21
//  1009 - Added units button startup timeout.
//  1010 - fix weight off by one error / increased max connections from 4 to 10
//  1011 - Outputs 0.000 to remote display instead of 0.00 when in Kg mode.
//  1012 - Add startup check for stylesheet and flash purple led
//  1013 - Fix Units button startup problems
const int FW_VERSION = 1013;


// Set wifi login and password
const char* ssid     = "ProTournamentScales";
String password = "987654321";
const int passAddress = 0;
unsigned long timer = millis();

// Port 80 where the server will be listening
AsyncWebServer server(80);
// A websocket input called /ws
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address  0.0.0.0 means no router, this will allow mobile phones to still connect to internet using data.
IPAddress gateway(0, 0, 0, 0);

IPAddress subnet(255, 255, 255, 0);

// Number of modes available.  requesteted by remote display to calculate mode switching
String maxMode = "4";

// A Scale object instance on Pin 25 and 27
Scale scale(25,27);

// Get the current Firmware Version
String getVersion() {
  String versionNum = String(FW_VERSION / 1000) + "." + String(FW_VERSION % 1000 / 100) + "." + String(FW_VERSION % 100);
  return versionNum;
}

String processStringForRemote(String weight, String oz) {
  String s = "";
  if (scale.getUnits() == "") {                 // if empty string returned in lbs oz mode
    int tempLb = weight.toInt();                // get lbs from scale and convert to int
    float tempOz = oz.toFloat();                // convert oz to a float
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
      s += tempOz;                              // now append the ounces  Remote display should cut off decimal if needed.
    }
  }
  else if (scale.getUnits() == "kg") {
      s = weight;                                // must be in Kg mode 
    int decimalCounter = 0;
    s.replace(" ", "");                         // delete all the spaces
    // loop through weight to see if there are no decimals
    for (int i = 0; i < s.length(); i++) {
      if (s[i] == '.') {
        decimalCounter++;
      } 
    }
    if (decimalCounter > 0) {
      if (s.toFloat() < 0.050) {                    // check if value is less than .01
        s = "0.000";                              //  if it is just display zeros
      }
    } else {
      if (s.toInt() < 5) {
        s = "   0";
      }
    }
    //s = String(s.toFloat());
  }
  else { 
    s = weight;                                      // must be in Lb mode 
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
  else if (var == "LOCKODO") {
    return scale.getLockOdo();
  }
  return String();
}

void setup() {





  // TODO uint8_t cardType;
  Serial.begin(115200);                      // start serial port 0 (debug monitor and programming port)
  Serial.println("Booting Up...");
  Serial.print("Software Version: ");
  Serial.println(FW_VERSION);
  scale.begin();
  //Serial.println("Searching for Update...");
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
  // Setup wifi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password.c_str(),1,0,10);
  Serial.println("Wait 500 ms for AP_START...");
  delay(500);
  
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
  server.on("/islockedandprintpressed", HTTP_GET, [](AsyncWebServerRequest *request){
    scale.isAuto = false;
    request->send(200, "text/plain", scale.getPrintButtonStatus().c_str());
    scale.changePrintStatus(false); 
    // Serial.println(scale.getWeight());
  });
  server.on("/islockedandauto", HTTP_GET, [](AsyncWebServerRequest *request){
    scale.isAuto = true;
    request->send(200, "text/plain", scale.getPrintButtonStatus().c_str());
    scale.changePrintStatus(false); 
    // Serial.println(scale.getWeight());
  });


  // xBee Legacy Radio makes a call to get old style legacy weight
  server.on("/getlegacyweight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLegacyWeight().c_str());
    //Serial.println(scale.getLegacyWeight());
  });

  // remote display makes a call to discover the max different display modes available
  server.on("/getmaxmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", maxMode.c_str());
    Serial.println("maxMode");
  });

  // Remote display server request
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
      Serial.println(remoteDisplay(mode).c_str());
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
    request->send(200, "text/plain", scale.getLastLocked().c_str());
    //Serial.println("last locked value Sent");
  });
  server.on("/getLockedOdo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", scale.getLockOdo().c_str());
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


server.begin();               //start server

}

void loop() {
  scale.readScale();
}