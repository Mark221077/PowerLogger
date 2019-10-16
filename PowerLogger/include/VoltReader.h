// VoltReader.h

#ifndef _VOLTREADER_h
#define _VOLTREADER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
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
	unsigned long zeroCrossT = 0;
	bool skipNow = false;	//skip every second zero cross

public:
	double getReading();         //returns the current votlage level
	void zeroCrossDetected();     //call this method when zero crossing is detected
	uint16_t zeroCrossPW = 540;		//in microseconds
};

#endif


