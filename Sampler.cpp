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
		sampleQuantization[x] = 8;
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

		_renderedTempoStep = _renderedQuantization = sampleQuantization[selectedSample];
		_step_h = 40;
		_step_w = _tft.getDisplayXSize() / _renderedQuantization;
		_step_y = ((_tft.getDisplayYSize() - 1) / 2) - (_step_h / 2);
		_step_m = (_tft.getDisplayXSize() - (_step_w * _renderedQuantization)) / 2;
		_tft.setColor(0, 0, 0);
		_tft.fillRect(0, _step_y - 2, _tft.getDisplayXSize() - 1, _step_y + _step_h + 3);
		for(x=0; x<_renderedQuantization; x++) {
			renderStep(x);
			renderStepSelection(x);
		}

		int qx = _tft.getDisplayXSize() - 160,
			qy = _tft.getDisplayYSize() - 17;

		_tft.setColor(0, 0, 0);
		_tft.fillRect(qx, qy, _tft.getDisplayXSize() - 1, _tft.getDisplayYSize() - 1);
		_tft.setBackColor(0, 0, 0);
		_tft.setColor(255, 255, 255);
		String qt = "";
		qt += _renderedQuantization / 2;
		qt += "th notes";
		_tft.print(qt, qx, qy);
	}

	if(_tempoStep != _renderedTempoStep) {
		if(_renderedTempoStep != _renderedQuantization) renderStep(_renderedTempoStep);
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
	_tempoStep = tempoStep * _renderedQuantization / numTempoSteps;
}

void Sampler::renderStep(byte step, bool active) {
	if(active) _tft.setColor(0, 0, 255);
	else _tft.setColor(255, 255, 255);
	int x = _step_m + (step * _step_w);
	_tft.drawRoundRect(x, _step_y, x + _step_w - 3, _step_y + _step_h);
}

void Sampler::renderStepSelection(byte step) {
	bool selected = _sequencerSteps[selectedSample][step * numTempoSteps / _renderedQuantization];
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
	byte step = id * numTempoSteps / _renderedQuantization;

	_sequencerSteps[selectedSample][step] = !_sequencerSteps[selectedSample][step];
	renderStepSelection(id);
}

void Sampler::onTouchEnd() {
	_lastTouch = 255;
}
