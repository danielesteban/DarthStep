/*
    Synth.h - DarthStep Synth logic & UIView.
    Created by Daniel Esteban, December 29, 2012.
*/

#ifndef Synth_h
#define Synth_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>
#include "SequencableUI.h"
#include "Sequencer.h"
#include <Wave.h>
#include <Midi.h>
#include <SD.h>    

typedef struct synthSequence { 
    byte note;
    unsigned int gain;
    byte chainSawInterval;
    //int circle[2];
} synthSequence;

class Synth : public UI, public SequencableUI {
    public:
        Synth(int sampleRate, Midi midi, byte midiChannel);
        void render(UTFT tft);
        void update();
        void setScale(byte id);
        void setOctave(byte id);
        int output();
        void midiToggle();
        void accelerometer(int x, int y, int z);
        void photoResistor(int read, unsigned int min, unsigned int max);
        void sequencerTick(byte tempoStep);
        void clearSequence();
        void saveSequence(char * path);
        void loadSequence(char * path);

        static const byte numWaves = 4,
            numNotes = 7,
            numOctaves = 7,
            numScales = 13;

        byte waveOn,
            sequencerStatus,
            axis[6];

        unsigned int gain,
            waveGain[numWaves];

        bool mute;

        Wave * waves[numWaves];
    private:
        UTFT _tft;
		Midi _midi;

        static const byte _sampleBits = 8;

        byte _note, 
            _midiChannel, 
            _selectedOctave,
            _selectedScale,
            _selectedRoot,
            _waveNoteOffset[numWaves],
            _renderedNote,
            _renderedScale,
            _chainSawInterval;

        bool _touching, 
            _chainSaw,
            _midiEnabled;
            
		int _output,
            _circle[2],
            _renderedCircle[2];

        unsigned int _scale[numOctaves * numNotes];

        unsigned long _chainSawLastLoop;

        float _timelineW;

        static const byte numTempoSteps = Sequencer::numTempoSteps;

        synthSequence _sequencerSteps[numTempoSteps];

        byte _tempoStep,
            _renderedTempoStep;

        void setNote(byte nt);
        void setScale(byte id, byte root);
        void onTouch(byte orientation, int x, int y);
        void onTouchEnd();
        void renderCircle();
		void renderNote();
        void renderTimeline();
        void renderScale(bool force = false);
        void chainSawTick();
};
 
#endif
