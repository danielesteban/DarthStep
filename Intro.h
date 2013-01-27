/*
    Intro.h - DarthStep intro UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#ifndef Intro_h
#define Intro_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>

class Intro : public UI {
    public:
        Intro(TouchEvent onTouch);
        void render(UTFT tft);
        void update();
    private:
    	UTFT _tft;
        TouchEvent _onTouch;
		unsigned long _lastFrame;
		byte _index,
			yoffset,
			xoffset;

        void onTouch(byte orientation, int x, int y);
};
 
#endif