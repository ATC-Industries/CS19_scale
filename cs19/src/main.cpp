

#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "Scale.h"
#include "webfiles/Index.h"
#include "webfiles/bootstrap.min.css.h"
#include "webfiles/bootstrap.min.js.h"
#include "webfiles/jquery-3.3.1.slim.min.js.h"
#include "webfiles/jquery-slim.min.js.h"
#include "webfiles/popper.min.js.h"


//  ---- FIRMWARE UPDATES ====  //
// See CHANGELOG.md
struct Version{
  int major = 1;
  int minor = 1;
  int patch = 1;
} VERSION;

// A Scale object instance on Pin 25 and 27
Scale scale(25,27);
// unsigned long timer = millis();  //initial start time

String getVersion();
String processStringForRemote(String weight, String oz);
String remoteDisplay(char *mode);
String processor(const String& var);

// Set wifi login and password
const char* ssid  = "ProTournamentScales";
String password   = "987654321";

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


void setup() {

  Serial.begin(115200);                      // start serial port 0 (debug monitor and programming port)
  Serial.println("Booting Up...");
  Serial.print("Software Version: ");
  Serial.println(getVersion());
  scale.begin();

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

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // if (millis() - timer > 500) {
    //       request->send_P(200, "text/html", index_html, processor);
    //       timer = millis();
    // }
    request->send_P(200, "text/html", index_html, processor);
    //Serial.println("Index Requested");
  });

  server.on("/weight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getWeight().c_str());
    //Serial.println(scale.getWeight());
    //Serial.println("/weight");

  });
  server.on("/islockedandprintpressed", HTTP_GET, [](AsyncWebServerRequest *request){
    scale.isAuto = false;
    request->send(200, "text/plain", scale.getPrintButtonStatus().c_str());
    scale.changePrintStatus(false); 
    //Serial.println("/islockedandprintpressed");
    // Serial.println(scale.getWeight());
  });
  server.on("/islockedandauto", HTTP_GET, [](AsyncWebServerRequest *request){
    scale.isAuto = true;
    request->send(200, "text/plain", scale.getPrintButtonStatus().c_str());
    scale.changePrintStatus(false); 
    //Serial.println("/islockedandauto");

    // Serial.println(scale.getWeight());
  });


  // xBee Legacy Radio makes a call to get old style legacy weight
  server.on("/getlegacyweight", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLegacyWeight().c_str());
    //Serial.println("/getlegacyweight");
    //Serial.println(scale.getLegacyWeight());
  });

  // remote display makes a call to discover the max different display modes available
  server.on("/getmaxmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", maxMode.c_str());
    //Serial.println("/getmaxmode");
    //Serial.println("maxMode");
  });

  // Remote display server request
  server.on(
    "/remote",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      Serial.print("/remote - REQUEST MADE - Mode: ");
      char mode[len];
      for (size_t i = 0; i < len; i++) {
        //Serial.write(data[i]);
        mode[i] = data[i];
      }
      Serial.println(mode);
  
      //Serial.println();
 
      request->send(200, "text/plain", remoteDisplay(mode).c_str());
      Serial.print("/remote - RESPONSE SENT: ");
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
        //Serial.write(data[i]);
      }
      //Serial.println();
      request->send_P(200, "text/html", index_html, processor);
      //Serial.println("/remoteMode");
      //request->send(200, "text/plain", remoteDisplay(mac).c_str());
  });

  server.on("/v1/getJSON", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", scale.getJSON().c_str());
    Serial.println(scale.getJSON().c_str());
  });

  server.on("/getUnits", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getUnits().c_str());
    //Serial.println("/getUnits");
    //Serial.println("Weight Sent");
  });
  server.on("/isLocked", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", scale.getLockStatus().c_str());
    //Serial.println("/isLocked");
    //Serial.println("locked status Sent");
  });
  server.on("/getLastLocked", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", scale.getLastLocked().c_str());
    //Serial.println("/getLastLocked");
    //Serial.println("last locked value Sent");
  });

  server.on("/getLockedOdo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", scale.getLockOdo().c_str());
    //Serial.println("/getLockedOdo");
    //Serial.println("last locked value Sent");
  });

  // Route to load bootstrap.css file
  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/css", bootstrap_min_css);
        //Serial.println("/bootstrap.min.css");
  });

  // Route to load jQuery js file
  server.on("/jquery-3.3.1.slim.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "application/javascript", jquery_3_3_1_slim_min_js);
        //Serial.println("/jquery-3.3.1.slim.min.js");
  });

  // Route to load popper js file
  server.on("/popper.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "application/javascript", popper_min_js);
        //Serial.println("/popper.min.js");
  });

  // Route to load bootstrap.js file
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "application/javascript", bootstrap_min_js);
        //Serial.println("/bootstrap.min.js");
  });

  // Route to load jquery js file
  server.on("/jquery-slim.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "application/javascript", jquery_slim_min_js);
        //Serial.println("/jquery-slim.min.js");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404);
  });

  server.begin();               //start server

}

void loop() {
  scale.readScale();
}


String getVersion() {
  return String(VERSION.major) + "." + String(VERSION.minor) + "." + String(VERSION.patch);
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
  else if (var == "LAST1") {
    return scale.getLast1();
  }
  else if (var == "LAST2") {
    return scale.getLast2();
  }
  else if (var == "LAST3") {
    return scale.getLast3();
  }
  else if (var == "LAST4") {
    return scale.getLast4();
  }
  else if (var == "LAST5") {
    return scale.getLast5();
  }
  return String();
}
