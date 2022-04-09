#include "SensorTemp.h"
#include "math.h"
#include "Constants.h"

//may need to add nointerrupts() ???

void SensorTemp::getIATSensor(int* sensorVals) {
  double tempR = R_divIAT / (maxADC/sensorVals[IAT_CHAN] - 1);
  IATval =  tempBetaIAT / (log(tempR) + tempConstIAT);
}

void SensorTemp::getECTSensor(int* sensorVals) {
  double tempR = R_divECT / (maxADC/sensorVals[ECT_CHAN] - 1);
  ECTval = tempBetaECT / (log(tempR) + tempConstECT);
}

double SensorTemp::getIAT() {
    return IATval;
}

double SensorTemp::getECT() {
    return ECTval;
}
        