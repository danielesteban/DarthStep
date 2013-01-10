/*
    Sampler.h - DarthStep Sampler logic & UIView.
    Created by Daniel Esteban, January 4, 2013.
*/

#ifndef Sampler_h
#define Sampler_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>
#include <Midi.h>    
#include <SD.h>    

class Sampler : public UI {
    public:
        Sampler(Midi midi);
        void render(UTFT tft);
        void update();
    private:
        UTFT _tft;
		Midi _midi;
};
 
#endif
