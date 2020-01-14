#ifndef SCALE_H
#define SCALE_H
/**
 * @file Scale.h
 * @author Adam Clarkson (adam@agri-tronix.com)
 * @brief Scale Class
 * @version 0.1
 * @date 2019-10-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <Arduino.h>
#include <vector> 
#include <Preferences.h>

Preferences preferences;

class Scale {
    private:
        char radio_rx_array[31];        // array being recieved on xbee radio
        bool radio_rx_ready = false;    // whether the rec radio string is complete
        int radio_rx_pointer;           // pointer for radio rx buffer
        int no_signal_timer;            // timeout counter used to display No Signal on display
        bool no_sig_flag = 0;           // flag to prevent display from updating on no change of No Signal message
        char output_string[31];         // converted data to send out
        String legRemWeigh = "";
        bool isBootUp = true;

        int RXD2 = 25;                             //define pins to use with uart 2
        int TXD2 = 27;
        int lockLedRed = 13;                         //red lock led on front panel
        int lockLedGreen = 14;                      // green lock len on front panel
        int lockLedBlue = 12;                      // green lock len on front panel
        int scale_print_button = 32;               //this pin generates an interrupt when activity is deteced on serial port controlled by console print button
        char rx2_buffer[25];                       //rs 232 port 2 recieve string from scale
        enum Units { LB, LBOZ, KG };    // Units datatype
        enum Taremode { NET, GROSS };
        enum Status { VALID, MOTION, OVERUNDER };

        // Scale Statuses
        bool isLocked = false;
        bool hasSignalFlag = false;
        char weight[30] = " ";                     //array to hold weight value
        String lastLocked;            // last loocked value
        String lockedOz;
        Units units = LB;
        Taremode tareMode = GROSS;
        Status status = VALID;
        char outLb[2];
        char outOz[4];

        unsigned int lockedCounter = preferences.getUInt("lockedCounter", 0);
        unsigned int lastUnits = preferences.getUInt("lastUnits", units);
        bool lastLockedStatus = isLocked;


        /**
         * @brief Turns on LOCKED Indicator LED
         * 
         */
        void ledOn(int ledNum);
        /**
         * @brief Turns off LOCKED Indicator LED
         * 
         */
        void ledOff(int ledNum);

        void ledRGBStatus(bool, bool, bool);
    public:

        /**
         * @brief Construct a new Scale object
         * 
         * @param   rx          Pin for RX line     (optional)
         * @param   tx          Pin for TX line     (optional)
         * @param   lockLed     Pin for Locked LED  (optional)
         * @param   printPin    Pin to monitor Print button press   (optional)
         */
        Scale();
        Scale(int rx, int tx);
        Scale(int rx, int tx, int lockLed);
        Scale(int rx, int tx, int lockLed, int printPin);
        /**
         * @brief Destroy the Scale object
         * 
         */
        ~Scale();
        /**
         * @brief Begin Serial port communication with Scale
         * @details To be called in setup routine. will initiate all serial communications
         * 
         */
        void begin();

        //------------ radio uart -----------------------------------------------------------------------------
        /**
         * @brief Read Scale Serial String
         * @todo 
         * 
         */
        void readScale();
        void clear_radio_rx_array(void);
        //----------------------Process radio string if flag is set--------------------------------
        void processRadioString();
        String getWeight();
        String getLegacyWeight();
        String getUnits();
        String getTareMode();
        String getStatus();
        String getLockStatus();
        String getLastLocked();
        String getLockedOz();
        String getLb();
        String getOz();
        static void print_pb_isr();
        void clear_buffer();
        void zeroBtn();
        void tareBtn();
        void grossMode();
        void netMode();
        void unitsBtn();
        void printBtn();
};

#endif