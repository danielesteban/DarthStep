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
#include <Directory.h>
#include "Samples.h"

class Sampler : public UI {
    public:
        Sampler(Midi midi);
        void render(UTFT tft);
        void update();
        void loadSamples();
        int output();
        void sequencerTick(byte tempoStep);
        void renderStep(byte step, bool active = false);
        void renderStepSelection(byte step);
        byte selectedSample,
            sampleQuantization[numSamples];
    private:
        UTFT _tft;
		Midi _midi;
		byte _renderedSample,
			_tempoStep,
            _renderedTempoStep,
            _renderedQuantization,
            _step_n,
            _step_w,
			_step_h,
			_step_y,
			_step_m,
            _lastTouch;

        unsigned int _sampleIndex[numSamples],
            _sampleOn;

		static const byte numTempoSteps = 64; //This should be Sequencer::numTempoSteps

		bool _sequencerSteps[numSamples][numTempoSteps];

        void onTouch(byte orientation, int x, int y);
        void onTouchEnd();
};
 
#endif
