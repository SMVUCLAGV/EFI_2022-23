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

### InitializeParameters()
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

### ReadSensors()
* For more information, check out the ADC section of the documentation
* Retrieves data from the MAX11624EEG+ ADC's outputs
    * If ADC can be refreshed, refresh ADC data
    * Then, If ADC output values are valid
        * retrieve all analog sensor readings

### CountRevolutions()


* Records and increments the total amount of revolutions the engine has gone through
* We keep three variables for counting engine revolutions
    * `revolutions` responsible for RPM calculations (see updateRPM)
    * `totalRevolutions` Counts the total amount of revs since the ECU first turned on
    * `startingRevolutions` responsible for measuring revolutions every time the engine restarts while the electrical system is still on            
* First, check for conditions where there may be undefined behavior
    * If called too soon since the last revolution (an unreasonably short amount of time (smaller than `minDelayPerRev`) has passed since the last hall effect sensor hit), the function returns without the rev counts incrementing
    * When `micros()` overflows (in which case `micros` will be smaller than `previousRev`), the function also returns
    * If the engine is overheating (exceeds `MAX_ALLOWABLE_ECT`), the function also returns, preventing fuel combustion any further to protect mechanical components of the engine

* If none of the above conditions occur, we proceed by recording the timestamp of the current revolution, and increment all the rev counters
    * stored as variable `previosRev` so that it can be referred to in the future for RPM calculations




# Sensor Modules

## 