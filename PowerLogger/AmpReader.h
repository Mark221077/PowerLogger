// AmpReader.h

#ifndef _AMPREADER_h
#define _AMPREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#include "ADC_ADS1015.h"


#ifndef VOLTPERAMP
#define VOLTPERAMP  0.148730963
#endif // !VOLTPERAMP

#ifndef ZEROAMPOFFSET
#define ZEROAMPOFFSET (-0.00095933)      //the average value of readings in volts
#endif // !ZEROAMPOFFSET

#ifndef MAXALLOWEDREADING
#define MAXALLOWEDREADING 1800    //at which value should we increase the pga
#endif // !MAXALLOWEDREADING

#ifndef MINALLOWEDREADING
#define MINALLOWEDREADING ( MAXALLOWEDREADING / 2 )     //at which value should we decrease the pga
#endif // !MINALLOWEDREADING


#ifndef TIMETORESET
#define TIMETORESET   5000        //the time offset to decrease the PGA
#endif // !TIMETORESET



class AmpReader
{
private:
  int lastMax;
  uint16_t pgaConfig;
  unsigned long resetTime;
  void decreasePGA();
  void increasePGA();

public:
  double readAmp();

  ADC_ADS1015 adc;
  void init();
};

#endif


