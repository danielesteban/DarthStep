/*
    Sequencer.cpp - DarthStep Sequencer logic.
    Created by Daniel Esteban, January 3, 2013.
*/

#include "Sequencer.h"

byte Sequencer::UIView;

byte Sequencer::_numSequencableUIs,
    Sequencer::_tempoStep;

unsigned int Sequencer::_tempoBpm;
float Sequencer::_tempoStepFloat,
    Sequencer::_tempoIncrement;

SequencableUI ** Sequencer::_sequencableUIs;

String Sequencer::_menuItems[_numMenuItems] = {NULL, NULL, "Clear Sequence"};

Sequencer::Sequencer(byte numSequencableUIs, SequencableUI * sequencableUIs[]) : Menu("Sequencer", _numMenuItems, _menuItems, menuOnClick) {
    _tempoStep = 0;
    _tempoStepFloat = 0;
    setTempo(120);
    _numSequencableUIs = numSequencableUIs;
    _sequencableUIs = sequencableUIs;
}

void Sequencer::tick() {
    _tempoStepFloat += _tempoIncrement;
    if(_tempoStep != (int) _tempoStepFloat) {
        _tempoStep = (int) _tempoStepFloat;
        _tempoStep >= numTempoSteps && (_tempoStepFloat = _tempoStep = 0);
        for(byte x=0; x<_numSequencableUIs; x++) _sequencableUIs[x]->sequencerTick(_tempoStep);
    }
}

void Sequencer::setTempo(unsigned int tempoBpm) {
    _tempoBpm = tempoBpm;
    _tempoIncrement = (((float) numTempoSteps / 8) / (float) rate) * ((float) _tempoBpm / 60);
}

void Sequencer::menuOnClick(byte id) {
    switch(id) {
        case 0:
            if(sdStatus) renderFileBrowser("Sequence Loader", UIView == _numSequencableUIs - 1 ? "/SAMPLER" : "/SYNTH", fileBrowserCallback);
        break;
        case 1:
            if(sdStatus) renderKeyboard(keyboardSaveCallback, 8);
        break;
        case 2:
            _sequencableUIs[UIView]->clearSequence();
            setUIView(UIView, true);
    }
}

void Sequencer::fileBrowserCallback(String str) {
    char p[str.length() + 1];
    str.toCharArray(p, str.length() + 1);
    _sequencableUIs[UIView]->loadSequence(p);
    setUIView(UIView, true);
}

void Sequencer::keyboardSaveCallback(String str) {
    str = (UIView == _numSequencableUIs - 1 ? "/SAMPLER/" : "/SYNTH/") + str + ".SEQ";
    char p[str.length() + 1];
    str.toCharArray(p, str.length() + 1);
    _sequencableUIs[UIView]->saveSequence(p);
    setUIView(UIView, true);
}

void Sequencer::updateSdStatus() {
    setLabel(0, !sdStatus ? "NO SDCARD" : "Load Sequence", rendered);
    setLabel(1, !sdStatus ? "NO SDCARD" : "Save Sequence", rendered);
}
