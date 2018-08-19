// 
// 
// 

#include "Smoother.h"


Smoother::Smoother() {
  for (int i = 0; i < NUMOFREADINGS; ++i)
    readings[i] = 0;    //sets all the values in the array to 0

  total = 0;
  average = 0;
  readIndex = 0;
}

void Smoother::makeReading(int reading) {

  total -= readings[readIndex];     //replace a reading in the array with the new one
  readings[readIndex] = reading;      //and recalculate total
  total += readings[readIndex];
  
  ++readIndex;      //increment index
  
  if (readIndex >= NUMOFREADINGS)   //if too large go to 0
    readIndex = 0;

  average = total / NUMOFREADINGS;    //calculate average
}

