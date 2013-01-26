/*
    License.h - DarthStep license UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#ifndef License_h
#define License_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <stdlib.h>
#include <WProgram.h>
#endif
#include <UI.h>

class License : public UI {
    public:
        License();
        void render(UTFT tft);
    private:
        
};
 
#endif
