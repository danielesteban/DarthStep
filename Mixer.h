/*
    Mixer.h - DarthStep mixer logic & UIView.
    Created by Daniel Esteban, January 27, 2013.
*/

#ifndef Mixer_h
#define Mixer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>
#include "Synth.h"
#include "Sampler.h"

class Mixer : public UI {
    public:
        Mixer(byte numSynths, Synth * synths[], Sampler * sampler);
        void render(UTFT tft);
        void update();
    private:
  		UTFT _tft;
  		byte _numSynths;
  		Synth ** _synths;
        Sampler * _sampler;
      
      unsigned long _lastFrame;

      unsigned int renderedGains[3];

  		void renderSlider(byte id, int gain);
  		void renderMute(byte id);
  		void renderButton(byte id, char * l, int x, int y, int w, bool on);

      void onTouch(byte orientation, int x, int y);
  		void onClick(byte id);
};
 
#endif
