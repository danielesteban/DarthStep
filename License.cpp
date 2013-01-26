/*
    License.cpp - DarthStep license UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#include "License.h"

prog_char licenseText1[] PROGMEM = "License:";
prog_char licenseText2[] PROGMEM = "This program is free software: you can";
prog_char licenseText3[] PROGMEM = "redistribute it and/or modify it under";
prog_char licenseText4[] PROGMEM = "the terms of the GNU General Public";
prog_char licenseText5[] PROGMEM = "License as published by the Free";
prog_char licenseText6[] PROGMEM = "Software Foundation, either version 3";
prog_char licenseText7[] PROGMEM = "of the License, or (at your option) any";
prog_char licenseText8[] PROGMEM = "later version.";
prog_char licenseText9[] PROGMEM = "This program is distributed in the hope";
prog_char licenseText10[] PROGMEM = "that it will be useful, but WITHOUT ANY";
prog_char licenseText11[] PROGMEM = "WARRANTY; without even the implied warranty";
prog_char licenseText12[] PROGMEM = "of MERCHANTABILITY or FITNESS FOR A PARTICULAR";
prog_char licenseText13[] PROGMEM = "PURPOSE. See the GNU General Public License for";
prog_char licenseText14[] PROGMEM = "more details.";

PROGMEM const char *licenseTexts[] = {
	licenseText1,
	licenseText2,
	licenseText3,
	licenseText4,
	licenseText5,
	licenseText6,
	licenseText7,
	licenseText8,
	licenseText9,
	licenseText10,
	licenseText11,
	licenseText12,
	licenseText13,
	licenseText14
};

License::License() {
	availableOrientations[PORTRAIT] = 0;
}

void License::render(UTFT tft) {
	tft.clrScr();
	tft.setFont(SmallFont);
	char buffer[64];
	byte offset = 0;
	for(byte x=0; x<14; x++) {
		strcpy_P(buffer, (char *) pgm_read_word(&(licenseTexts[x])));
		(x == 1 || x == 8) && (offset += 15);
		tft.print(buffer, 10, (x * 15) + offset);
	}
}
