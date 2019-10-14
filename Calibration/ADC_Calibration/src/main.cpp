#include <Arduino.h>

#include "AmpReader.h"
#include "ADC_ADS1015.h"

ADC_ADS1015 adc;


void setup() {
  // put your setup code here, to run once:
  adc.startContinuous(MUXA0A1, PGA05, DR1600);

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  long sum = 0;
  int count = 1000;
  for(int i = 0; i < count; ++i) {
    sum += adc.readVolts()*1000;
    delayMicroseconds(500);
  }

  Serial.print((int) (1.0*sum/count) );
  Serial.println(" mV");
}