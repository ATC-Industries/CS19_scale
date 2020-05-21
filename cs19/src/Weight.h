#ifndef WEIGHT_H
#define WEIGHT_H
/**
 * @file Weight.h
 * @author Adam Clarkson (adam@agri-tronix.com)
 * @brief Weight Class
 * @version 0.1
 * @date 2020-05-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <Arduino.h>


class Weight {
    private:
        char weight[30] = " ";                     //array to hold weight value
        enum Units { LB, LBOZ, KG, NOTUSED };    // Units datatype
        float pounds;
        int wholePounds;
        float ounces;
        float kilograms;

    public:
        float getPounds();
        int getWholePounds();
        float getOunces();
        float getKilograms();
        Units getUnits();
};

#endif