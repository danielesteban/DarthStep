/*
    SynthConfig.h - DarthStep Synth Config UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#ifndef SynthConfig_h
#define SynthConfig_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>
#include "Synth.h"

class SynthConfig : public UI {
    public:
        SynthConfig(Synth * synth);
        void render(UTFT tft);
        Synth * _synth;
    private:
        UTFT _tft;

        void renderAxis(byte axis);
        void renderWave(byte wave);
        void renderButton(byte id, char * l, int x, int y, int w, bool on);
        void onClick(byte id);
};
 
#endif
