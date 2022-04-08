#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "SensorMAP.h"
#include "spi_adc.h"

class Controller {
public:
  // Constructor
  Controller();

  // Updates sensor values.
  bool readSensors();

  // Sends the current sensor values and requested parameter information.
  void sendCurrentData();

  // If the currentlySendingData flag is set and timer1 triggers an interrupt sends out a data sample.
  void trySendingData();

  // Increments the counted number of revolutions since the last RPM update.
  // Every 2nd revolution, switches the INJ_Pin to HIGH and sets a timer for
  // the amount of time equal to the most recent calculated pulse time.
  void countRevolution();

  // Updates the RPM reading by dividing the total number of revolutions since
  // the last update by the number of minutes that have passed.
  void updateRPM();

  // Performs 2D interpolation to lookup a pulse time for the given engine conditions.
  void lookupPulseTime();

  //Finds the total amount of fuel used by measuiring the amount of fuel injected every revolution
  //and accumulating the amount. Returns fuel level.
  long getFuelUsed();

  // If singleVal is true, determines the pulse time for the specified cell [row][col]
  // of the AFR Table in units of microseconds times kelvin.
  // If singleVal is false, determines the pulse time for every single AFR Table value
  // in the AFR Table. In its current state, this will overwrite any adjustments that
  // the O2 sensor feedback loop has made to the base pulse times.
  void calculateBasePulseTime(bool singleVal, int row, int col);

  // Returns true if the engine drops below the minimum RPM to consider the engine running.
  bool detectEngineOff();

  // Checks to see if the engine is on or off. If the engine switches state since the last
  // check, changes parameters accordingly.
  void updateEngineState();

  bool inStartingRevs();

  // Turns the injector on if it is not disabled.
  void pulseOn();

  // Turns the injector off.
  void pulseOff();

  // Attaches the interrupt timer for shutting the injector off and
  // sets the INJisDisabled flag to false.
  void enableINJ();

  // Detaches the interrupt timer for shutting the injector off and
  // sets the INJisDisabled flag to true.
  void disableINJ();

  long getRPM (long int timePassed, int rev);
  double getTPS();
  double getIAT();
  double getECT();
  double getMAP();

  void initializeParameters();

  long interpolate2D(int blrow, int blcol, double x, double y);
  double doubleMap(double val, double minIn, double maxIn, double minOut, double maxOut);

private:
  // Has a value of true if the timer3 interrupt is detached from the "pulseOff" function.
  // Also prevents the injector from pulsing on if true.
  SPI_ADC* adc;

  bool refreshAvailable;
  const int* sensorVals;

  bool INJisDisabled; //flag

  int revolutions; //misc
  unsigned long totalRevolutions; //misc
  unsigned long startingRevolutions; //misc

  unsigned long previousRev; //misc

  unsigned long totalPulseTime; //misc
  unsigned long lastPulse;
  unsigned long totalFuelUsed;

  long lastRPMCalcTime;//RPM stuff
  long injectorPulseTime;

  //Data Retrieval
  bool enableSendingData;
  bool currentlySendingData; // used to disable data transmission during injection
  bool haveInjected; // allows data retrieval to know when injections happened
  bool SDConnected;
  const char baseFileName[6] = "sdlog";
  char fileName[20] = "NOFILE";

  double scaledMAP;
  double scaledRPM;
  int mapIndex;
  int rpmIndex;

  unsigned long lastThrottleMeasurementTime;

  double constModifier;

  long RPM;
  double TPS;
  double DTPS;
  double ECT;
  double IAT;

  SensorMAP* s_map; //MAP module, responsible for collecting data and processing data from the Manifold Pressure Sensor

  double AFR;

  //
  // For some undocumented reason they use this table to account for
  // Volumetric Efficiency so these values are more like AFR/VE
  //
  double fuelRatioTable[numTableRows][numTableCols] =
    {
    {14.5,14.8,15.0,15.3,15.3,15.2,14.6,14.3,13.8,13.6},  // minimum pressure
    {14.5,14.8,15.0,15.3,15.3,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,15.0,15.3,15.3,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,15.5,15.3,16.0,16.0,15.7,14.3,13.8,13.6},
    {14.5,16.0,19.2,19.2,16.0,16.0,15.7,14.3,13.8,13.6},
    {14.5,19.0,19.2,19.2,16.0,16.0,15.7,14.3,13.8,13.6},
    {14.5,19.0,19.2,19.2,15.0,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,19.2,19.2,15.0,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,19.2,19.2,15.3,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,15.2,15.3,15.3,15.2,14.6,14.3,13.8,13.6},
    {14.5,14.8,15.0,15.3,15.3,15.2,14.6,14.3,13.8,13.6}   // maximum pressure
    };

  long injectorBasePulseTimes[numTableRows][numTableCols];
};

#endif
