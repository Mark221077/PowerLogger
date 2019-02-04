#ifndef _WRAPPER_UNIONS_H
#define _WRAPPER_UNIONS_H

#include <Arduino.h>

#define FLOATSIZE 4
//helper type to convert float to 4 bytes(float on atmega328 is 4 bytes)
typedef union _floatWrapper {
    float num;
    byte b[FLOATSIZE];
} FloatWrapper;

#define LONGSIZE 4
//same as above for long
typedef union _longWrapper {
    unsigned long num;
    byte b[LONGSIZE];
} LongWrapper;


#endif