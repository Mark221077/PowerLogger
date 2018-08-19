// 
// 
// 

#include "AmpReader.h"


//used to increase the PGA range in the adc
//when the read value is above a certain limit: MAXALLOWEDREADING
void AmpReader::increasePGA() {

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

  adc.startContinuous(MUXA0A1, pgaConfig, DR1600);  //reset the adc
}

//same as above, in the other way
//the threshold is determined by MINALLOWEDREADING
void AmpReader::decreasePGA() {
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

  adc.startContinuous(MUXA0A1, pgaConfig, DR1600);
}

double AmpReader::readAmp()
{
  //substract the zero amp offset from the read value, and convert to amps
  double reading = ( adc.readVolts() - ZEROAMPOFFSET ) / VOLTPERAMP;

  int val = adc.readValue();      //to see if pga needs to be changed


  if (abs(val) > lastMax)       //if lastMax is low PGA will be lowered
    lastMax = abs(val);

  if (val > MAXALLOWEDREADING) {    //if the new value is too high, increase PGA gain
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

  return reading;
}

void AmpReader::init()
{
  adc.startContinuous(MUXA0A1, PGA05, DR1600);      //set up ADC

  pgaConfig = PGA05;            //set the current PGA config
  lastMax = 0;
  resetTime = millis();
}

