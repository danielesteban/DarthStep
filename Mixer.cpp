/*
    Mixer.cpp - DarthStep mixer logic & UIView.
    Created by Daniel Esteban, January 27, 2013.
*/

#include "Mixer.h"

Mixer::Mixer(byte numSynths, Synth * synths[], Sampler * sampler) {
	_numSynths = numSynths;
	_synths = synths;
	_sampler = sampler;
	_lastFrame = 0;
	for(byte x=0;x<3; x++) addButton(NULL);
}

void Mixer::render(UTFT tft) {
	tft.clrScr();

	tft.setColor(255, 255, 255);
	tft.fillRect(0, 0, tft.getDisplayXSize() - 1, 12);
	tft.setBackColor(255, 255, 255);
	tft.setColor(0, 0, 0);
	tft.setFont(SmallFont);
	tft.print("Mixer", 10, 0);

	_tft = tft;
	for(byte x=0;x<3; x++) {
		renderedGains[x] = 0;
		renderMute(x);
	}
	update();
}

void Mixer::update() {
	unsigned long t = millis();
	if(t - _lastFrame < 50) return; //20 frames per second;
	_lastFrame = t;
	for(byte id=0;id<3; id++) {
		int gain = id == 0 ? _sampler->gain : _synths[id - 1]->gain;
		if(renderedGains[id] != gain) renderSlider(id, gain);
	}
}

void Mixer::renderSlider(byte id, int gain) {
	int w = (_tft.getDisplayXSize() - 11) / 3,
		x = 10 + (w * id),
		y = 22 + ((long) (256 - gain) * (_tft.getDisplayYSize() - 50 - 22) / 256);

	_tft.setColor(65, 65, 65);
    _tft.fillRect(x, 22, x + w - 10, y);
    _tft.setColor(225, 65, 65);
    _tft.fillRect(x, y, x + w - 10, _tft.getDisplayYSize() - 60);
    renderedGains[id] = gain;
}

void Mixer::renderMute(byte id) {
	int w = (_tft.getDisplayXSize() - 11) / 3,
		x = 10 + (w * id),
		y = _tft.getDisplayYSize() - 40;

	renderButton(id, (char *) (id == 0 ? "Sampler" : id == 1 ? "Synth 1" : "Synth 2"), x, y, w, !(id == 0 ? _sampler->mute : _synths[id - 1]->mute));
}

void Mixer::onTouch(byte orientation, int x, int y) {
	if(y < 12 || y > (_tft.getDisplayYSize() - 40) || x < 10 || x > _tft.getDisplayXSize() - 11) return;
	
	unsigned int h = (_tft.getDisplayYSize() - 50 - 22);

	y -= 22;
	y < 0 && (y = 0);
	y > h && (y = h);
	byte id = (x - 21) / ((_tft.getDisplayXSize() - 21) / 3);
	
	unsigned int val = (h - y) * 256 / h;

	switch(id) {
		case 0:
			_sampler->gain = val;
		break;
		case 1:
		case 2:
			_synths[id - 1]->gain = val;
	}
	renderSlider(id, val);
}

void Mixer::onClick(byte id) {
	switch(id) {
		case 0:
			_sampler->mute = !_sampler->mute;
		break;
		case 1:
		case 2:
			_synths[id - 1]->mute = !_synths[id - 1]->mute;
	}
	renderMute(id);
}

//Total Copy Paste from synthConfig! This should be part of UI.h
//Even should be used on Menu.h instead of all of this render functions.. Lame!
void Mixer::renderButton(byte id, char * l, int x, int y, int w, bool on) {
    UIButton * b = _buttons;
    byte i = 0;
    while(b != NULL && i < id) {
        b = b->next;
        i++;
    }
    b->x = x;
    b->y = y;
    b->width = w - 10;
    b->height = 30;

    if(on) _tft.setColor(255, 255, 255);
    else _tft.setColor(65, 65, 65);
    _tft.fillRect(x, y, x + w - 10, y + b->height);
    if(on) {
        _tft.setBackColor(255, 255, 255);
        _tft.setColor(0, 0, 0);
    } else {
        _tft.setBackColor(65, 65, 65);
        _tft.setColor(127, 127, 127);
    }
    if(l != NULL) _tft.print(l, x + 10, y + 10);
}
