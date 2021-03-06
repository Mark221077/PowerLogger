// 
// 
// 

#include "AmpReader.h"




double AmpReader::readAmp()
{
	//substract the zero amp offset from the read value, and convert to amps
	double val = adc.readVolts() / VOLTPERAMP;

	return val;
}

void AmpReader::init()
{
	adc.startContinuous(MUXA0A1, PGA05, DR3300);      //set up ADC
}

