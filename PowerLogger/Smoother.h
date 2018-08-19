// Smoother.h

#ifndef _SMOOTHER_h
#define _SMOOTHER_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif

#ifndef NUMOFREADINGS

#define NUMOFREADINGS 15

#endif // !NUMOFREADINGS


class Smoother {
private:
  int readings[NUMOFREADINGS];
  int total = 0;
  int readIndex = 0;

public:
  int average = 0;
  Smoother();
  void makeReading(int reading);
};

#endif


