/*
    Sequencer.h - DarthStep Sequencer logic & UIView.
    Created by Daniel Esteban, January 3, 2013.
*/

#ifndef Sequencer_h
#define Sequencer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <Menu.h>
#include "sequencableUI.h"

extern bool sdStatus;
extern void setUIView(byte view);
extern void renderFileBrowser(String title, const char * path, StringCallback callback);
extern void renderKeyboard(StringCallback callback, byte maxLength);

class Sequencer : public Menu {
    public:
        Sequencer(byte numSequencableUIs, SequencableUI * sequencableUIs[]);
        void updateSdStatus();
        static void tick();
        static void setTempo(unsigned int tempoBpm);
        static byte UIView;
        static const byte numTempoSteps = 64;
        static const unsigned int rate = 1024;
    private:
        static byte _numSequencableUIs,
            _tempoStep;
        static unsigned int _tempoBpm;
        static float _tempoStepFloat,
            _tempoIncrement;
        static SequencableUI ** _sequencableUIs;
        static const byte _numMenuItems = 3; 
        static String _menuItems[_numMenuItems];
        static void menuOnClick(byte id);
        static void fileBrowserCallback(String str);
        static void keyboardSaveCallback(String str);
};

#endif
