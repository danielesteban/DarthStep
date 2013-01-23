/*
    Sampler.cpp - DarthStep Sampler logic & UIView.
    Created by Daniel Esteban, January 4, 2013.
*/

#include "Sampler.h"

Sampler::Sampler(Midi midi) {
	byte x;
	_midi = midi;
	selectedSample = _sampleOn = 0;
	for(x=0; x<numSamples; x++) {
		sampleQuantization[selectedSample] = 8;
		for(byte s=0; s<numTempoSteps; s++) _sequencerSteps[x][s] = 0;
	}
}

void Sampler::render(UTFT tft) {
	tft.clrScr();
	tft.setFont(BigFont);
	_tft = tft;
	_renderedSample = numSamples;
	update();
}

void Sampler::update() {
	byte x;

	if(selectedSample != _renderedSample || sampleQuantization[selectedSample] != _renderedQuantization) {
		if(selectedSample != _renderedSample) {
			_tft.setColor(255, 255, 255);
			_tft.fillRect(0, 0, _tft.getDisplayXSize() - 1, 16);
			_tft.setBackColor(255, 255, 255);
			_tft.setColor(0, 0, 0);
			_tft.print(sampleNames[selectedSample], 0, 0);
			_renderedSample = selectedSample;
		}

		_step_h = 40;
		_step_w = _tft.getDisplayXSize() / sampleQuantization[selectedSample];
		_step_y = ((_tft.getDisplayYSize() - 1) / 2) - (_step_h / 2);
		_step_m = (_tft.getDisplayXSize() - (_step_w * sampleQuantization[selectedSample])) / 2;
		_tft.setColor(0, 0, 0);
		_tft.fillRect(0, _step_y - 2, _tft.getDisplayXSize() - 1, _step_y + _step_h + 3);
		for(x=0; x<sampleQuantization[selectedSample]; x++) {
			renderStep(x);
			renderStepSelection(x);
		}
		_renderedTempoStep = _renderedQuantization = sampleQuantization[selectedSample];

		/*UIButton * b = _buttons;
		x = 0;
		while(b != NULL) {
			b->width = _step_w - 3;
			b->height = _step_h;
			if(x >= sampleQuantization[selectedSample]) b->x = b->y = -1;
			else {
				b->x = _step_m + (x * _step_w);
				b->y = _step_y;
			}
			b = b->next;
			x++;
		}*/
	}

	if(_tempoStep != _renderedTempoStep) {
		if(_renderedTempoStep != sampleQuantization[selectedSample]) renderStep(_renderedTempoStep);
		renderStep(_tempoStep, true);
		_renderedTempoStep = _tempoStep;
	}
}

int Sampler::output() {
	int output = 0;
	for(byte x=0; x<numSamples; x++) {
		if(!(_sampleOn & (1 << x))) continue;
		if(_sampleIndex[x] == sampleSize){
			_sampleOn &= ~(1 << x);
			//if(samplerMidiEnabled) midi.sendNoteOff(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
		} else {
			output += 127 - pgm_read_byte_near(samples[x] + _sampleIndex[x]);
			//output += (long) (127 - pgm_read_byte_near(samples[SAMPLE.alt & (1 << i) ? 1 : 0][i] + SAMPLE.index[i])) * SAMPLE.gain[i];
			_sampleIndex[x]++;
		}
	}

	return output;
	//return ((long) output * (long) gain) >> _sampleBits;
}

void Sampler::sequencerTick(byte tempoStep) {
	for(byte x=0; x<numSamples; x++) {
		if(_sequencerSteps[x][tempoStep]) {
			_sampleIndex[x] = 0;
			//if(samplerMidiEnabled && SAMPLE.on & (1 << i)) midi.sendNoteOff(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
			_sampleOn |= (1 << x);
			//if(samplerMidiEnabled) midi.sendNoteOn(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
			//TODO: MIDI STUFF...
		}
	}
	_tempoStep = tempoStep * sampleQuantization[selectedSample] / numTempoSteps;
}

void Sampler::renderStep(byte step, bool active) {
	if(active) _tft.setColor(0, 0, 255);
	else _tft.setColor(255, 255, 255);
	int x = _step_m + (step * _step_w);
	_tft.drawRoundRect(x, _step_y, x + _step_w - 3, _step_y + _step_h);
}

void Sampler::renderStepSelection(byte step) {
	bool selected = _sequencerSteps[selectedSample][step * numTempoSteps / sampleQuantization[selectedSample]];
	if(selected) _tft.setColor(255, 255, 255);
	else _tft.setColor(0, 0, 0);
	int x = _step_m + (step * _step_w);
	_tft.fillRect(x + 2, _step_y + 2, x + _step_w - 5, _step_y + _step_h - 1);
}

void Sampler::onTouch(byte orientation, int x, int y) {
	if(y < _step_y || y > _step_y + _step_h || x < _step_m || x > _tft.getDisplayXSize() - 1 - _step_m) {
		_lastTouch = 255;
		return;
	}

	byte id = ((x - _step_m) / _step_w);
	if(_lastTouch == id) return;
	_lastTouch = id;
	byte step = id * numTempoSteps / sampleQuantization[selectedSample];

	_sequencerSteps[selectedSample][step] = !_sequencerSteps[selectedSample][step];
	renderStepSelection(id);
}

void Sampler::onTouchEnd() {
	_lastTouch = 255;
}
