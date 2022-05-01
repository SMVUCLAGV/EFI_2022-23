# EFI 2022-2023 Documentation

prepared by Mike Qu

# Controller Module

## Controller.cpp


> Constructor  
* Creates a controller object
    * Sets Up Serial Comms
        * note: we also to prevent blocking when teensy is not plugged into the computer by setting a timer
    * Sets up Injection Pin outputs using pinMode
    * Calls `InitializeParameters`
    * Calls `readSensors`

> InitializeParameters()
* Initializes parameters for the engine's recorded data
    * Initialize engine rev count
    * Initialize RPM data
    * Initialize ADC
    * Initialize AFR values
    * Initialize all sensors (MAP, TPS, IAT, ECT)
    * Initialize EFI
    * Initialize data sending

> ReadSensors()
* Essentially retrieves data from the MAX11624EEG+ ADC's outputs
    * If ADC can be refreshed, refresh ADC data
    * Then, If ADC output values are valid
        * get all analog sensor readings

> CountRevolutions()
* Records and increments the total amount of revolutions the engine has gone through
    * When called too soon, the function exits
    * Records the timestamp of the current revolution so that it can be compared to in the future

# Sensor Modules