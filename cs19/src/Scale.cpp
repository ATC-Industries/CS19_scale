/**
 * @file Scale.cpp
 * @author Adam Clarkson (adam@agri-tronix.com)
 * @brief 
 * @version 0.1
 * @date 2019-10-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "Scale.h"

/**
 * @brief Construct a new Scale:: Scale object
 * 
 */
Scale::Scale() {

}

/**
 * @brief Construct a new Scale:: Scale object
 * 
 * @param rx Pin number of RX line
 * @param tx Pin Number of TX Line
 */
Scale::Scale(int rx, int tx){
  RXD2 = rx;
  TXD2 = tx;
}

/**
 * @brief Construct a new Scale:: Scale object
 * 
 * @param rx Pin Number
 * @param tx Pin Number
 * @param lockLed Pin Number
 */
Scale::Scale(int rx, int tx, int lockLed){
  RXD2 = rx;
  TXD2 = tx;
  lockLedRed = lockLed;
}

/**
 * @brief Construct a new Scale:: Scale object
 * 
 * @param rx Pin Number
 * @param tx Pin Number
 * @param lockLed Pin Number
 * @param printPin Pin Number
 */
Scale::Scale(int rx, int tx, int lockLed, int printPin){
  RXD2 = rx;
  TXD2 = tx;
  lockLedRed = lockLed;
  scale_print_button = printPin;
}

Scale::~Scale() {}

/**
 * @brief Begin Serial port communication with Scale
 * @details To be called in setup routine. will initiate all serial communications
 * 
 */
void Scale::begin(){
  isBootUp = true;
  preferences.begin("my-app", false);
  lockedCounter = preferences.getUInt("lockedCounter", 0);
  Serial.println("Total number of Locks: " + String(lockedCounter));
  units = NOTUSED;
  lastUnits = preferences.getUInt("lastUnits", static_cast<int>(units));
  preferences.end();
  // Make sure LEDs are off first thing
  ledRGBStatus(0,0,0);
  pinMode(scale_print_button, INPUT);
  attachInterrupt(digitalPinToInterrupt(scale_print_button),this->Scale::print_pb_isr,CHANGE);     //this calls 'print_pb()' when user presses print button on cs-19
  // RS232 comm with xBee Radio
  Serial1.begin(9600, SERIAL_8N1, 18, 19);
  // RS232 input from CS19
  Serial2.begin(9600, SERIAL_8N1,RXD2,TXD2);
  
  delay(1000);
  Serial1.print("+++");
  Serial.println("\n+++ Sent");
  delay(1500);
  if (Serial1.available()) {
    String response = Serial1.readStringUntil('\n');
    Serial.println(response);
    response.trim();
    if (response.equals("OK")) { 
      Serial.println("xBee Radio Found");
      //  Rainbow the leds at startup
      for (int i = 0; i < 3; i++) {
        ledRGBStatus(1,0,0);
        delay(500);
        ledRGBStatus(0,1,0);
        delay(500);
        ledRGBStatus(0,0,1);
        delay(500);
      }
      ledRGBStatus(0,0,0);
    } else {
      Serial.println("No Legacy Board Found");
      // Blink LOCKED LED at startup
      int x = 6; 
      while (x != 0){
        ledRGBStatus(1,0,0);
        delay(250);
        ledRGBStatus(0,0,0);
        delay(250);
        x = x-1; 
      }
    } 
  } else {
      Serial.println("No Legacy Board Found");
      // Blink LOCKED LED at startup
      int x = 6; 
      while (x != 0){
        ledRGBStatus(1,0,0);
        delay(250);
        ledRGBStatus(0,0,0);
        delay(250);
        x = x-1; 
      }
    }    
}

/**
 * @brief Interupt Service Routine to for Print button
 * 
 */
void Scale::print_pb_isr(){                                          //This is an  isr that is called when CS-19 print button is pressed
  //insert code here to execute when cs19 print button is pressed
  // NOTE this routine will be called for every character coming in off of serial port.
  // need code eliminate duplicates
  Serial.println("Print button pressed");                    //***diagnostic*** to check that esp32 is ack the print button
}

/**
 * @brief read scale RS232 signal
 * 
 */
void Scale::readScale(){
  static int rx2_pointer;                       //pointer for rs 232 port 2 rx string  
  bool process_buffer_flag = 0;              //flag to signal to process rx2 string 
  bool lock_flag = 0;                        //flag for lock condition
  static int decimalCounter;

  //check uart for input from scale
  if (Serial2.available()) {                              
    hasSignalFlag = true;
    rx2_buffer[rx2_pointer]  = Serial2.read();            //read character and place in buffer
    switch (rx2_buffer[rx2_pointer])
    {
    case 0x02:                                            //if beginning of string character reset the pointer
      rx2_buffer[0] = 0x02;                               //set first character to 0x02
      rx2_pointer = 0;                                    //reset the pointer
      decimalCounter = 0;
      
      break;
    case 0x2E:
      decimalCounter++;
      break;
    case 'H':                                             //if there is an 'H' in the string then set flag to turn on lock led
      lock_flag = 1;                                      //set flag so lock light will come on when processing string
      break;
    case 'L':
      units = LB;
      checkPref();
      break;
    case 'K':
      units = KG;
      checkPref();
      break;
    case 'G':
      tareMode = GROSS;
      break;
    case 'N':
      tareMode = NET;
      break;
    case 0x20:
      status = VALID;
      break;
    case 'M':
      status = MOTION;
      break;
    case 'O':
      status = OVERUNDER;
      break;
    case 0x0D:
      process_buffer_flag = 1;                            //set flag so code will process buffer

      break;
    default:
      break;
    }

    // if (rx2_buffer[rx2_pointer] == 0x02) {                
    //   rx2_buffer[0] = 0x02;                               //set first character to 0x02
    //   rx2_pointer = 0;                                    //reset the pointer
    // } else if(rx2_buffer[rx2_pointer] == 'H'){            //if there is an 'H' in the string then set flag to turn on lock led
    //   lock_flag = 1;                                      //set flag so lock light will come on when processing string
    // } else if(rx2_buffer[rx2_pointer] == 0x0D){           //check for end of string
    //   process_buffer_flag = 1;                            //set flag so code will process buffer 
    // }
    if (decimalCounter >= 2) {
      units = LBOZ;
      checkPref();
    }
    if (++ rx2_pointer >= 24){                //increment pointer and check for overflow
      rx2_pointer = 0;                        //reset pointer on buffer overflow
      decimalCounter = 0;
    }
  } else {
    hasSignalFlag = false;
  }
  if (rx2_buffer[11] == 'H') {lock_flag = 1;}
  // fix for bobs software
  if (rx2_buffer[11] == ' ') {rx2_buffer[11] = 'M';}
  if (process_buffer_flag){
    if (lock_flag == 1){
      ledOn(lockLedRed);                                //turn on lock light
      isLocked = true;
    } else {
      ledOff(lockLedRed);                               //turn off lock light
      isLocked = false;
    }
    char legacyRemWeight[30] = "\x02 ";
    //strncpy(legacyRemWeight,"\x02 ",3);
    
    strncpy(legacyRemWeight+2,rx2_buffer+1,13);
    //Serial.println(legacyRemWeight);
    if (legacyRemWeight[2] == '-' || legacyRemWeight[12] == 'O') {
    } else { 
      Serial1.print(legacyRemWeight);
    }
    legRemWeigh = legacyRemWeight;
    // output weight string
    if (units == LB || units == KG) {
      for (int i=0;i<30; i++){weight[i]=' ';}
      
      strncpy(weight,rx2_buffer+1,8);
    } else if (units == LBOZ) {
      for (int i=0;i<30; i++){weight[i]=' ';}
      for (int i=0;i<2; i++){outLb[i]=' ';}
      for (int i=0;i<4; i++){outOz[i]=' ';}

      char outLbOz[12];
      strncpy(outLb,rx2_buffer+3,2);
      strncpy(outOz,rx2_buffer+6,4);
      for(int i=0;i<2;i++ ){
        outLbOz[i] = outLb[i];
      }
      outLbOz[2] = 'l';
      outLbOz[3] = 'b';
      outLbOz[4] = ' ';
      for(int i=0;i<4;i++ ){
        outLbOz[i+5] = outOz[i];
      }
      outLbOz[9] = 'o';
      outLbOz[10] = 'z';
      outLbOz[11]='\0'; // terminate the char*

      strncpy(weight,outLbOz,12);   //create output string from outLbOz
    } else
    {
      // error
    }
    rx2_buffer[rx2_pointer] = 0x00;           //add null zero to string

    // Reset back to initial state                                     
    lock_flag = 0;                            //reset the lock flag
    process_buffer_flag = 0;                  //reset flag
    

    if (isLocked){
      for (int i=0;i<30; i++){lastLocked[i]=' ';}
      lastLocked = weight;
      lockedOz = outOz;
    }

    // Serial1.println(rx2_buffer);
    // Serial.println("send");
    //Serial1.println("0123456789012");
     
    //Serial.println(getLockStatus() + " " + weight);               //send to debug screen
    //Serial.println("Units: " + getUnits() + " | Tare Mode: " + getTareMode() + " | Status: " +  getStatus());
    

    decimalCounter = 0;
    clear_buffer();                           //clear the rs232 buffer

  } 
}

void Scale::checkPref(){
   preferences.begin("my-app", false);

  if (!isBootUp) {
    if (units != oldUnits) {
      preferences.putUInt("lastUnits", static_cast<int>(units));
      Serial.println("Save Units to: " + String(static_cast<int>(units)));
      oldUnits = units;
    }
  }

  // Match startup units to last used units
  if (isBootUp) {
    Serial.println("units: " + String(static_cast<int>(units)) + " lastUnits: " + String(lastUnits));
    if (static_cast<int>(units) != lastUnits) {
      unitsBtn();
      } else {
      isBootUp = false;
      Serial.println("isBootUp False");
    }
  }

  // Lock "odometer" counter
  // check if locked status changed
  if (isLocked != lastLockedStatus) {
    // if it did and the scale is locked increment locked counter by 1
    if (isLocked == true) {
      lockedCounter++;
      Serial.println("Total number of Locks: " + String(lockedCounter));
      preferences.putUInt("lockedCounter", lockedCounter);
    }
    // regardless change lastLockedStatus to match current isLocked
    lastLockedStatus = isLocked;  
  }
  preferences.end();
}


/**
 * @brief Clears scale buffer
 * 
 */
void Scale::clear_buffer(){
  int x = 20;
  while (x != 0){
    rx2_buffer[x] = 0x00;
    x = x-1;
  }
}

/**
 * @brief get weight from scale
 * 
 * @return String 
 */
String Scale::getWeight(){
if (hasSignalFlag) {
        return weight;
    }
    return weight;
}

String Scale::getLegacyWeight(){
if (hasSignalFlag) {
        return legRemWeigh;
    }
    return legRemWeigh;
}

/**
 * @brief Get units from scale
 * 
 * @return String 
 */
String Scale::getUnits() {
// if (!hasSignalFlag) {
//     return "";
//   }
//   else 
  if (units == LB) {
    return "lbs";
  } 
  else if (units == LBOZ) {
    return "";
  } 
  else if (units == KG) {
    return "kg";
  }
  else {
    return "";
  }
}

String Scale::getTareMode(){
  switch (tareMode)
  {
  case GROSS:
    return "G";
    break;
  case NET:
    return "N";
    break;
  default:
    return "";
    break;
  }
}

String Scale::getStatus(){
switch (status)
  {
  case VALID:
    return "Valid";
    break;
  case MOTION:
    return "Motion Detected";
    break;
  case OVERUNDER:
    return "Scale is either over or under range";
    break;
  default:
    return "";
    break;
  }
}


/**
 * @brief Get locked status in string format
 * 
 * @return String 
 */
String Scale::getLockStatus() {
// if (!hasSignalFlag) {
  //   return "No Signal";
  // } else 
  if (isLocked) {
    return "LOCKED";
  } else if (!isLocked && atof(weight) <= 0.0) {
    return "READY";
  } else {
    return "Calculating...";
  }
}

/**
 * @brief Get last locked weight value.  If scale is currently locked will return locked value.
 * 
 * @return String 
 */
String Scale::getLastLocked() {
return lastLocked;
}

String Scale::getLockOdo() {
  return String(lockedCounter);
}

String Scale::getLb() {
  return outLb;
}

String Scale::getLockedOz() {
  return lockedOz;
}

String Scale::getOz() {
  return outOz;
}

/**
 * @brief Turn LOCKED LED ON
 * 
 */
void Scale::ledOn(int ledNum){
  digitalWrite(ledNum,LOW);
}
  
/**
 * @brief Turn LOCKED LED OFF
 * 
 */
void Scale::ledOff(int ledNum){
  digitalWrite(ledNum,HIGH); 
}

void Scale::ledRGBStatus(bool red, bool green, bool blue) {
    pinMode(lockLedRed,OUTPUT); 
    pinMode(lockLedGreen,OUTPUT); 
    pinMode(lockLedBlue,OUTPUT); 

    if (red) {
      ledOn(lockLedRed);
    } else {
      ledOff(lockLedRed);
    }

    if (green) {
      ledOn(lockLedGreen);
    } else {
      ledOff(lockLedGreen);
    }

    if (blue) {
      ledOn(lockLedBlue);
    } else {
      ledOff(lockLedBlue);
    }


}

void Scale::zeroBtn(){
  Serial2.write('Z');  
}

void Scale::tareBtn(){
  Serial2.write('T');
}

void Scale::grossMode(){
  Serial2.write('G');
}

void Scale::netMode(){
  Serial2.write('N');
}

void Scale::unitsBtn(){
   Serial2.write('C');
}

void Scale::printBtn(){
   Serial2.write('P');
}