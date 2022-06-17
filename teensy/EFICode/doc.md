# EFI 2022-2023 V1 Documentation

prepared by mike qu

# EFICode.ino

## Main Goals of our ECU
* Electronic Fuel Injection: Robustness, Efficiency, Reliability
* Output useful data for analysis
<br>




# Controller Module 


## Controller.cpp
<br>

***

### Constructor

>Returns: None\
>Parameters: None

Sets up an EFI Controller object 

```c++
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
    this->initializeParameters();

    // Update sensors to their initial values.
    this->readSensors();
}
```

***

### initializeParameters()
>Returns: None\
>Parameters: None

Initializes EFI's core parameters with their starting values

```c++
void Controller::initializeParameters() {
    // Initialize all 3 Rev Counters.
    revolutions = 0; // Counts revs within the current RPM calc period
    totalRevolutions = 0; // Counts total revs since ECU was first turned on
    startingRevolutions = 0; // Counts revs from the last time the engine was running

    // Number of revolutions that must pass before recalculating RPM.
    previousRev = micros(); // timestamp of last flywheel HES hit

    // Initialize ADC's SPI comms with the Microcontroller
    adc = new SPI_ADC();
    refreshAvailable = true;

    // Initialize AFR values.
    AFR = 0; // unused

    // Initialize Analog Sensors
    s_temp = new SensorTemp(); //Temperature Sensors: ECT and IAT
    s_map = new SensorMAP(); //MAP
    s_tps = new SensorTPS(); //TPS

    // Initialize MAP and RPM indicies to zero.
    mapIndex = 0; //discrete MAP levels for AFR table
    rpmIndex = 0; //discrete RPM levels for AFR table

    // Initialize injector to disabled mode.
    // Used to detach the timer interrupt for pulsing off when the engine is not running.
    INJisDisabled = true;
    constModifier = 1.0; //injector pulse time modifier

    // Used to determine the amount of fuel used. (W22)
    lastPulse = 0; // timestamp of latest injector pulse
    totalPulseTime = 0; // accumulated injector on time
    totalFuelUsed = 0; // approximation of how much total fuel is used

    enableSendingData = true;

    currentlySendingData = enableSendingData;
    haveInjected = false;
    if(enableSendingData) {
      SDConnected = SD.begin(BUILTIN_SDCARD); // check if SD card is connected
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
```
***

### readSensors()

>Returns: Boolean, indicating whether read is successful (not implemented)\
>Parameters: None

Retrieves analog sensor data readings from the MAX11624EEG+ ADC's outputs

For more information, check out the ADC section of the documentation

```c++
bool Controller::readSensors() {
  //If ADC can be refreshed, refresh ADC data
  if (refreshAvailable){
    adc->refresh();
    refreshAvailable = false;
  }

  adc->checkEOC();
  
  // If ADC values are valid, retrieve all analog sensor readings
  if (adc->get_validVals() == 1){
    const int* channels = adc->getChannels();
    sensorVals = channels;

    s_tps->getTPSSensor(sensorVals);
    s_temp->getECTSensor(sensorVals);
    s_temp->getIATSensor(sensorVals);
    s_map->readMAP(sensorVals);

    refreshAvailable = true;
  }
  return true;
}
```
***

### countRevolutions()

>Returns: None\
>Parameters: None

Records and increments the total amount of revolutions the engine has gone through. This function is also responsible for turning on the fuel injector on each of the engine's intake stroke. 

This function is triggered on an interrupt by the flywheel's Hall Effect Sensor

```c++
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
  if (s_temp->getECT() > MAX_ALLOWABLE_ECT){
    digitalWrite(LED_1, HIGH);
    return;
  }

  //Inject on every second revolution because this is a 4 stroke engine
  if (!detectEngineOff() && inStartingRevs()) {
    if (totalRevolutions % 2 == 1)
      pulseOn();
  } 
  else {  // inject when the time since the last trough is < 1 period (2 rotations between troughs)
    if (!detectEngineOff() && (s_map->getMapGauss() > s_map->getMap()))//&& ((60 * 1E6) / RPM > micros() - MAPTrough))
      pulseOn();
  }
}
```

* We keep three variables for counting engine revolutions
    * `revolutions` responsible for RPM calculations. It resets to 0 after each calculation cycle for RPM (see `updateRPM()`)
    * `totalRevolutions` counts the total amount of revs since the vehicle's electrical system was first turned on
    * `startingRevolutions` is responsible for measuring revolutions every time the engine restarts while the electrical system is still on. This works in tandem with the `inStartingRevs()` function to enable special fuel injection behavior when the engine is first turned on (more information below)            
* First, three conditions to eliminate where there may be undefined behavior
    * If called too soon since the last revolution (an unreasonably short amount of time (smaller than `minDelayPerRev`) has passed since the last hall effect sensor hit), the function returns without the rev counts incrementing
    * When `micros()` overflows (in which case `micros` will go back to 0 and become smaller than `previousRev`), the function returns with rev counts incrementing
    * If the engine is overheating (exceeds `MAX_ALLOWABLE_ECT`), the function also returns, preventing fuel combustion any further to protect mechanical components of the engine.
* If none of the above conditions occur, we proceed by recording the timestamp of the current HES hit, and increment all the rev counters
    * stored as variable `previousRev` so that it can be referred to in the future for RPM calculations
* There are two situations that calls for different ways to handle fuel injection
    * When the engine has just started, we inject fuel every two revolutions. This is because the intake stroke takes place once every two revolutions, for more information, look at the four-stroke engine (intake, compression, combustion, exhaust). 
    * After the engine has been running a while, we switch to use our MAP (manifold air pressure) sensor for fuel injection. Injection takes place when the average MAP is higher than the current instantaneous MAP, in other words: when there is a sudden dip in manifold pressure. This is because on an intake stroke, the cylinder's volume increases as the piston moves down, drawing air through the intake into the engine. 
    * Why do we treat the two differently
        * When the engine has just started, it is far from reaching its ideal working conditions yet in terms of temperature, pressure, fuel vaporization, etc. By manually timing it to inject every two revolutions, we can ensure it is getting fuel when needed.
        * After a while, the engine has reached its working temperatures, meaning idling and running will be a lot more consistent. In this case, relying on the pressure sensor would be more accurate as the hall effect sensor can sometimes deliver faulty readings (double-counts) over the long term. 
***

### doubleMap()

### enableINJ()

### disableINJ()



# Sensor Modules

## 