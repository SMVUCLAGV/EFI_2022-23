#include "Controller.h"

#include "Arduino.h"
#include "math.h"

// timepassed is in microseconds
long Controller::getRPM (long int timePassed, int rev) {
  return (60 * 1E6 * rev) / (timePassed);
}

//TPS Measurement
/*
const double TPSConversion = .0019685;
const double TPSOffset = -.33746;
*/
const double TPS_0Deg = 54;
const double TPS_90Deg = 951;

double Controller::getTPS() {
  unsigned long currThrottleMeasurementTime = micros();
  // calculate open throttle area (i think)
  //double newTPS = 1 - cos(((double(analogRead(TPS_Pin))-TPS_0Deg)/(TPS_90Deg - TPS_0Deg))*HALF_PI);

  double newTPSVal = sensorVals[TPS_CHAN];
  double newTPS = this->doubleMap(newTPSVal, TPS_0Deg, TPS_90Deg, 0, 1); //need to re-adjust TPS_0Deg and TPS_90Deg
  
  if(newTPS < 0)
    newTPS = 0;
  if(newTPS > 1)
    newTPS = 1;
  if(currThrottleMeasurementTime - lastThrottleMeasurementTime > 0)
    DTPS = (newTPS - TPS) / (currThrottleMeasurementTime - lastThrottleMeasurementTime);
  lastThrottleMeasurementTime = currThrottleMeasurementTime;
  return newTPS;
}