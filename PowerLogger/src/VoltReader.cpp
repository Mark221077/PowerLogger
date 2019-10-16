// 
// 
// 

#include "VoltReader.h"

// MAGIC = 2*pi*f/1000000  - where f=50Hz and 1000000 converts microseconds to seconds
#define MAGIC 0.000314159265358979323846264338327950288419716939937510582

int VoltReader::getReading()
{
  //the current voltage on AC line
  return  (int) PEAKVALUE * sin(MAGIC * (micros() - zeroCrossT));
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
