#ifndef Samples_h
#define Samples_h

#include "Arduino.h"
#include <avr/pgmspace.h>

const unsigned int sampleSize = 5000;

const byte numSamples = 9;

const String sampleNames[numSamples] = {"BASSDRUM", "SNARE", "STICK", "HI-HAT CLOSED", "HI-HAT OPEN", "HIGH TOM", "FLOOR TOM", "RAID", "CRASH"};

extern const char * samples[numSamples];

#endif
