// ADC_ADS1015.h

#ifndef _ADC_ADS1015_h
#define _ADC_ADS1015_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "arduino.h"
#else
  #include "WProgram.h"
#endif








//config constants, refer to datasheet

#define I2CADDR  0x48

#define CONVERSIONREGISTER  0x00
#define CONFIGREG  0x01
#define LOTHRESHREGISTER  0x02
#define HITHRESHREGISTER  0x03

#define STATUSNOEFFECT  0x00
#define STATUSSTARTSINGLECONV  0x8000

#define MUXA0A1  0x00
#define MUXA0A3  0x1000
#define MUXA1A3  0x2000
#define MUXA2A3  0x3000
#define MUXA0  0x4000
#define MUXA1  0x5000
#define MUXA2  0x6000
#define MUXA3  0x7000

#define  PGA6  0x00
#define PGA4  0x200
#define PGA2  0x400
#define PGA1  0x600
#define PGA05  0x800
#define PGA02  0xA00

#define MODECONTINUOUS  0x00
#define MODESINGLESHOT  0x100

#define DR128  0x00
#define DR250  0x20
#define DR490  0x40
#define DR920  0x60
#define DR1600  0x80
#define DR2400  0xA0
#define DR3300  0xC0

#define COMPMODETRADITIONAL  0x00
#define COMPMODEWINDOW  0x10

#define COMPPOLACTLOW  0x00
#define COMPPOLACTHIGH  0x8

#define COMPLATNON  0x00
#define COMPLATYES  0x4

#define COMPQUEUEAFTERONE  0x00
#define COMPQUEUEAFTERTWO  0x01
#define COMPQUEUEAFTERFOUR  0x10
#define COMPQUEUEDISABLEHIZ  0x11


class ADC_ADS1015
{

public:

  ADC_ADS1015();
  ~ADC_ADS1015();

  double voltPerBit;    //used to calculate the measured voltage

  void startContinuous(uint16_t mux, uint16_t pga, uint16_t dataRate);    //starts the adc in continuous mode

  int readValue();      //returns the reading in bytes

  double readVolts();     //returns the reading in volts
};


#endif


