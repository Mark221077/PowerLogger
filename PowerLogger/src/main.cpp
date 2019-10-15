#include <Arduino.h>
#include "VoltReader.h"
#include "AmpReader.h"



#define WIFISENDRATE	5000		//timeout for sending data over wifi
unsigned long wifiMillis = 0;


#define HALFCYCLESTOAVERAGE   10      //after how many AC cycles average the readings, should be an odd number, more than 0

#define MINPERIODMILLIS   8000      //to avoid false trigger of zero cross pin, should be less than half the period of the AC line = 1/(2*f)*1000000 in micros


#define JOULETOKWH (2.77778e-7)

#define POWERCORRECTION (1.06)   //correct the error of the calculation


#define ZEROCROSSPIN 3

//variables
AmpReader ampreader;
VoltReader voltReader;


unsigned long last = 10000;     //to avoid false triggering of the zero cross interrupt

void zeroCrossDetected();     //zero cross ISR

unsigned long sampleMillis = 0;    //when to update

float sumOfReadings = 0, powerConsumed = 0, currPower = 0;     //self explaining, see below
long totalReadings = 0;

int halfCyclesCount = 0;
long rmsReadingCount = 0;

float rmsA = 0, readingA, readingV, rmsSum = 0, avgCurrent = 0, currentSum = 0;      //for calculating the RMS values

void setup()
{
	pinMode(ZEROCROSSPIN, INPUT);
	ampreader.init();         //initiate the ampreader and lcd


	//measure zero cross pulse width
	while (pulseIn(ZEROCROSSPIN, HIGH) == 0) { }   //wait till AC is connected

	unsigned long sum = 0;
	int total = 1000;
	int i = 0, p = 0;

	while (i < total) {
		p = pulseIn(ZEROCROSSPIN, HIGH);
		if (p > 500) {
			//real pulse
			++i;
			sum += p;
		}
	}

	voltReader.zeroCrossPW = sum / total;     //set the average width of the pulses

	//TODO: connect to wifi and server

	attachInterrupt(digitalPinToInterrupt(ZEROCROSSPIN), zeroCrossDetected, RISING);    //attach the interrupt

	sampleMillis = millis();

	wifiMillis = millis();

}

void loop()
{

	//readings must be as fast as frequent as possible
	//cant be done in timer ISR because the I2C takes too long
  //and freezes up the code

	readingA = ampreader.readAmp();
	readingV = voltReader.getReading();

	sumOfReadings += readingV * readingA * POWERCORRECTION;
	++totalReadings;

	rmsSum += readingA * readingA;
	++rmsReadingCount;

	currentSum += readingA;

	if (halfCyclesCount >= HALFCYCLESTOAVERAGE) {
		//the "long" term average is always positive
		//in our case we could have shifted the voltage by 180degs because we olny look at zero crosses
		//so the average would be negative, but in the real world its always positive
		currPower = abs(sumOfReadings / totalReadings);

		#ifdef USELCD
		smoother.makeReading(currPower);    //used for the lcd display
		#endif

		powerConsumed += currPower * (millis() - sampleMillis) / 1000.0 * JOULETOKWH;   //the consumed power is saved in kWh
		sampleMillis = millis();
		sumOfReadings = 0;
		totalReadings = 0;
		halfCyclesCount = 0;
	}

	//all these take long, so only one of them is done in one loop

	if(millis() - wifiMillis > WIFISENDRATE ) {
		//TODO: send data to server
		wifiMillis = millis();
	}
}



void zeroCrossDetected() {

	if (micros() - last < MINPERIODMILLIS) return;    //ignore, this couldnt have been a zero cross
	voltReader.zeroCrossDetected();
	++halfCyclesCount;
	last = micros();
}

