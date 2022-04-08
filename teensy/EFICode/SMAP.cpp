#include "SMAP.h"

#include "Arduino.h"

SMAP::SMAP(){
    MAPAvg = new NoiseReduced(100);
    MAP_val = 0;
    prevdMAP = 0;
    prevMAP = 0;
    MAPPeak = 0;
    MAPTrough = 0;
    updateddMAP = 0;
}


double SMAP::getMap() {
  return MAP_val;
}

double SMAP::getMapSensor(int* sensorVals) {
  //Calculates MAP, outputs in Pa
  return MAPConversion * sensorVals[MAP_CHAN] + MAPOffset;
}

double SMAP::getMapGauss(){
  return MAPAvg->getGauss();
}

double SMAP::getMapData(){
  return MAPAvg->getData();
}

unsigned long SMAP::getMAPPeak(){
  return MAPPeak;
}

unsigned long SMAP::getMAPTrough(){
  return MAPTrough;
}

double SMAP::getPrevdMAP(){
  return prevdMAP;
}

void SMAP::readMAP(int* sensorVals){
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
