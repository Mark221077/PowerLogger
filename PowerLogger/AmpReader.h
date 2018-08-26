// AmpReader.h

#ifndef _AMPREADER_h
#define _AMPREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#include "ADC_ADS1015.h"


#define VOLTPERAMP  0.148730963

#define ZEROAMPOFFSET (-0.00095933)      //the average value of readings in volts

#define TIMETORESET   5000        //the time offset to decrease the PGA



class AmpReader
{
private:
	ADC_ADS1015 adc;

public:
  double readAmp();

  void init();
};

#endif


