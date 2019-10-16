// 
// 
// 

#include "ADC_ADS1015.h"
#include <Wire.h>


static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {

	Wire.beginTransmission(i2cAddress);   //start communication with the ADC

	Wire.write((uint8_t)reg);       //the register to which we will write
	Wire.write((uint8_t)(value >> 8));    //the first 8 bits
	Wire.write((uint8_t)(value & 0xFF));  //the second 8 bits

	Wire.endTransmission();       //end the communication
}


static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
	Wire.beginTransmission(i2cAddress);         //starts transmitting

	Wire.write(reg);                  //the register we want to read
	Wire.endTransmission();
	Wire.requestFrom(i2cAddress, (uint8_t)2);     //we want to read 2 8bit values

	return ((Wire.read() << 8) | Wire.read());      //combine the two 8bit values into 16bit
}


//this is used to read the conversion register faster
//make sure the last register written to/requested from was the conversion register, 0x00
static uint16_t readLastRegister(uint8_t i2cAddress) {
	Wire.requestFrom(i2cAddress, (uint8_t)2);     //we want to read 2 8bit values

	return (uint16_t)(Wire.read() << 8) | (uint16_t)Wire.read();      //combine the two 8bit values into 16bit
}


void ADC_ADS1015::startContinuous(uint16_t mux, uint16_t pga, uint16_t dr)    //sets up the adc to continuous mode
{
	uint16_t config =       //refer to datasheet
		STATUSNOEFFECT
		| mux
		| pga
		| MODECONTINUOUS
		| dr
		| COMPMODETRADITIONAL
		| COMPPOLACTHIGH
		| COMPLATNON
		| COMPQUEUEAFTERTWO;

	pgaConfig = pga;
	switch (pga)                //set the voltPerBit according to pga configuration
	{                           //voltperbit is used to calculate voltage
	case PGA6: this->voltPerBit = 0.003;
		break;
	case PGA4: this->voltPerBit = 0.002;
		break;
	case PGA2: this->voltPerBit = 0.001;
		break;
	case PGA1: this->voltPerBit = 0.0005;
		break;
	case PGA05: this->voltPerBit = 0.00025;
		break;
	case PGA02: this->voltPerBit = 0.000125;
		break;
	default:
		voltPerBit = 0.001;
	}

	writeRegister(I2CADDR, CONFIGREG, config);        //set the config register

	////set up the alert pin
	//writeRegister(I2CADDR, HITHRESHREGISTER, 0xFFFF);   //hi-thresh to 1

	//writeRegister(I2CADDR, LOTHRESHREGISTER, 0x0000);     //lo thresh to 0

	writeRegister(I2CADDR, CONVERSIONREGISTER, 0);        //set up the fast reading using readLastRegister()
}

int16_t ADC_ADS1015::readValue()          //returns bytes
{
	uint16_t res = readLastRegister(I2CADDR) >> 4;    //ignore the last 4 bits

	if (res > 0x07FF)
	{
		// negative number - extend the sign to 16th bit
		res |= 0xF000;
	}


	int16_t val = (int16_t)res;

	if (abs(val) > lastMax)       //if lastMax is low PGA will be lowered
		lastMax = abs(val);

	if (abs(val) > MAXALLOWEDREADING) {    //if the new value is too high, increase PGA gain
		increasePGA();
		lastMax = 0;
		resetTime = millis();
	}


	if (millis() - resetTime > TIMETORESET) {       //check if in the last period was any high reading
		if (lastMax < MINALLOWEDREADING) {          //if there were no huge values decrease PGA
			decreasePGA();
		}

		resetTime = millis();
		lastMax = 0;
	}

	return val;

}

//returns volts
double ADC_ADS1015::readVolts()
{
	double val = readValue() * voltPerBit;
	return val;
}



//used to increase the PGA range in the adc
//when the read value is above a certain limit: MAXALLOWEDREADING
void ADC_ADS1015::increasePGA()
{
	//set the pgaConfig to the next level
	switch (pgaConfig) {
	case PGA02: pgaConfig = PGA05;
		break;
	case PGA05: pgaConfig = PGA1;
		break;
	case PGA1: pgaConfig = PGA2;
		break;
	case PGA2: pgaConfig = PGA4;
		break;
	case PGA4: pgaConfig = PGA6;
		break;
	case PGA6: return;
	default: return;

	}

	startContinuous(MUXA0A1, pgaConfig, DR3300);  //reset the adc
}

//same as above, in the other way
//the threshold is determined by MINALLOWEDREADING
void ADC_ADS1015::decreasePGA()
{

	switch (pgaConfig) {
	case PGA02: return;
	case PGA05: pgaConfig = PGA02;
		break;
	case PGA1: pgaConfig = PGA05;
		break;
	case PGA2: pgaConfig = PGA1;
		break;
	case PGA4: pgaConfig = PGA2;
		break;
	case PGA6: pgaConfig = PGA4;
		break;
	default: return;

	}

	startContinuous(MUXA0A1, pgaConfig, DR3300);
}



ADC_ADS1015::ADC_ADS1015()
{
	Wire.begin();   //initiate Wire library
	Wire.setClock(400000);		//set clock to fast, 400kHz mode
}


ADC_ADS1015::~ADC_ADS1015()
{
}

