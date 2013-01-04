/*
    Sequencer.h - DarthStep Sequencer logic.
    Created by Daniel Esteban, January 3, 2013.
*/

#ifndef Sequencer_h
#define Sequencer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include "Synth.h"    

class Sequencer {
    public:
        Sequencer(byte numSynths, Synth * synths[]);
        void tick();
        void setTempo(unsigned int tempoBpm);
        static const byte numTempoSteps = 64;
        static const unsigned int rate = 512;
    private:
        byte _numSynths,
            _tempoStep;
        unsigned int _tempoBpm;
        float _tempoStepFloat,
            _tempoIncrement;
        Synth ** _synths;
};

#endif
