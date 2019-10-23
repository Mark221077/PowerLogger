#include <Arduino.h>
#include "VoltReader.h"
#include "AmpReader.h"
#include "PowerTracker.h"


#define WIFISENDRATE	5000		//timeout for sending data over wifi
unsigned long wifiMillis = 0;


#define ZEROCROSSPIN 3




PowerTracker tracker(ZEROCROSSPIN);

void setup()
{
	tracker.init();
	//TODO: connect to wifi and server

	attachInterrupt(digitalPinToInterrupt(ZEROCROSSPIN), [](){tracker.zeroCrossDetector();}, RISING);    //attach the interrupt

	wifiMillis = millis();
}

void loop()
{
	//all these take long, so only one of them is done in one loop

	if(millis() - wifiMillis > WIFISENDRATE ) {
		//TODO: send data to server
		wifiMillis = millis();
	}
}
