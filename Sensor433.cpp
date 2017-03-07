#include "Sensor433.h"
#include "Arduino.h"

#include "RCSwitch.h"

namespace Sensor433
{

  
//
// Transmitter class
//

Transmitter::Transmitter(byte transmitterPin) : seq(0)
{
  rc.enableTransmit(transmitterPin); 
  rc.setRepeatTransmit(25);      
}

void Transmitter::sendWord(byte sensorId, word data)
{
  unsigned long dataToSend = encode32BitsToSend(sensorId, seq, data);

  // Send the data twice to reduce risk of receiver getting noise
  rc.send(dataToSend, 32);
  delay(1000);
  rc.send(dataToSend, 32);
  
  seq++;
  if (seq > 15)
  {
    seq = 0;
  }  
}

void Transmitter::sendFloat(byte sensorId, float data)
{
  word twoBytes = encodeFloatToTwoBytes(data);
  sendWord(sensorId, twoBytes);
}

unsigned long Transmitter::encode32BitsToSend(byte sensorId, byte seq, word data)
{
    byte checkSum = sensorId + seq + data;
    byte byte3 = ((0x0F & sensorId) << 4) + (0x0F & seq);
    word byte2_and_byte_1 = 0xFFFF & data;
    byte byte0 = 0xFF & checkSum;
    unsigned long dataToSend = (byte3 << 24) + (byte2_and_byte_1 << 8) + byte0;

    return dataToSend;
}

word Transmitter::encodeFloatToTwoBytes(float floatValue)
{
  bool sign = false;
  
  if (floatValue < 0) 
    sign=true;  
      
  int integer = (100*fabs(floatValue));
  unsigned int word = integer & 0XFFFF;
  
  if (sign)
    word |= 1 << 15;

  return word;
}


//
// Receiver class
//

unsigned long prevValue = 0;
int numIdenticalInRow = 0;
ReceivedMessage receivedMessage = { 0, 0, 0.0 };

Receiver::Receiver(byte receiverPin)
{
  rc.enableReceive(receiverPin);
}

ReceivedMessage Receiver::getData()
{
  return receivedMessage;
}

bool Receiver::hasNewData()
{
  if (!rc.available())
    return false;
    
  unsigned long newValue = rc.getReceivedValue();
  if (newValue == 0)
    return false;

  if (newValue == prevValue)
  {
    numIdenticalInRow++;
  }
  else
  {
    numIdenticalInRow = 0;
  }

  // Get the different parts of the 32-bit / 4-byte value
  // that has been read over 433MHz
  byte checksum = newValue & 0x000000FF;
  word sensordata = (newValue >> 8) & 0x0000FFFF;
  byte byte3 = (newValue >> 24) & 0x000000FF;
  byte seq = byte3 & 0x0F;
  byte sensorId = (byte3 & 0xF0) >> 4;

  byte calculatedCheckSum = 0xFF & (sensorId + seq + sensordata);
  
  if ((calculatedCheckSum == checksum) && (seq <= 15))
  {
    prevValue = newValue;

    // Require at least two readings of the same value in a row
    // to reduce risk of reading noise. Ignore any further duplicated
    // values
    if (numIdenticalInRow == 2)
    {
      receivedMessage.sensorId = sensorId;
      receivedMessage.dataAsWord = sensordata;
      receivedMessage.dataAsFloat = decodeTwoBytesToFloat(sensordata);
    }
  }

  return true;
}

float Receiver::decodeTwoBytesToFloat(word twoBytes)
{
  bool sign = false;
  
  if ((twoBytes & 0x8000) == 0x8000) 
    sign=true;  

  float fl = (twoBytes & 0x7FFF) / 100.0;
        
  if (sign)
    fl = -fl;

  return fl;
}

}

