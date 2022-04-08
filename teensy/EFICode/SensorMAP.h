#ifndef SENSORMAP_H
#define SENSORMAP_H

#include "NoiseReduced.h"
#include "Constants.h"

// MPX4115A MAP sensor calibration
const double MAPVs = Vs_5;
const double MAPDelta = 0.045; // should be between +/- 0.0675 volts (1.5 * 0.009 * Vs where Vs is 5)
const double MAPSlope = 1E3/(MAPVs*0.009);  //Pa / Volt
const double MAPOffset = 1E3*MAPDelta/(MAPVs*0.009) + 1E3*0.095/0.009;   //Pa
const double MAPConversion = MAPSlope * voltConv;    // Pascals / 1023
const unsigned long minMAPdt = 4000; // in microseconds


class SensorMAP {
    public:
        SensorMAP();
        //Fetch MAP's last stored value
        double getMap();

        //Fetch a new MAP value from the sensor
        double getMapSensor(int* sensorVals);

        //MAP Averaging Getter Functions
        double getMapGauss();
        double getMapData();
        unsigned long getMAPPeak();
        unsigned long getMAPTrough();
        double getPrevdMAP();

        //Reads the MAP Sensor's values and performs averaging
        void readMAP(int* sensorVals);

    private:
        double MAP_val;

        double prevdMAP;
        double prevMAP;
        unsigned long updateddMAP;
        unsigned long MAPPeak; // time of previous peak
        unsigned long MAPTrough; // time of previous trough

        NoiseReduced* MAPAvg;
};

#endif
