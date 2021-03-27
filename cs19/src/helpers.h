#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>
#include "main.cpp"
#include "Scale.h"

// A Scale object instance on Pin 25 and 27
extern Scale scale(25,27);
extern unsigned long timer = millis();  //initial start time

/**
 * @brief Get the current Firmware Version
 * 
 * @return String 
 */
String getVersion() {
  // String versionNum = String(FW_VERSION / 1000) + "." + String(FW_VERSION % 1000 / 100) + "." + String(FW_VERSION % 100);
  String versionNum = String(VERSION.major) + "." + String(VERSION.minor) + "." + String(VERSION.patch);
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

String remoteDisplay(Scale scale, char *mode) {
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

#endif
