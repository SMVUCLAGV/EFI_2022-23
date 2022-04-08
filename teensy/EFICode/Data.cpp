#include "Controller.h"

#include "Arduino.h"
#include "SD.h"
#include "SPI.h"

void Controller::sendCurrentData() { // THIS MUST TAKE LESS THAN 1 ms (to guaruntee micros doesn't miss an overflow)
// TODO:
// -last row and column used in tables ( maybe send back actual values used)
// -whether we are still on startup cycles
// -engine on? (or just use RPM = 0)
  char toSend [1000];
  sprintf(toSend, "%010u:%06i:%03.3f:%03.3f:%03.3f:%03.3f:%03.3f:%03.3f:%05i:%05i:%05i:%02.2f:%02.2f:%01.3f:%01i:%01i:%010u:%03.3f:%03.3f:%01i:%s:%01i:%03.3f\n", // about 97 bytes? (800-900 us)
  	micros(), 
	totalRevolutions, 
	ECT, 
	IAT, 
	s_map->getMap(), 
	s_map->getMapData(),
	TPS, 
	AFR, 
	RPM, 
  injectorPulseTime,
	lastPulse,
	scaledMAP, 
	scaledRPM,
	AFR,
	startingRevolutions <= numRevsForStart,
	haveInjected,
	s_map->getMAPTrough(),
  s_map->getPrevdMAP(),
  s_map->getMapGauss(),
  SDConnected,
  fileName,
  ECT > MAX_ALLOWABLE_ECT,
  AFR);

  if(SDConnected) { // open and write to file
    File logFile = SD.open(fileName, FILE_WRITE);
    logFile.write(toSend);
    logFile.close();
  }
  Serial.write(toSend);
}

void Controller::trySendingData() {
  if (currentlySendingData) {
      sendCurrentData();
      haveInjected = false;
    }
  }
