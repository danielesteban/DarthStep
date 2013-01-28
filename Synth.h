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
#include <Wave.h>
#include <Midi.h>
#include <SD.h>    

typedef struct synthSequence { 
    byte note;
    unsigned int gain;
    byte chainSawInterval;
    int circle[2];
} synthSequence;

class Synth : public UI {
    public:
        Synth(int sampleRate, Midi midi, byte midiChannel);
        void render(UTFT tft);
        void update();
        void setScale(byte id);
        void setNote(byte nt);
        void setOctave(byte id);
        int output();
        void chainSawTick();
        void midiToggle();
        void sequencerTick(byte tempoStep);
        void clearSequencer();
        void saveSequence();
        void loadSequence(String name);

        static const byte numWaves = 4,
            numNotes = 7,
            numOctaves = 7,
            numScales = 13;

        byte note,
            selectedNote,
            waveOn,
            sequencerStatus,
            axis[5];

        unsigned int gain,
            waveGain[numWaves];

        bool mute;

        Wave * waves[numWaves];
    private:
        UTFT _tft;
		Midi _midi;

        static const byte _sampleBits = 8;

        byte _midiChannel, 
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

        static const byte numTempoSteps = 64; //This should be Sequencer::numTempoSteps

        synthSequence _sequencerSteps[numTempoSteps];

        byte _tempoStep,
            _renderedTempoStep;

        void setScale(byte id, byte root);
        void onTouch(byte orientation, int x, int y);
        void onTouchEnd();
        void renderCircle();
		void renderNote();
        void renderTimeline();
        void renderScale(bool force = false);
};
 
#endif
