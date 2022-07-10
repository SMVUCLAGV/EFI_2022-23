#ifndef SENSORTEMP_H
#define SENSORTEMP_H

class SensorTemp {
    public:
        void getECTSensor(int* sensorVals);
        void getIATSensor(int* sensorVals);
        double getECT();
        double getIAT();
    private:
        double m_IATval;
        double m_ECTval;
};

#endif

