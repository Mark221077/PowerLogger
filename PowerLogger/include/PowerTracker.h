#pragma once

#include "VoltReader.h"
#include "AmpReader.h"

#ifndef MINPERIODMILLIS
    #define MINPERIODMILLIS   8000      //to avoid false trigger of zero cross pin, should be less than half the period of the AC line = 1/(2*f)*1000000 in micros
#endif

#ifndef HALFCYCLESTOAVERAGE
    #define HALFCYCLESTOAVERAGE   10      //after how many AC cycles average the readings, should be an odd number, more than 0
#endif

#define JOULETOKWH (2.77778e-7)

class PowerTracker;

struct PowerData {
    double rmsA;        //the averaged rms current
    double watts;       //average power consumption
    double wattHours;   //all power used
    double var;         //averaged volt-amper reactive
    double varHours;    //volt-amper reactive hours
};

class PowerTracker {
private:
    VoltReader vr;
    AmpReader ar;
    const uint8_t zcPin;

    double amp = 0;             //last read current
    double volt = 0;            //last read voltage
    double sumOfReadings = 0;   //sum of power before averaging
    double rmsASum = 0;         //sum of RMS current to average
    uint32_t totalReadings = 0; //counter of readings to average
    unsigned long lastSample = 0;   //time of last average
    uint16_t zeroCrossCnt = 0;  //to count the zerocross, to track when to average
    unsigned long lastZC = 0;   //time of last zero cross

public:
    PowerTracker(uint8_t _zcPin) : zcPin(_zcPin) {}
    PowerData data = {};
    void zeroCrossDetector() {
        if (micros() - lastZC < MINPERIODMILLIS) return;    //ignore, this couldnt have been a zero cross       
        vr.zeroCrossDetected();
	    ++zeroCrossCnt;
	    lastZC = micros();
    }
    inline void manage() {
        const unsigned long ms = millis();
        amp = ar.readAmp();
        volt = vr.getReading();

        sumOfReadings = volt*amp;
        ++totalReadings;

        rmsASum = amp*amp;

        if(zeroCrossCnt > HALFCYCLESTOAVERAGE) {
            data.watts = fabs(sumOfReadings / totalReadings);
            data.wattHours += data.watts * (ms - lastSample) / 1000.0 * JOULETOKWH;
            data.rmsA = rmsASum / totalReadings;
            data.var = data.rmsA*325.0;
            data.varHours += data.rmsA * 325.0 * (ms - lastSample) / 1000.0 * JOULETOKWH;

            lastSample = millis();
            zeroCrossCnt = 0;
        }
    }

    void init() {
        ar.init();
        pinMode(zcPin, INPUT);
        while (pulseIn(zcPin, HIGH) == 0) { }   //wait till AC is connected

        uint16_t sum = 0;
        constexpr uint16_t total = 1000;
        uint16_t i = 0;
        unsigned long p = 0;

        while (i < total) {
            p = pulseIn(zcPin, HIGH);
            if( (uint16_t)(sum+p) < sum) break;  //overflow
            if (p > 500) {
                //real pulse
                ++i;
                sum += p;
            }
        }
        vr.zeroCrossPW = sum / total;     //set the average width of the pulses
        lastSample = millis();
    }
};
