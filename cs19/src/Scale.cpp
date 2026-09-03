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
// #include "SPIFFS.h"

bool Scale::isPrintPressed = false;
bool Scale::isNewLock = false;
bool Scale::isPrintButtonPressed = false;
portMUX_TYPE Scale::mux = portMUX_INITIALIZER_UNLOCKED;

/**
 * @brief Construct a new Scale:: Scale object
 *
 */
Scale::Scale() { isPrintPressed = false; }

/**
 * @brief Construct a new Scale:: Scale object
 *
 * @param rx Pin number of RX line
 * @param tx Pin Number of TX Line
 */
Scale::Scale(int rx, int tx) {
  RXD2 = rx;
  TXD2 = tx;
  changePrintStatus(false);
}

/**
 * @brief Construct a new Scale:: Scale object
 *
 * @param rx Pin Number
 * @param tx Pin Number
 * @param lockLed Pin Number
 */
Scale::Scale(int rx, int tx, int lockLed) {
  RXD2 = rx;
  TXD2 = tx;
  lockLedRed = lockLed;
  isPrintPressed = false;
}

/**
 * @brief Construct a new Scale:: Scale object
 *
 * @param rx Pin Number
 * @param tx Pin Number
 * @param lockLed Pin Number
 * @param printPin Pin Number
 */
Scale::Scale(int rx, int tx, int lockLed, int printPin) {
  RXD2 = rx;
  TXD2 = tx;
  lockLedRed = lockLed;
  scale_print_button = printPin;
  isPrintPressed = false;
}

Scale::~Scale() {}

/**
 * @brief Begin Serial port communication with Scale
 * @details To be called in setup routine. will initiate all serial communications
 *
 */
void Scale::begin() {
  isBootUp = true;
  preferences.begin("my-app", false);
  lockedCounter = preferences.getUInt("lockedCounter", 0);
  Serial.println("Total number of Locks: " + String(lockedCounter));
  units = NOTUSED;
  lastUnits = preferences.getUInt("lastUnits", 0);
  preferences.end();
  // Make sure LEDs are off first thing
  ledRGBStatus(0, 0, 0);
  pinMode(scale_print_button, INPUT);
  attachInterrupt(digitalPinToInterrupt(scale_print_button), this->Scale::print_pb_isr,
                  CHANGE);  // this calls 'print_pb()' when user presses print button on cs-19
  // RS232 comm with xBee Radio
  Serial1.begin(9600, SERIAL_8N1, 19, 21);
  // RS232 input from CS19
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(500);
  Serial1.print("+++");
  Serial.println("\n+++ Sent");
  delay(1500);
  if (Serial1.available()) {
    // delay(500);
    String response = Serial1.readStringUntil('\n');
    Serial.println(response);
    response.trim();
    // delay(500);
    if (response.equals("OK")) {
      Serial.println("xBee Radio Found");
      //  Rainbow the leds at startup
      for (int i = 0; i < 4; i++) {
        ledRGBStatus(1, 0, 0);
        delay(300);
        ledRGBStatus(0, 1, 0);
        delay(300);
        ledRGBStatus(0, 0, 1);
        delay(300);
      }
      ledRGBStatus(0, 0, 0);
    } else {
      Serial.println("Serial 1 Found\nNo Legacy Board Found");
      // Blink LOCKED LED at startup
      int x = 8;
      while (x != 0) {
        ledRGBStatus(1, 0, 0);
        delay(150);
        ledRGBStatus(0, 0, 0);
        delay(150);
        x = x - 1;
      }
    }
  } else {
    Serial.println("Serial 1 Not Found\nNo Legacy Board Found");
    // Blink LOCKED LED at startup
    int x = 8;
    while (x != 0) {
      ledRGBStatus(1, 0, 0);
      delay(150);
      ledRGBStatus(0, 0, 0);
      delay(150);
      x = x - 1;
    }
  }
  // readScale();
  int timeoutCounter = 0;

  // while(units == NOTUSED){
  //   Serial.println("Checking Units: " + String(static_cast<int>(units)) + ". Loading... ");
  //   readScale();
  // }
  while (units != lastUnits) {
    unitsBtn();
    Serial.println("Checking Units: " + String(static_cast<int>(units)) + ". Should be: " + String(lastUnits) +
                   ". Pressing Units... ");
    timeoutCounter++;
    readScale();
    if (timeoutCounter > 0) {
      break;
    }
  }
  isBootUp = false;
}

/**
 * @brief Interupt Service Routine to for Print button
 *
 */
void IRAM_ATTR Scale::print_pb_isr() {  // This is an  isr that is called when CS-19 print button is pressed
  portENTER_CRITICAL_ISR(&mux);
  isPrintButtonPressed = true;
  portEXIT_CRITICAL_ISR(&mux);
}

int countDecimalPlaces(const char* str) {
  const char* dot = strchr(str, '.');
  if (!dot) return 0;
  int count = 0;
  for (const char* p = dot + 1; *p && isdigit(*p); ++p) {
    ++count;
  }
  return count;
}

String Scale::normalizeWeightString(const String &value) const {
  String normalized = value;
  normalized.trim();
  return normalized;
}

bool Scale::parseWeightString(const String &value, float &parsedWeight) const {
  String normalized = normalizeWeightString(value);
  if (normalized.length() == 0 || normalized == "---") {
    return false;
  }

  if (normalized.indexOf("lb") != -1 && normalized.indexOf("oz") != -1) {
    int lbIndex = normalized.indexOf("lb");
    int ozIndex = normalized.indexOf("oz");
    String pounds = normalized.substring(0, lbIndex);
    String ounces = normalized.substring(lbIndex + 2, ozIndex);
    pounds.trim();
    ounces.trim();
    parsedWeight = pounds.toFloat() + (ounces.toFloat() / 16.0f);
    return true;
  }

  parsedWeight = normalized.toFloat();
  return true;
}

String Scale::getApiUnits() const {
  switch (units) {
    case KG:
      return "kg";
    case LB:
    case LBOZ:
      return "lb";
    default:
      return "";
  }
}

String Scale::getApiUnitMode() const {
  switch (units) {
    case KG:
      return "kg";
    case LB:
      return "lb";
    case LBOZ:
      return "lb_oz";
    default:
      return "unknown";
  }
}

String Scale::getApiLockState() const {
  if (isLocked) {
    return "locked";
  }
  if (atof(weight) <= 0.0f) {
    return "ready";
  }
  return "calculating";
}

String Scale::getApiStatus() const {
  switch (status) {
    case VALID:
      return "valid";
    case MOTION:
      return "motion";
    case OVERUNDER:
      return "over_under";
    default:
      return "unknown";
  }
}

/**
 * @brief read scale RS232 signal
 *
 */
void Scale::readScale() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();

  if (isPrintButtonPressed) {
    // insert code here to execute when cs19 print button is pressed
    //  NOTE this routine will be called for every character coming in off of serial port.
    //  need code eliminate duplicates
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200) {
      Serial.println("Print button pressed");  //***diagnostic*** to check that esp32 is ack the print button
      changePrintStatus(true);
    }
    last_interrupt_time = interrupt_time;
    portENTER_CRITICAL(&mux);
    isPrintButtonPressed = false;
    portEXIT_CRITICAL(&mux);
  }
  static int rx2_pointer;        // pointer for rs 232 port 2 rx string
  bool process_buffer_flag = 0;  // flag to signal to process rx2 string
  bool lock_flag = 0;            // flag for lock condition
  static int decimalCounter;

  // check uart for input from scale
  if (Serial2.available()) {
    hasSignalFlag = true;
    rx2_buffer[rx2_pointer] = Serial2.read();  // read character and place in buffer
    switch (rx2_buffer[rx2_pointer]) {
      case 0x02:               // if beginning of string character reset the pointer
        rx2_buffer[0] = 0x02;  // set first character to 0x02
        rx2_pointer = 0;       // reset the pointer
        decimalCounter = 0;

        break;
      case 0x2E:
        decimalCounter++;
        break;
      case 'H':         // if there is an 'H' in the string then set flag to turn on lock led
        lock_flag = 1;  // set flag so lock light will come on when processing string
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
        process_buffer_flag = 1;  // set flag so code will process buffer
        Serial.println("Complete weight reading received at " + String(millis()) + "ms. Processing buffer.");
        break;
      default:
        break;
    }

    // if (rx2_buffer[rx2_pointer] == 0x02) {
    //   rx2_buffer[0] = 0x02;                               //set first character to 0x02
    //   rx2_pointer = 0;                                    //reset the pointer
    // } else if(rx2_buffer[rx2_pointer] == 'H'){            //if there is an 'H' in the string then set flag to turn on
    // lock led
    //   lock_flag = 1;                                      //set flag so lock light will come on when processing
    //   string
    // } else if(rx2_buffer[rx2_pointer] == 0x0D){           //check for end of string
    //   process_buffer_flag = 1;                            //set flag so code will process buffer
    // }
    if (decimalCounter >= 2) {
      units = LBOZ;
      checkPref();
    }
    if (++rx2_pointer >= 24) {  // increment pointer and check for overflow
      rx2_pointer = 0;          // reset pointer on buffer overflow
      decimalCounter = 0;
    }
  } else {
    hasSignalFlag = false;
  }
  if (rx2_buffer[11] == 'H') {
    lock_flag = 1;
  }
  // fix for bobs software
  if (rx2_buffer[11] == ' ') {
    rx2_buffer[11] = 'M';
  }
  if (process_buffer_flag) {
    if (lock_flag == 1) {
      ledOn(lockLedRed);  // turn on lock light
      isLocked = true;
    } else {
      ledOff(lockLedRed);  // turn off lock light
      isLocked = false;
    }
    char legacyRemWeight[30] = "\x02 ";
    strncpy(legacyRemWeight + 2, rx2_buffer + 1, 14);

    // Debug output
    Serial.println("Raw rx2_buffer: " + String(rx2_buffer));
    Serial.println("legacyRemWeight: " + String(legacyRemWeight));

    float weightValue = atof(legacyRemWeight + 2);
    int decimalPlaces = countDecimalPlaces(legacyRemWeight + 2);

    Serial.println("Extracted weight value: " + String(weightValue));
    Serial.println("Decimal places: " + String(decimalPlaces));

    char outputWeight[30];
    if (weightValue <= 0 || legacyRemWeight[12] == 'O') {
      // Format zero weight to match positive weight format, but use zeros
      snprintf(outputWeight, sizeof(outputWeight), "\x02     %.*fLG%c", decimalPlaces, 0.0,
               legacyRemWeight[12] == 'O' ? 'O' : 'M');
      Serial.println("Sending zero/negative weight to Serial1");
    } else {
      // For positive weights, use the original string
      strncpy(outputWeight, legacyRemWeight, sizeof(outputWeight) - 5);
      Serial.println("Sending positive weight to Serial1");
    }

    // Append the ending characters
    size_t len = strlen(outputWeight);
    outputWeight[len] = 0x0D;
    outputWeight[len + 1] = 0x0A;
    outputWeight[len + 2] = 0x02;
    outputWeight[len + 3] = 0x0D;
    outputWeight[len + 4] = 0x0A;
    outputWeight[len + 5] = '\0';

    Serial1.print(outputWeight);
    legRemWeigh = outputWeight;

    // Debug: Print final output
    Serial.println("Final output sent to legacy remote: " + String(outputWeight));
    Serial.println("Time since last update: " + String(millis() - lastUpdateTime) + " ms");
    lastUpdateTime = millis();

    Serial.print("Raw bytes sent to Serial1:");
    for (int i = 0; outputWeight[i] != '\0'; i++) {
      Serial.print(" " + String((uint8_t)outputWeight[i], HEX));
    }
    Serial.println();

    // output weight string
    if (units == LB) {
      // Clear weight Char array
      memset(weight, 0, sizeof(weight));

      strncpy(weight, rx2_buffer + 1, 8);
      // Serial.println(weight);
    } else if (units == KG) {
      // Clear weight Char array
      memset(weight, 0, sizeof(weight));

      strncpy(weight, rx2_buffer + 1, 8);
      // Serial.println(weight);
    }

    else if (units == LBOZ) {
      // for (int i=0;i<30; i++){weight[i]=' ';}
      memset(weight, 0, sizeof(weight));
      memset(outLb, 0, sizeof(outLb));
      memset(outOz, 0, sizeof(outOz));
      // for (int i=0;i<2; i++){outLb[i]=' ';}
      // for (int i=0;i<4; i++){outOz[i]=' ';}

      char outLbOz[12];
      strncpy(outLb, rx2_buffer + 3, 2);
      strncpy(outOz, rx2_buffer + 6, 4);
      for (int i = 0; i < 2; i++) {
        outLbOz[i] = outLb[i];
      }
      outLbOz[2] = 'l';
      outLbOz[3] = 'b';
      outLbOz[4] = ' ';
      for (int i = 0; i < 4; i++) {
        outLbOz[i + 5] = outOz[i];
      }
      outLbOz[9] = 'o';
      outLbOz[10] = 'z';
      outLbOz[11] = '\0';  // terminate the char*

      strncpy(weight, outLbOz, 12);  // create output string from outLbOz
    } else {
      // error
    }
    rx2_buffer[rx2_pointer] = 0x00;  // add null zero to string

    // Reset back to initial state
    lock_flag = 0;            // reset the lock flag
    process_buffer_flag = 0;  // reset flag

    if (isLocked) {
      for (int i = 0; i < 30; i++) {
        lastLocked[i] = ' ';
      }
      lastLocked = weight;
      lockedOz = outOz;
    }

    // Serial1.println(rx2_buffer);
    // Serial.println("send");
    // Serial1.println("0123456789012");

    // Serial.println(getLockStatus() + " " + weight);               //send to debug screen
    // Serial.println("Units: " + getUnits() + " | Tare Mode: " + getTareMode() + " | Status: " +  getStatus());

    decimalCounter = 0;
    clear_buffer();  // clear the rs232 buffer
  }

  // READ XBEE and check if any commands are sent
  if (Serial1.available()) {
    String response = Serial1.readStringUntil('\x03');
    Serial.println(response);
    response.trim();

    // verify the value exists. (not 0 in length)
    if (response.length() > 0) {
      for (int i = 0; i < response.length(); i++) {
      }
      // Zero button pressed
      if (response.indexOf("Z") != -1 || response.indexOf("z") != -1) {
        Serial.println("Zero Pressed");
        zeroBtn();
      }
      // Tare button pressed
      else if (response.indexOf("t") != -1 || response.indexOf("T") != -1) {
        Serial.println("tare Pressed");
        tareBtn();
      }
      // Units button pressed
      else if (response.indexOf("c") != -1 || response.indexOf("C") != -1) {
        Serial.println("Units Pressed");
        unitsBtn();
      }
      // Net/Gross button pressed
      else if (response.indexOf("n") != -1 || response.indexOf("N") != -1) {
        Serial.println("Net Pressed");
        grossMode();
      } else if (response.indexOf("g") != -1 || response.indexOf("G") != -1) {
        Serial.println("Gross Pressed");
        netMode();
      }
    }
  }
}

void Scale::checkPref() {
  preferences.begin("my-app", false);

  if (!isBootUp) {
    if (units != oldUnits) {
      preferences.putUInt("lastUnits", static_cast<int>(units));
      Serial.println("Save Units to: " + String(static_cast<int>(units)));
      oldUnits = units;
    }
  }

  // Match startup units to last used units
  //  int timeoutCounter = 0;
  if (isBootUp) {
    // Serial.println("units: " + String(static_cast<int>(units)) + " lastUnits: " + String(lastUnits));
    //  delay(250);

    // if (static_cast<int>(units) != lastUnits) {
    //   unitsBtn();
    //   //delay(250);
    //   } else {
    //   isBootUp = false;
    //   Serial.println("isBootUp False");
    // }
    // timeoutCounter++;
    // if (timeoutCounter >20) {
    //   isBootUp = false;
    // }
  }

  // Lock "odometer" counter
  // check if locked status changed
  if (isLocked != lastLockedStatus) {
    // if it did and the scale is locked increment locked counter by 1
    if (isLocked == true) {
      lockedCounter++;
      updateLastLock(weight);
      isNewLock = true;
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
void Scale::clear_buffer() {
  int x = 20;
  while (x != 0) {
    rx2_buffer[x] = 0x00;
    x = x - 1;
  }
}

/**
 * @brief get weight from scale
 *
 * @return String
 */
String Scale::getWeight() {
  if (hasSignalFlag) {
    return weight;
  }
  return weight;
}

String Scale::getLegacyWeight() {
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
  if (units == LB) {
    return "lbs";
  } else if (units == LBOZ) {
    return "";
  } else if (units == KG) {
    return "kg";
  } else {
    return "";
  }
}

String Scale::getTareMode() {
  switch (tareMode) {
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

String Scale::getStatus() {
  switch (status) {
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
  if (units == LB || units == KG) {
    // Serial.println("getLastLocked: ");
    // Serial.println(lastLocked);
    // return String(lastLocked.toFloat());
    return String(lastLocked);
  }
  return String(String(lastLocked.toInt()) + "lb " + String(getLockedOz().toFloat(), 1) + "oz");
  // return lastLocked;
}

void Scale::updateLastLock(String weight) {
  last5 = last4;
  last4 = last3;
  last3 = last2;
  last2 = last1;
  last1 = weight;
  return;
}

String Scale::getLast1() { return last1; }
String Scale::getLast2() { return last2; }
String Scale::getLast3() { return last3; }
String Scale::getLast4() { return last4; }
String Scale::getLast5() { return last5; }

String Scale::getLockOdo() { return String(lockedCounter); }

String Scale::getLb() { return outLb; }

String Scale::getLockedOz() { return lockedOz; }

String Scale::getOz() { return outOz; }

/**
 * @brief Turn LOCKED LED ON
 *
 */
void Scale::ledOn(int ledNum) { digitalWrite(ledNum, LOW); }

/**
 * @brief Turn LOCKED LED OFF
 *
 */
void Scale::ledOff(int ledNum) { digitalWrite(ledNum, HIGH); }

void Scale::ledRGBStatus(bool red, bool green, bool blue) {
  pinMode(lockLedRed, OUTPUT);
  pinMode(lockLedGreen, OUTPUT);
  pinMode(lockLedBlue, OUTPUT);

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

String Scale::getPrintButtonStatus(void) {
  if (isAuto) {
    Serial.println("isAuto");
    if (isNewLock) {
      Serial.println("isNewLock");
      if (isLocked) {
        Serial.println("isLocked");
        isNewLock = false;
        return "1";
      }
    }
    return "0";
  } else {
    if (isPrintPressed) {
      if (isLocked) {
        return "1";
      }
    }
    return "0";
  }
}

void Scale::changePrintStatus(bool status) {
  isPrintPressed = status;
  isNewLock = status;
}

void Scale::zeroBtn() { Serial2.write('Z'); }

void Scale::tareBtn() { Serial2.write('T'); }

void Scale::grossMode() { Serial2.write('G'); }

void Scale::netMode() { Serial2.write('N'); }

void Scale::unitsBtn() {
  Serial2.write('C');
  readScale();
  // Serial.println("Units Button Command Sent");

  // unitsBtnCounter++;
}

void Scale::printBtn() { Serial2.write('P'); }

String Scale::getJSON() {
  String output;
  StaticJsonDocument<512> doc;

  doc["weight"] = getWeight();
  doc["units"] = getUnits();
  doc["locked"] = getLockStatus();
  doc["lockedodo"] = lockedCounter;

  JsonArray lastLocked = doc.createNestedArray("lastLocked");
  lastLocked.add(getLastLocked());
  lastLocked.add(getLast2());
  lastLocked.add(getLast3());
  lastLocked.add(getLast4());
  lastLocked.add(getLast5());

  serializeJson(doc, output);

  return output;
}

String Scale::getApiJSON() {
  String output;
  StaticJsonDocument<768> doc;
  float parsedWeight = 0.0f;
  const String currentWeight = getWeight();

  doc["weight"] = parseWeightString(currentWeight, parsedWeight) ? parsedWeight : nullptr;
  doc["units"] = getApiUnits();
  doc["unit_mode"] = getApiUnitMode();
  doc["display_weight"] = currentWeight;
  doc["locked"] = isLocked;
  doc["lock_state"] = getApiLockState();
  doc["stable"] = (status == VALID);
  doc["status"] = getApiStatus();
  doc["lock_sequence"] = lockedCounter;
  doc["gross_net"] = (tareMode == GROSS) ? "gross" : "net";
  doc["gross"] = (tareMode == GROSS);
  doc["net"] = (tareMode == NET);
  doc["valid"] = (status == VALID);
  doc["motion"] = (status == MOTION);
  doc["over_under"] = (status == OVERUNDER);
  doc["has_signal"] = hasSignalFlag;

  JsonArray recentLocked = doc.createNestedArray("recent_locked_weights");
  const String recentValues[] = {getLastLocked(), getLast2(), getLast3(), getLast4(), getLast5()};
  for (const String &recentValue : recentValues) {
    float parsedRecent = 0.0f;
    if (parseWeightString(recentValue, parsedRecent)) {
      recentLocked.add(parsedRecent);
    } else {
      recentLocked.add(nullptr);
    }
  }

  serializeJson(doc, output);
  return output;
}

String Scale::getApiStateKey() {
  return getWeight() + "|" + getUnits() + "|" + getLockStatus() + "|" + getStatus() + "|" + getTareMode() + "|" +
         String(lockedCounter) + "|" + getLastLocked() + "|" + String(hasSignalFlag ? 1 : 0);
}
