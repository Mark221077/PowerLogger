#include <Arduino.h>

#include <SPI.h>
#include <RTClib.h>
#include <SdFat.h>
#include "Smoother.h"
#include "VoltReader.h"
#include "LCDControl.h"
#include "AmpReader.h"
#include "ADC_ADS1015.h"

//in milliseconds
#define SAMPLETIME  100
#define LCDUPDATERATE 500
#define SDWRITERATE 2000    //every 2 seconds, try to increase if unstable

#define SDFLUSHCOUNT  4     //how often flush to sd

#define HALFCYCLESTOAVERAGE   10      //after how many AC cycles average the readings, should be an odd number, more than 0

#define MINPERIODMILLIS   8000      //to avoid false trigger of zero cross pin, should be less than half the period of the AC line = 1/(2*f)*1000000 in micros


#define JOULETOKWH (2.77778e-7)

#define POWERCORRECTION (1.06)   //correct the error of the calculation

#define CSPIN 4       //for the SD card adapter

#define ZEROCROSSPIN 3


//kept it from the sd card library
#define error(msg) sd.errorHalt(F(msg))



//variables
AmpReader ampreader;
VoltReader voltReader;
LCDControl lcdControl;
Smoother smoother;
RTC_DS3231 rtc;           //control for the rtc module

unsigned long last = 10000;     //to avoid false triggering of the zero cross interrupt

void zeroCrossDetected();     //zero cross ISR

void logData();           //log to sd card

void updateLCD();


SdFat sd;             //for the logging on SD
SdFile file;

int writeNow = 0;         //counter for the flush to SD


unsigned long sampleMillis = 0, sdMillis = 0, lcdMillis = 0;    //when to update

double sumOfReadings = 0, powerConsumed = 0, currPower = 0;     //self explaining, see below
long totalReadings = 0;

int halfCyclesCount = 0;
long rmsReadingCount = 0;

double rmsA = 0, readingA, readingV, rmsSum = 0, avgCurrent = 0, currentSum = 0;      //for calculating the RMS values

void setup()
{
	pinMode(ZEROCROSSPIN, INPUT);


	ampreader.init();         //initiate the ampreader and lcd
	lcdControl.lcd.begin();
	lcdControl.lcd.backlight();

	//measure zero cross pulse width
	while (pulseIn(ZEROCROSSPIN, HIGH) == 0)    //wait till AC is connected
		lcdControl.setLine1("Connect AC");

	lcdControl.setLine1(F("Calibrating...."));      //msg for the user
	lcdControl.setLine2(F("Please wait"));

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



	if (!sd.begin(CSPIN, SD_SCK_MHZ(16))) {     //init the sd
		sd.initErrorHalt();
		lcdControl.setLine1(F("SD Error"));
		while (1);      //stop
	}

	char fileName[13] = "000.txt";

	//search for an available file name
	while (sd.exists(fileName)) {
		if (fileName[2] != '9') {
			fileName[2]++;
		}
		else if (fileName[1] != '9') {
			fileName[2] = '0';
			fileName[1]++;
		}
		else if (fileName[0] != '9') {
			fileName[1] = '0';
			fileName[2] = '0';
			fileName[0]++;
		}
		else {
			lcdControl.setLine1(F("SD Error Cant"));
			lcdControl.setLine2(F("create filename"));
			error("Can't create file name");
			while (1);      //halt
		}
	}

	if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {

		lcdControl.setLine1("SD FILE ERR");
		lcdControl.setLine2("");
		error("file.open");
		while (1);   //halt
	}

	lcdControl.setLine1(F("Logging to: "));
	lcdControl.setLine2(String(fileName));    //msg to user
	delay(2000);


	attachInterrupt(digitalPinToInterrupt(ZEROCROSSPIN), zeroCrossDetected, RISING);    //attach the interrupt

	sampleMillis = millis();
	lcdMillis = millis();
	sdMillis = millis();
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

		smoother.makeReading(currPower);    //used for the lcd display

		powerConsumed += currPower * (millis() - sampleMillis) / 1000.0 * JOULETOKWH;   //the consumed power is saved in kWh
		sampleMillis = millis();
		sumOfReadings = 0;
		totalReadings = 0;
		halfCyclesCount = 0;
	}

	//all these take long, so only one of them is done in one loop

	//update LCD
	if (millis() - lcdMillis > LCDUPDATERATE) {
		rmsA = sqrt(rmsSum / rmsReadingCount);    //RMS current, because we have space on the LCD   :)
		rmsSum = 0;
		ampreader.averageOfReadings = currentSum / rmsReadingCount;
		rmsReadingCount = 0;
		updateLCD();
		lcdMillis = millis();
	}
	//write to file
	else if (millis() - sdMillis > SDWRITERATE) {
		logData();
		++writeNow;   //see below
		sdMillis = millis();
	}
	else if (writeNow >= 3) {   //fill the buffer first(512kB), and write then
		writeNow = 0;
		if (!file.sync() || file.getWriteError()) {
			lcdControl.setLine1(F("Write error"));
			lcdControl.setLine2(" ");
			error("write error");
			while (1);
		}
	}
}



void zeroCrossDetected() {

	if (micros() - last < MINPERIODMILLIS) return;    //ignore, this couldnt have been a zero cross
	voltReader.zeroCrossDetected();
	++halfCyclesCount;
	last = micros();
}


void logData() {
	file.print(rtc.now().unixtime());
	file.write(',');
	file.println(powerConsumed * 1000, 6);      //write to file in Wh, for better accuracy at lower numbers
}



void updateLCD() {
	//the first line has the current power consumption and the RMS current

	//if (rmsA < 0.05) rmsA = 0.0;        //independant from power calculation, just makes it look nicer    

	lcdControl.setLine1(String((smoother.average)) + " W  " + String(rmsA, 2) + " A");

	//if the consumed power is too low, display in Wh, otherwise in kWh, in the 2nd line
	if (powerConsumed < 0.1) {
		lcdControl.setLine2(String(powerConsumed * 1000, 3) + " Wh");
	}
	else {
		lcdControl.setLine2(String(powerConsumed, 3) + " kWh");
	}
}

