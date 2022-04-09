#ifndef SENSORTEMP_H
#define SENSORTEMP_H

//The following constants are to complete the following eq for temperature
//
// Temp = tempBeta / (ln(R) + (tempBeta/T_0 - lnR_0)) //	where R is the resistance of the sensor (found using voltage divider) //	eq from: https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation //
const double tempBetaIAT = 3988; // tolerance: {+/-1%,+/-1.5%}
const double tempBetaECT = 3988;
const double T_0 = 298.15; // temp in Kelvin at which R_0 values are taken
const double lnR_0_IAT = 9.21034; //8.45531}; // {ln(10000 (10000 +/-1%)),ln(4700 (4559 to 4841))}
const double lnR_0_ECT = 8.4849;
const double tempConstIAT = tempBetaIAT/T_0 - lnR_0_IAT;
const double tempConstECT = tempBetaECT/T_0 - lnR_0_ECT;
const double R_divIAT = 10000; // resistance of other resistor in voltage divider
const double R_divECT = 10000;


class SensorTemp {
    public:
        void getECTSensor(int* sensorVals);
        void getIATSensor(int* sensorVals);
        double getECT();
        double getIAT();
    private:
        double IATval;
        double ECTval;
};

#endif

