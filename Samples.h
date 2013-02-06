#ifndef Samples_h
#define Samples_h

#define DEBUG (1)

#include "Arduino.h"
#include <avr/pgmspace.h>

const unsigned int sampleSize = 5000;

#ifdef DEBUG
	const byte numSamples = 1,
		samplesMidi[numSamples] = {36};

	const String sampleNames[numSamples] = {"BASSDRUM"};
#else
	const byte numSamples = 9,
		samplesMidi[numSamples] = {36, 40, 37, 44, 46, 48, 41, 51, 55};

	const String sampleNames[numSamples] = {"BASSDRUM", "SNARE", "STICK", "HI-HAT CLOSED", "HI-HAT OPEN", "HIGH TOM", "FLOOR TOM", "RAID", "CRASH"};
#endif

extern const char * samples[numSamples];

#endif
