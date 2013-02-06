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

Sequencer::Sequencer(byte numSequencableUIs, SequencableUI * sequencableUIs[]) : Menu("Sequencer", 3, new String, menuOnClick) {
    _tempoStep = 0;
    _tempoStepFloat = 0;
    setTempo(120);
    _numSequencableUIs = numSequencableUIs;
    _sequencableUIs = sequencableUIs;
    setLabel(2, "Clear Sequence", false);
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
            if(sdStatus) {
                Directory * dir = new Directory(UIView == _numSequencableUIs - 1 ? "/SAMPLER" : "/SYNTH");
                file * f = dir->getFiles();

                byte count = 0,
                    c = 0;

                while(f != NULL) { //this is really lame, but i'm kinda tired ;P
                    count++;
                    f = f->next;
                }
                
                String filenames[count];
                
                f = dir->getFiles();
                while(f != NULL) {
                    filenames[c] = f->name;
                    c++;
                    f = f->next;
                }

                delete dir;

                const byte UIViewSequenceLoader = 8; //FAIL
                if(UIViews[UIViewSequenceLoader] != NULL) delete UIViews[UIViewSequenceLoader];
                UIViews[UIViewSequenceLoader] = new Menu("Load sequence", count, filenames, sequenceLoaderOnClick);
                setUIView(UIViewSequenceLoader);
            }
        break;
        case 1:
            if(sdStatus) renderKeyboard(keyboardSaveCallback, 8);
        break;
        case 2:
            _sequencableUIs[UIView]->clearSequence();
            setUIView(UIView);
    }
}

void Sequencer::sequenceLoaderOnClick(byte id) {
    Directory * dir = new Directory(UIView == _numSequencableUIs - 1 ? "/SAMPLER" : "/SYNTH");
    file * f = dir->getFiles();

    byte c = 0;

    while(f != NULL) {
        if(c == id) break;
        c++;
        f = f->next;
    }

    String path = UIView == _numSequencableUIs - 1 ? "/SAMPLER/" : "/SYNTH/";
    path += f->name;
    delete dir;
    char p[path.length()];
    path.toCharArray(p, path.length() + 1);
    _sequencableUIs[UIView]->loadSequence(p);
    setUIView(UIView);
}

void Sequencer::keyboardSaveCallback(String str) {
    str = (UIView == _numSequencableUIs - 1 ? "/SAMPLER/" : "/SYNTH/") + str + ".SEQ";
    char p[str.length()];
    str.toCharArray(p, str.length() + 1);
    _sequencableUIs[UIView]->saveSequence(p);
    setUIView(UIView);
}

void Sequencer::updateSdStatus() {
    setLabel(0, !sdStatus ? "NO SDCARD" : "Load Sequence", rendered);
    setLabel(1, !sdStatus ? "NO SDCARD" : "Save Sequence", rendered);
}
