// AmpReader.h

#ifndef _AMPREADER_h
#define _AMPREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "ADC_ADS1015.h"


#define VOLTPERAMP  0.13830963

#define ZEROAMPOFFSET (-0.005571471)      //the average value of readings in volts




class AmpReader
{

public:

	ADC_ADS1015 adc;
	double readAmp();

	void init();
};

#endif


