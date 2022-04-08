#include "SensorMAP.h"

#include "Arduino.h"

SensorMAP::SensorMAP(){
    MAPAvg = new NoiseReduced(100);
    MAP_val = 0;
    prevdMAP = 0;
    prevMAP = 0;
    MAPPeak = 0;
    MAPTrough = 0;
    updateddMAP = 0;
}


double SensorMAP::getMap() {
  return MAP_val;
}

double SensorMAP::getMapSensor(int* sensorVals) {
  //Calculates MAP, outputs in Pa
  return MAPConversion * sensorVals[MAP_CHAN] + MAPOffset;
}

double SensorMAP::getMapGauss(){
  return MAPAvg->getGauss();
}

double SensorMAP::getMapData(){
  return MAPAvg->getData();
}

unsigned long SensorMAP::getMAPPeak(){
  return MAPPeak;
}

unsigned long SensorMAP::getMAPTrough(){
  return MAPTrough;
}

double SensorMAP::getPrevdMAP(){
  return prevdMAP;
}

void SensorMAP::readMAP(int* sensorVals){
  MAP_val = getMapSensor(sensorVals);

  MAPAvg->addData(MAP_val);
  // Update MAPPeak and MAPTrough
  if(updateddMAP - micros() > minMAPdt) {
    double dMAP = MAPAvg->getGauss() - prevMAP;
    if((prevdMAP < 0) != (dMAP < 0)) { // if slopes have different sign
      if(dMAP < 0)
        MAPPeak = micros();
      else
        MAPTrough = micros();
	  }
    prevdMAP = dMAP;
		prevMAP = MAPAvg->getGauss();
	  updateddMAP = micros();
  }
}
