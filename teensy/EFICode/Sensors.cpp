#include "Controller.h"

#include "Arduino.h"
#include "math.h"

// timepassed is in microseconds
long Controller::getRPM (long int timePassed, int rev) {
  return (60 * 1E6 * rev) / (timePassed);
}
