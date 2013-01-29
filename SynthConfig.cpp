/*
    SynthConfig.cpp - DarthStep Synth Config UIView.
    Created by Daniel Esteban, January 26, 2013.
*/

#include "SynthConfig.h"

SynthConfig::SynthConfig(Synth * synth, TouchEvent photoResistorEnable) : UI() {
    availableOrientations[PORTRAIT] = _mode = 0;
    _synth = synth;
    _photoResistorEnable = photoResistorEnable;
    byte x;
    for(x=0; x<6+(_synth->numWaves*5); x++) addButton(NULL);
}

void SynthConfig::render(UTFT tft) {
    tft.clrScr();
    tft.setFont(SmallFont);
    _tft = tft;
    byte x;
    switch(_mode) {
        case 0:
    	    for(x=0; x<2; x++) renderAxis(x);
    	    for(x=0; x<_synth->numWaves; x++) renderWave(x);
    	break;
    	case 1: //accelerometer & lightResistor
    	    for(x=2; x<6; x++) renderAxis(x);
    }
}

void SynthConfig::toggleMode() {
    _mode = _mode == 1 ? 0 : 1;
    render(_tft);
}

void SynthConfig::renderAxis(byte axis) {
    byte ax = axis > 1 ? axis - 2 : axis,
        btid = ax * 3;

    int bw = (_tft.getDisplayXSize() - 31) / 3,
        x = 30, y = (axis > 1 ? 40 : 10) + (ax * 40);

    _tft.setBackColor(0, 0, 0);
    _tft.setColor(255, 255, 255);
    _tft.print(ax == 0 ? "X" : ax == 1 ? "Y" : ax == 2 ? "Z" : "LR", 10, y + 10);
    renderButton(btid, "Pitch", x, y, bw, _synth->axis[axis] == 0);
    renderButton(btid + 1, "Gain", x + bw, y, bw, _synth->axis[axis] == 1);
    renderButton(btid + 2, "ChainSaw", x + bw + bw, y, bw, _synth->axis[axis] == 2);
}

void SynthConfig::renderWave(byte wave) {
    int bw = (_tft.getDisplayXSize() - 56) / 5,
        m = (bw - 50) / 2,
        x = 55, y = 90 + (wave * 35);
   
    byte btid = (wave * 5) + 6;

    bool on = _synth->waveOn & (1 << wave);
    byte shape = _synth->waves[wave]->getShape();

    String str = "Wave";
    str += (wave + 1);

    _tft.setBackColor(0, 0, 0);
    _tft.setColor(255, 255, 255);
    _tft.print(str, 10, y + 10);

    renderButton(btid, NULL, x, y, bw, on && shape == WaveShapeSquare);
    x += m;
    _tft.drawLine(x, y + 20, x + 10, y + 20);
    _tft.drawLine(x + 10, y + 20, x + 10, y + 10);
    _tft.drawLine(x + 10, y + 10, x + 20, y + 10);
    _tft.drawLine(x + 20, y + 10, x + 20, y + 20);
    _tft.drawLine(x + 20, y + 20, x + 30, y + 20);
    _tft.drawLine(x + 30, y + 20, x + 30, y + 10);
    _tft.drawLine(x + 30, y + 10, x + 40, y + 10);
    _tft.drawLine(x + 40, y + 10, x + 40, y + 20);
    x += bw - m;
    renderButton(btid + 1, NULL, x, y, bw, on && shape == WaveShapeTriangle);
    x += m;
    _tft.drawLine(x, y + 20, x + 5, y + 10);
    _tft.drawLine(x + 5, y + 10, x + 10, y + 20);
    _tft.drawLine(x + 10, y + 20, x + 15, y + 10);
    _tft.drawLine(x + 15, y + 10, x + 20, y + 20);
    _tft.drawLine(x + 20, y + 20, x + 25, y + 10);
    _tft.drawLine(x + 25, y + 10, x + 30, y + 20);
    _tft.drawLine(x + 30, y + 20, x + 35, y + 10);
    _tft.drawLine(x + 35, y + 10, x + 40, y + 20);
    x += bw - m;
    renderButton(btid + 2, NULL, x, y, bw, on && shape == WaveShapeSaw);
    x += m;
    _tft.drawLine(x, y + 20, x + 10, y + 10);
    _tft.drawLine(x + 10, y + 10, x + 10, y + 20);
    _tft.drawLine(x + 10, y + 20, x + 20, y + 10);
    _tft.drawLine(x + 20, y + 10, x + 20, y + 20);
    _tft.drawLine(x + 20, y + 20, x + 30, y + 10);
    _tft.drawLine(x + 30, y + 10, x + 30, y + 20);
    _tft.drawLine(x + 30, y + 20, x + 40, y + 10);
    _tft.drawLine(x + 40, y + 10, x + 40, y + 20);
    x += bw - m;
    renderButton(btid + 3, NULL, x, y, bw, on && shape == WaveShapeSine);
    x += m;
    const byte sineY[20] = {15,17,18,19,20,20,20,19,18,17,15,13,12,11,10,10,10,11,12,13};
    for(byte c=0; c<40; c++) _tft.drawPixel(x + c, y + sineY[c > 19 ? c - 20 : c]);
    x += bw - m;
    renderButton(btid + 4, "Off", x, y, bw, !on);
}

void SynthConfig::renderButton(byte id, char * l, int x, int y, int w, bool on) {
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

void SynthConfig::onClick(byte id) {
    if(_mode == 1 || id < 6) { //axis
        byte axis = id / 3;
        _mode == 1 && (axis += 2); //accelerometer & lightResistor
        id = id % 3;
        for(byte x=0;x<6; x++) {
            if(x != axis && _synth->axis[x] == id) {
                _synth->axis[x] = _synth->axis[axis];
                if((_mode == 0 && x < 2) || (_mode == 1 && x > 1)) renderAxis(x);
                if(x == 5) _photoResistorEnable(_synth->axis[x] != 255);
                break;
            }
        }
        _synth->axis[axis] = _synth->axis[axis] == id ? 255 : id;
        if(axis == 5) _photoResistorEnable(_synth->axis[axis] != 255);
        renderAxis(axis);
    } else { //waves
        id -= 6;
        byte wave = id / 5;
        _synth->waveOn |= (1 << wave);
        id = id % 5;
        if(id == 4) {
            _synth->waveOn &= ~(1 << wave);
        } else {
            _synth->waves[wave]->setShape(id);
        }
        renderWave(wave);
    }
}
