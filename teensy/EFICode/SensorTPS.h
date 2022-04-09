#ifndef SENSORTPS_H
#define SENSORTPS_H

const double TPS_0Deg = 54;
const double TPS_90Deg = 951;

class SensorTPS {
    public:
        SensorTPS();
        void getTPSSensor(int* sensorVals);
        double doubleMap(double val, double minIn, double maxIn, double minOut, double maxOut);
        double getTPS();
    private:
        double TPSval;
        unsigned long lastThrottleMeasurementTime;
        double DTPS;
};

#endif