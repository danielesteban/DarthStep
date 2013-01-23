/*
    Sequencer.cpp - DarthStep Sequencer logic.
    Created by Daniel Esteban, January 3, 2013.
*/

#include "Sequencer.h"

Sequencer::Sequencer(byte numSynths, Synth * synths[], Sampler * sampler) {
    _tempoStep = 0;
    _tempoStepFloat = 0;
    setTempo(120);
    _numSynths = numSynths;
    _synths = synths;
    _sampler = sampler;
}

void Sequencer::tick() {
    _tempoStepFloat += _tempoIncrement;
    if(_tempoStep != (int) _tempoStepFloat) {
        _tempoStep = (int) _tempoStepFloat;
        _tempoStep >= numTempoSteps && (_tempoStepFloat = _tempoStep = 0);
        for(byte x=0; x<_numSynths; x++) _synths[x]->sequencerTick(_tempoStep);
        _sampler->sequencerTick(_tempoStep);
    }
}

void Sequencer::setTempo(unsigned int tempoBpm) {
    _tempoBpm = tempoBpm;
    _tempoIncrement = (((float) numTempoSteps / 8) / (float) rate) * ((float) _tempoBpm / 60);
}
