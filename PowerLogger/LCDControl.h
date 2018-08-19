// LCDControl.h

#ifndef _LCDCONTROL_h
#define _LCDCONTROL_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#include <LiquidCrystal_I2C.h>

//removes flicker and the need for clear()
class LCDControl {
  
private:
  String line1;
  String line2;

public:
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
  void setLine1(String line);
  void setLine2(String line);

};

#endif


