// VoltReader.h

#ifndef _VOLTREADER_h
#define _VOLTREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#ifndef PEAKVALUE
#define PEAKVALUE 325     //the peak on the AC line
#endif // !PEAKVALUE

#ifndef FREQUENCY
#define FREQUENCY 50      //the frequency of the AC line in Hz
#endif // !FREQUENCY



class VoltReader
{
private:
  long zeroCrossT = 0;
  boolean skipNow = false;
  int lastVoltage = 0;        //default value, not yet set

public:
  int getReading();         //returns the current votlage level
  void zeroCrossDetected();     //call this method when zero crossing is detected
  int zeroCrossPW = 540;
};

#endif


