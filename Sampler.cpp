/*
    Sampler.cpp - DarthStep Sampler logic & UIView.
    Created by Daniel Esteban, January 4, 2013.
*/

#include "Sampler.h"

Sampler::Sampler(Midi midi) {
	_midi = midi;
}

void Sampler::render(UTFT tft) {
	tft.clrScr();
	tft.setFont(SmallFont);
	_tft = tft;
	
}

void Sampler::update() {
	
}