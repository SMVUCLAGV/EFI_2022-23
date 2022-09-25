# EFI_2022-23 [DEPRECATED]

# Specs:
## Phase 1: Code Cleanup (by end of summer?)
* Remove obsolete and unused code until we have a bare-bones version of the EFI able to run the engine. 
    * follow good object-oriented programming practices (encapsulation, modularization, etc.)
* The code after cleanup should be well-commented (for smv's future wow), modularized (so that we can add more features in the future), and easily debuggable.

### Features Removed (Temporarily)
* RPM Feedback
* AFR Feedback
* Startup Modifier
* Throttle Adjustment

## Phase 2: Re-addition & new features (throughout  year)
* Re-implemented obsolte code. we might need to buy new sensors and also revamp how EFI is done (move away from using the ideal gas law)
* Allow the ability for the teensy ECU to interface with devices like a raspberry pi for data collection, wireless DAQ, etc.
