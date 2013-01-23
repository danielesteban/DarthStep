#ifndef Samples_h
#define Samples_h

#include "Arduino.h"
#include <avr/pgmspace.h>

const unsigned int sampleSize = 5000;

const byte numSamples = 9;

const String sampleNames[numSamples] = {"BDRUM", "CRASH", "HIHATC", "HIHATO", "RAID", "SNARE", "STICK", "TOMH", "TOML"};

extern const unsigned char * samples[numSamples];

#endif
