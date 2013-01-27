/*
    Intro.cpp - DarthStep intro UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#include "Intro.h"

prog_char introText1[] PROGMEM = "      dP                  dP  dP";
prog_char introText2[] PROGMEM = "      88                  88  88";
prog_char introText3[] PROGMEM = ".d888b88.d8888b.88d888b.d8888P88d888b.";
prog_char introText4[] PROGMEM = "88'  `8888'  `8888'  `88  88  88'  `88";
prog_char introText5[] PROGMEM = "88.  .8888.  .8888        88  88    88";
prog_char introText6[] PROGMEM = "`88888P8`88888P8dP        dP  dP    dP";

prog_char introText7[] PROGMEM =  "          dP";
prog_char introText8[] PROGMEM =  "          88";
prog_char introText9[] PROGMEM =  ".d8888b.d8888P.d8888b.88d888b.";
prog_char introText10[] PROGMEM = "Y8ooooo.  88  88ooood888'  `88";
prog_char introText11[] PROGMEM = "      88  88  88.  ...88.  .88";
prog_char introText12[] PROGMEM = "`88888P'  dP  `88888P'88Y888P'";
prog_char introText13[] PROGMEM = "                      88";
prog_char introText14[] PROGMEM = "                      dP";

PROGMEM const char * introTexts[] = {
	introText1,
	introText2,
	introText3,
	introText4,
	introText5,
	introText6,
	introText7,
	introText8,
	introText9,
	introText10,
	introText11,
	introText12,
	introText13,
	introText14
};

Intro::Intro(TouchEvent onTouch) {
	availableOrientations[PORTRAIT] = 0;
	_onTouch = onTouch;
}

void Intro::render(UTFT tft) {
	tft.clrScr();
	tft.setFont(SmallFont);
	_tft = tft;
	_lastFrame = _index = 0;
	yoffset = xoffset = 10;
	for(byte x=0; x<14; x++) update();
}

void Intro::update() {
	unsigned long t = millis();
	if(_index == 14) {
		if(t - _lastFrame < 5000) return;
		_index = 0;
		yoffset = xoffset = 10;
	}

	char buffer[64];
	strcpy_P(buffer, (char *) pgm_read_word(&(introTexts[_index])));
	_tft.setColor(random(65, 256), random(65, 256), random(65, 256));
	_index == 6 && (xoffset += 30) && (yoffset += 15);
	_tft.print(buffer, xoffset, (_index * 15) + yoffset);
	for(byte y=0; y<5; y++) _tft.drawPixel(random(0, _tft.getDisplayXSize()), random(0, _tft.getDisplayYSize()));
	_index++;
	_index == 14 && (_lastFrame = t);
}

void Intro::onTouch(byte orientation, int x, int y) {
	_onTouch(255);
}