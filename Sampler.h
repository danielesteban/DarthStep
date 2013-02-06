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
#include "SequencableUI.h"
#include "Sequencer.h"
#include <Midi.h>    
#include <SD.h>
#include <Directory.h>
#include "Samples.h"

class Sampler : public UI, public SequencableUI {
    public:
        Sampler(Midi midi, byte midiChannel);
        void render(UTFT tft);
        void update();
        int output();
        void sequencerTick(byte tempoStep);
        void clearSequence();
        void loadSequence(char * path);
        void saveSequence(char * path);
        void midiToggle();
        void toggleSteps();
        void clearSample();
        byte selectedSample,
            sampleQuantization[numSamples];
        unsigned int gain;
        bool mute;
    private:
        UTFT _tft;
		Midi _midi;

        static const byte _sampleBits = 8;

		byte _midiChannel, 
            _renderedSample,
			_tempoStep,
            _renderedTempoStep,
            _renderedQuantization,
            _step_n,
            _step_w,
			_step_h,
			_step_y,
			_step_m,
            _lastTouch,;

        unsigned int _sampleIndex[numSamples],
            _sampleOn;

		static const byte numTempoSteps = Sequencer::numTempoSteps;

		bool _sequencerSteps[numSamples][numTempoSteps],
            _midiEnabled;

        void renderStep(byte step, bool active = false);
        void renderStepSelection(byte step);
        void onTouch(byte orientation, int x, int y);
        void onTouchEnd();
};
 
#endif
