#include "Sensor433.h"

Sensor433::Receiver r = Sensor433::Receiver(0); // interrupt 0 = pin 2 on Arduino UNO

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  if (r.hasNewData())
  {
      Sensor433::ReceivedMessage message = r.getData();
      Serial.print("Sensor id: ");
      Serial.println(message.sensorId);
      Serial.print("Data as word: ");
      Serial.println(message.dataAsWord);
      Serial.print("Data as float: ");
      Serial.println(message.dataAsFloat);
  }
}


