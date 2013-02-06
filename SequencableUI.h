/*
    SequencableUI.h - Library for interfacing the sequencer with UIViews.
    Created by Daniel Esteban, February 5, 2013.
*/

#ifndef SequencableUI_h
#define SequencableUI_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif

class SequencableUI {
    public:
        SequencableUI();
        virtual void sequencerTick(byte tempoStep) = 0;
        virtual void clearSequence() = 0;
        virtual void loadSequence(char * path) = 0;
        virtual void saveSequence(char * path) = 0;
    private:
        
};

#endif
