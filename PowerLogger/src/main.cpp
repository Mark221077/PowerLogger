#include <Arduino.h>

//#define USELCD 		1
#define LOGWIFI 	1
//#define LOGSD		1



#include "VoltReader.h"
#include "AmpReader.h"
#include "ADC_ADS1015.h"




// code for SD card
#ifdef LOGSD
#include <SPI.h>
#include <RTClib.h>
#include <SdFat.h>

RTC_DS3231 rtc;           //control for the rtc module

#define SDWRITERATE 2000    //every 2 seconds, try to increase if unstable
#define SDFLUSHCOUNT  4     //how often flush to sd

#define CSPIN 4       //for the SD card adapter

//kept it from the sd card library
#define error(msg) sd.errorHalt(F(msg))


SdFat sd;             //for the logging on SD
SdFile file;

int writeNow = 0;         //counter for the flush to SD

void logData();           //log to sd card

unsigned long sdMillis = 0;

#endif

#ifdef LOGWIFI

#define STARTSEQ	(010101L)	//indicates the start of communication between the ESP and Arduino

#define WIFISENDRATE	5000		//timeout for sending data over wifi
unsigned long wifiMillis = 0;

#define FLOATSIZE	4
//helper type to convert float to 4 bytes(float on atmega328 is 4 bytes)
typedef union _floatWrapper {
	float num;
	byte b[FLOATSIZE];
} FloatWrapper;

#define LONGSIZE	4
//same as above for long
typedef union _longWrapper {
	unsigned long num;
	byte b[LONGSIZE];
} LongWrapper;

FloatWrapper fWrapper;
LongWrapper lWrapper;

#endif


#ifdef USELCD
#include "LCDControl.h"
#include "Smoother.h"

#define LCDUPDATERATE 500


LCDControl lcdControl;
Smoother smoother;

void updateLCD();

unsigned long  lcdMillis = 0;


#endif


//in milliseconds
#define SAMPLETIME  100



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

float sumOfReadings = 0, powerConsumed = 0, currPower = 0, lastSent = 0;     //self explaining, see below
long totalReadings = 0;

int halfCyclesCount = 0;
long rmsReadingCount = 0;

float rmsA = 0, readingA, readingV, rmsSum = 0, avgCurrent = 0, currentSum = 0;      //for calculating the RMS values

void setup()
{
	pinMode(ZEROCROSSPIN, INPUT);


	ampreader.init();         //initiate the ampreader and lcd

#ifdef USELCD
	lcdControl.lcd.begin();
	lcdControl.lcd.backlight();
#endif

	//measure zero cross pulse width
	while (pulseIn(ZEROCROSSPIN, HIGH) == 0) {   //wait till AC is connected
		//if lcd connected msg to user, otherwise just wait
		#ifdef USELCD
		lcdControl.setLine1("Connect AC");
		#endif
	}

#ifdef USELCD
	lcdControl.setLine1(F("Calibrating...."));      //msg for the user
	lcdControl.setLine2(F("Please wait"));
#endif

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


#ifdef LOGSD
	if (!sd.begin(CSPIN, SD_SCK_MHZ(16))) {     //init the sd
		sd.initErrorHalt();
		#ifdef USELCD
		lcdControl.setLine1(F("SD Error"));
		#endif
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
			#ifdef USELCD
			lcdControl.setLine1(F("SD Error Cant"));
			lcdControl.setLine2(F("create filename"));
			error("Can't create file name");
			#endif
			while (1);      //halt
		}
	}

	if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
		#ifdef USELCD
		lcdControl.setLine1("SD FILE ERR");
		lcdControl.setLine2("");
		#endif
		error("file.open");
		while (1);   //halt
	}
	#ifdef USELCD
	lcdControl.setLine1(F("Logging to: "));
	lcdControl.setLine2(String(fileName));    //msg to user
	#endif

#endif

#ifdef LOGWIFI

	Serial.begin(115200);		//ESP8266 connected through Serial port

#endif

	delay(2000);


	attachInterrupt(digitalPinToInterrupt(ZEROCROSSPIN), zeroCrossDetected, RISING);    //attach the interrupt

	sampleMillis = millis();

#ifdef USELCD
	lcdMillis = millis();
#endif

#ifdef LOGSD
	sdMillis = millis();
#endif

#ifdef LOGWIFI
	wifiMillis = millis();
#endif

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

	
	if(0){}		//always false, but needed for easy exclusion of LCD, allows the use of else if
#ifdef USELCD
	//update LCD
	else if (millis() - lcdMillis > LCDUPDATERATE) {
		rmsA = sqrt(rmsSum / rmsReadingCount);    //RMS current, because we have space on the LCD   :)
		rmsSum = 0;
		rmsReadingCount = 0;
		updateLCD();
		lcdMillis = millis();
	}
#endif

#ifdef LOGSD
	//write to file
	else if (millis() - sdMillis > SDWRITERATE) {
		logData();
		++writeNow;   //see below
		sdMillis = millis();
	}
	else if (writeNow >= 3) {   //fill the buffer first(512kB), and write then
		writeNow = 0;
		if (!file.sync() || file.getWriteError()) {
			#ifdef USELCD
			lcdControl.setLine1(F("Write error"));
			lcdControl.setLine2(" ");
			#endif
			error("write error");
			while (1);
		}
	}
#endif

#ifdef LOGWIFI
	else if(millis() - wifiMillis > WIFISENDRATE ) {

		//first send the startSequence
		lWrapper.num = STARTSEQ;
		Serial.write(lWrapper.b, LONGSIZE);

		//then send the unixtime
//		lWrapper.num = (unsigned long) rtc.now().unixtime();
//		Serial.write(lWrapper.b, LONGSIZE);

		//lastly send the consumption
		fWrapper.num = powerConsumed;
		Serial.write(fWrapper.b, FLOATSIZE);

		//send the delta to allow power loss recovery

		fWrapper.num = powerConsumed - lastSent; 		//calculate the delta
		Serial.write(fWrapper.b, FLOATSIZE);

		lastSent = powerConsumed;


		wifiMillis = millis();
	}
#endif
}



void zeroCrossDetected() {

	if (micros() - last < MINPERIODMILLIS) return;    //ignore, this couldnt have been a zero cross
	voltReader.zeroCrossDetected();
	++halfCyclesCount;
	last = micros();
}

#ifdef LOGSD
void logData() {
	file.print(rtc.now().unixtime());
	file.write(',');
	file.println(powerConsumed * 1000, 6);      //write to file in Wh, for better accuracy at lower numbers
}
#endif


#ifdef USELCD
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
#endif
