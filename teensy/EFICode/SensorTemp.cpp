#include "SensorTemp.h"
#include "math.h"
#include "Constants.h"

//The following constants are to complete the following eq for temperature
//
// Temp = tempBeta / (ln(R) + (tempBeta/T_0 - lnR_0)) //	where R is the resistance of the sensor (found using voltage divider) //	eq from: https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation //
const double TEMP_BETA_IAT = 3988; // tolerance: {+/-1%,+/-1.5%}
const double TEMP_BETA_ECT = 3988;
const double T_0 = 298.15; // temp in Kelvin at which R_0 values are taken
const double lnR_0_IAT = 9.21034; //8.45531}; // {ln(10000 (10000 +/-1%)),ln(4700 (4559 to 4841))}
const double lnR_0_ECT = 8.4849;
const double tempConstIAT = TEMP_BETA_IAT/T_0 - lnR_0_IAT;
const double tempConstECT = TEMP_BETA_ECT/T_0 - lnR_0_ECT;
const double R_DIV_IAT = 10000; // resistance of other resistor in voltage divider
const double R_DIV_ECT = 10000;

//may need to add nointerrupts() ???

void SensorTemp::getIATSensor(int* sensorVals) {
  double tempR = R_DIV_IAT / (maxADC/sensorVals[IAT_CHAN] - 1);
  m_IATval =  TEMP_BETA_IAT / (log(tempR) + tempConstIAT);
}

void SensorTemp::getECTSensor(int* sensorVals) {
  double tempR = R_DIV_ECT / (maxADC/sensorVals[ECT_CHAN] - 1);
  m_ECTval = TEMP_BETA_IAT / (log(tempR) + tempConstECT);
}

double SensorTemp::getIAT() {
    return m_IATval;
}

double SensorTemp::getECT() {
    return m_ECTval;
}
        