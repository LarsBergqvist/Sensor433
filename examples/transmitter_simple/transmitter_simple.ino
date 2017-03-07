#include "Sensor433.h"

Sensor433::Transmitter t = Sensor433::Transmitter(5);
#define SENSORID1 10
#define SENSORID2 12

void setup() 
{
}

void loop() {
  t.sendWord(SENSORID1,20);
  t.sendFloat(SENSORID2,22.3);
  delay(5000);
}

