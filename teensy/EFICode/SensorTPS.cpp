#include "SensorTPS.h"

#include "Arduino.h"
#include "Constants.h"

SensorTPS::SensorTPS() {
    lastThrottleMeasurementTime = micros();
    TPSval = 0;
}

void SensorTPS::getTPSSensor(int* sensorVals) {
  unsigned long currThrottleMeasurementTime = micros();
  // calculate open throttle area (i think)
  //double newTPS = 1 - cos(((double(analogRead(TPS_Pin))-TPS_0Deg)/(TPS_90Deg - TPS_0Deg))*HALF_PI);

  double newTPSVal = sensorVals[TPS_CHAN];
  double newTPS = doubleMap(newTPSVal, TPS_0Deg, TPS_90Deg, 0, 1); //need to re-adjust TPS_0Deg and TPS_90Deg
  
  if(newTPS < 0)
    newTPS = 0;
  if(newTPS > 1)
    newTPS = 1;
  if(currThrottleMeasurementTime - lastThrottleMeasurementTime > 0)
    DTPS = (newTPS - TPSval) / (currThrottleMeasurementTime - lastThrottleMeasurementTime);
  lastThrottleMeasurementTime = currThrottleMeasurementTime;
  TPSval = newTPS;
}

double SensorTPS::getTPS() {
    return TPSval;
}

double SensorTPS::doubleMap(double val, double minIn, double maxIn, double minOut, double maxOut){
  return ((val - minIn) / (maxIn - minIn)) * (maxOut - minOut) + minOut;
}
