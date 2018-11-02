// 
// 
// 

#include "VoltReader.h"


int VoltReader::getReading()
{
  //the current voltage on AC line
  return  (int) PEAKVALUE * sin(2 * PI * FREQUENCY * ((micros() - zeroCrossT) / 1000000.0));
}

void VoltReader::zeroCrossDetected()
{ //reset the counter only every second zeroCross
  //so both haves of the sine wave are calculated

  //zero cross PW is the width of the pulse in microseconds
  //the zero corss is at the centre of this pulse, so divide by 2
  if (!skipNow)         
    zeroCrossT = micros() + zeroCrossPW / 2;    

  skipNow = !skipNow;
}
