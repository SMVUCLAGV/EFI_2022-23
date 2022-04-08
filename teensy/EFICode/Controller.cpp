#include "Controller.h" //NoiseReduced, Constants.h, Tach.h, spi_adc.h

#include "Arduino.h"
#include "TimerThree.h"
#include "SD.h"

Controller::Controller() {
    // Sets injector pin to output mode. All other pins default to input mode.
    pinMode(INJ_Pin, OUTPUT);

    // Initializes Serial input and output at the specified baud rate.
    Serial.begin(BAUD_RATE);

    // Prevent blocking caused by the lack of a serial connection with a laptop (fixed W22)
    long t = micros();
    while(!Serial && (micros() - t < 1e6));

    // Initializing message
    Serial.write("Initializing...\n");

    // Initialize parameters with their starting values.
    initializeParameters();

    // Update sensors to their initial values.
    readSensors();
}


bool Controller::readSensors() {
  if (refreshAvailable){
    adc->refresh();
    refreshAvailable = false;
  }

  adc->checkEOC();

  if (adc->get_validVals() == 1){
    const int* channels = adc->getChannels();
    sensorVals = channels;
    TPS = getTPS();
    ECT = getECT();
    IAT = getIAT();

    MAP = getMAP();
    MAPAvg->addData(MAP);
    // Update  MAPPeak and MAPTrough
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
    refreshAvailable = true;
  }
    return true;
}


void Controller::initializeParameters() {

    // Start at zero revolutions.
    revolutions = 0;
    totalRevolutions = 0;

    // Number of revolutions that must pass before recalculating RPM.
    constModifier = 1.0;
    previousRev = micros();

    // Initialize ADC
    adc = new SPI_ADC();
    refreshAvailable = true;

    // Initialize AFR values.
    AFR = 0;
    lastThrottleMeasurementTime = micros();

    // Initialize MAP averaging
    MAPAvg = new NoiseReduced(100);
    MAP = 0;
    prevdMAP = 0;
    prevMAP = 0;
    MAPPeak = 0;
    MAPTrough = 0;
    updateddMAP = 0;

    // Initialize MAP and RPM indicies to zero.
    mapIndex = 0;
    rpmIndex = 0;

    // Initialize injector to disabled mode.
    // Used to detach the timer interrupt for pulsing off
    // when the engine is not running.
    INJisDisabled = true;

    // Used to determine the amount of fuel used. (W22)
    totalPulseTime = 0;
    lastPulse = 0;
    totalFuelUsed = 0;

    // True   -> data reporting on.
    // False  -> data reporting off.
    enableSendingData = true;
    currentlySendingData = enableSendingData;
    haveInjected = false;
    if(enableSendingData) {
      SDConnected = SD.begin(BUILTIN_SDCARD);
      if(SDConnected) { // find new fileName
        int fileNumber = 0;
        do {
          sprintf(fileName, "%s%i", baseFileName, fileNumber);
          fileNumber++;
        } while(SD.exists(fileName));
      }
    }


    // Calculate base pulse times from fuel ratio table. Should actually
    // store the last table used and recall it from memory here!
    calculateBasePulseTime(false, 0, 0);

    lastRPMCalcTime = micros();
}

void Controller::countRevolution() {
  //  When called too soon, we skip countRevolution
  //  When micros() overflows, we continue as if its a normal countRevolution
  if (micros() - previousRev > 0 && (micros() - previousRev < minDelayPerRev))
     return;
  previousRev = micros();
  if (INJisDisabled) {
        enableINJ();
  }

  // Increment the number of revolutions
  revolutions++;
  totalRevolutions++;
  startingRevolutions++;

  // MAX TEMP CHECK
  if (ECT > MAX_ALLOWABLE_ECT){
    digitalWrite(LED_1, HIGH);
    return;
  }

  //Inject on every second revolution because this is a 4 stroke engine
  if (!detectEngineOff() && inStartingRevs()) {
      if (totalRevolutions % 2 == 1)
          pulseOn();
  } else {  // inject when the time since the last trough is < 1 period (2 rotations between troughs)
    if (!detectEngineOff() && (MAPAvg->getGauss() > MAP))//&& ((60 * 1E6) / RPM > micros() - MAPTrough))
          pulseOn();
  }
}


void Controller::enableINJ() {
  INJisDisabled = false;
}

void Controller::disableINJ() {
  Timer3.stop();
  digitalWrite(INJ_Pin, LOW);
  noInterrupts();
  INJisDisabled = true;
  interrupts();
}

void Controller::pulseOn() {
  // disable data sending
  currentlySendingData = false;
  if (injectorPulseTime > 2.5E5)
    injectorPulseTime = 2.5E5;
  Timer3.setPeriod(injectorPulseTime);
  digitalWrite(INJ_Pin, HIGH);
  Timer3.start();
  noInterrupts(); //To ensure when lastPulse is used in pulseOff(), it isn't read as lastPulse is getting modified
  lastPulse = micros(); //Race Conditions Problem
  interrupts();
}

void Controller::pulseOff() {
  // When it's time to turn the injector off, follow these steps and turn it off
  digitalWrite(INJ_Pin, LOW);
  Timer3.stop();

  // Save the amount of time the injector pin spent HIGH.
  totalPulseTime += (micros() - lastPulse);

  // Let data be sent again
  currentlySendingData = enableSendingData;
  haveInjected = true;
}


void Controller::updateRPM() {
  noInterrupts();
  int tempRev = revolutions; //Prevents revolutions being read while it is being modified by the
  //countRevolution() function associated with the interrupt
  interrupts();
  if (tempRev >= revsPerCalc) {
    noInterrupts(); //To ensure that the interrupt of countRev doesn't get lost in case of bad timing of threads
    unsigned long currentRPMCalcTime = micros();
    if(currentRPMCalcTime - lastRPMCalcTime > 0) // only write if this value is positive (protect from overflow)
    	RPM = getRPM(currentRPMCalcTime - lastRPMCalcTime, tempRev); //Uses the previously determined value of revolutions to reduce
    //amount of noInterrupts() calls
    lastRPMCalcTime = currentRPMCalcTime;
    revolutions = 0; //Race Conditions Modification Problem
    interrupts();

    // Should also dynamically change revsPerCalc. At lower RPM
    // the revsPerCalc should be lower but at higher RPM it should be higher.
  }
}

long Controller::getFuelUsed() {
  //volumetric flow rate = mass flow rate / density
  return givenFlow * totalPulseTime / density; //in mL
}


long Controller::interpolate2D(int blrow, int blcol, double x, double y) {
    // Takes the coordinate of the bottom left corner of the square to perform 2D interpolation over.
    // x and y must be given in unit form. i.e., y = (yc-y1)/(y2-y1) and x = (xc-x1)/(x2-x1)
    // (0 <= y <= 1 and 0 <= x <= 1)
    return
    injectorBasePulseTimes[blrow][blcol]*(1-y)*(1-x)+
    injectorBasePulseTimes[blrow+1][blcol]*(y)*(1-x)+
    injectorBasePulseTimes[blrow][blcol+1]*(1-y)*(x)+
    injectorBasePulseTimes[blrow+1][blcol+1]*(y)*(x);
  }


double Controller::doubleMap(double val, double minIn, double maxIn, double minOut, double maxOut) {
    return ((val - minIn) / (maxIn - minIn)) * (maxOut - minOut) + minOut;
  }


void Controller::lookupPulseTime() {
    // Map the MAP and RPM readings to the dimensionns of the AFR lookup table
    noInterrupts();

    scaledMAP = doubleMap(MAPAvg->getData(), minMAP, maxMAP, 0, numTableRows - 1); //number from 0 - numTableRows-1
    scaledRPM = doubleMap(RPM, minRPM, maxRPM, 0, numTableCols - 1); //number from 0 - numTableCols-1

    // Clip out of bounds to the min or max value, whichever is closer.
    scaledMAP = constrain(scaledMAP, 0, numTableRows - 1);
    scaledRPM = constrain(scaledRPM, 0, numTableCols - 1);

    // Get lower bounds for load and rpm indicies.
    mapIndex = scaledMAP; // double to int
    rpmIndex = scaledRPM;

    interrupts();

    // Clip extrapolation to the value at the max index. Otherwise, perform 2D interpolation to get
    // the base pulse time and then divide by the temperature.
    long tempPulseTime;
    if (rpmIndex < numTableCols - 1 && mapIndex < numTableRows - 1) {
        // Interpolation case
        tempPulseTime = interpolate2D(mapIndex, rpmIndex, scaledMAP-mapIndex, scaledRPM-rpmIndex) / IAT;
    }
    else {
        // Clipped case
        tempPulseTime = injectorBasePulseTimes[mapIndex][rpmIndex] / IAT;
    }

    // Add extra fuel for starting
    if (inStartingRevs()){
        tempPulseTime *= 1.4;
    }

    noInterrupts();
    injectorPulseTime = openTime + tempPulseTime * constModifier; // ADJUST OPEN TIME
    interrupts();
}


void Controller::calculateBasePulseTime(bool singleVal, int row, int col) {
  if (singleVal) {
    // Cover the range of pressure values from min - max inclusive.
    unsigned long pressure = map(row, 0, numTableRows - 1, minMAP, maxMAP);
    // Compute a base pulse time in units of microseconds * Kelvin. Temperature will be
    // divided on the fly to get the actual pulse time used.
    injectorBasePulseTimes[row][col] = 1E6 * pressure * injectionConstant / (fuelRatioTable[row][col]);
    return;
  }

  for (int x = 0; x < numTableRows; x++) {
    for (int y = 0; y < numTableCols; y++) {
      // Cover the range of pressure values from min - max inclusive.
      unsigned long pressure = map(x, 0, numTableRows - 1, minMAP, maxMAP);
      // Compute a base pulse time in units of microseconds * Kelvin. Temperature will be
      // divided on the fly to get the actual pulse time used.
      injectorBasePulseTimes[x][y] = 1E6 * pressure * injectionConstant / (fuelRatioTable[x][y]);
    }
  }
}


void Controller::updateEngineState() {
  if (detectEngineOff()) {
    revolutions = 0;
    startingRevolutions = 0;
    RPM = 0;
    lastRPMCalcTime = micros();
    if (!INJisDisabled) {
      disableINJ();
    }
  }
}

bool Controller::inStartingRevs() {
   return startingRevolutions <= numRevsForStart;
}


bool Controller::detectEngineOff() {
  // if micros() overflows, we can tolerate defaulting to on state (will just inject less)
  //   it will be worse if the engine goes off when it is actually on which means we'll inject more
  //   (as if in starting state)
  if (micros() - previousRev > SHUTOFF_DELAY) {
    return true;
  }
  return false;
}

