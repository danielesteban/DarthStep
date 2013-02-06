/*
    Synth.h - DarthStep Synth logic & UIView.
    Created by Daniel Esteban, December 29, 2012.
*/

#include "Synth.h"

prog_uint16_t midiNotes[] PROGMEM = {8,9,9,10,10,11,12,12,13,14,15,15,16,17,18,19,21,22,23,24,26,28,29,31,33,35,37,39,41,44,46,49,52,55,58,62,65,69,73,78,82,87,92,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902,8372,8870,9397,9956,10548,11175,11840,12544,13290,14080,14917,15804,16744};

prog_uchar scales[(Synth::numNotes - 1) * Synth::numScales] PROGMEM = {
    2, 1, 2, 2, 1, 2, // Aeolian
    1, 2, 2, 1, 2, 2, // Locrian
    2, 2, 1, 2, 2, 2, // Ionian
    2, 1, 2, 2, 2, 1, // Dorian
    1, 2, 2, 2, 1, 2, // Phrygian
    2, 2, 2, 1, 2, 2, // Lydian
    2, 2, 1, 2, 2, 1, // Mixolydian
    2, 1, 2, 2, 2, 2, // Melodic ascending minor
    1, 2, 2, 2, 2, 2, // Phrygian raised sixth
    2, 2, 2, 2, 1, 2, // Lydian raised fifth
    2, 2, 1, 2, 1, 2, // Major minor
    1, 2, 1, 2, 2, 2, // Altered
    1, 2, 2, 2, 1, 3 // Arabic
};

const String noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"},
	scaleNames[Synth::numScales] = {"Aeolian", "Locrian", "Ionian", "Dorian", "Phrygian", "Lydian", "Mixolydia", "MAscMinor", "Raised6th", "Raised5th", "MjrMinor", "Altered", "Arabic"};

Synth::Synth(int sampleRate, Midi midi, byte midiChannel) : UI(), SequencableUI() {
	byte x;
	axis[0] = axis[4] = axis[5] = _chainSawInterval = _note = 255;
	waveOn = 1;
	_output = 0;
	gain = (1 << _sampleBits) / 4;
	for(x=0; x<numWaves; x++) {
		//waveGain[x] = 1 << _sampleBits;
 		waves[x] = new Wave(WaveShapeSquare, sampleRate);
 	}
 	axis[3] = _chainSaw = _chainSawLastLoop = _midiEnabled = _selectedRoot = _tempoStep = _touching = mute = 0;
 	axis[1] = _selectedOctave = _selectedScale = 1;
	axis[2] = 2;
	_waveNoteOffset[0] = 0;
	_waveNoteOffset[1] = 7;
	_waveNoteOffset[2] = 12;
	_waveNoteOffset[3] = 16;
 	_midi = midi;
 	_midiChannel = midiChannel;
 	clearSequence();
 	setScale(_selectedScale, _selectedRoot);
	_circle[0] = _circle[1] = -1;
}

void Synth::setScale(byte id) {
	if(_selectedScale == id) return;
	setScale(id, _selectedRoot);
}

void Synth::setScale(byte id, byte root) {
	const byte offset = (numNotes - 1) * id;
	byte octave, nt,
		n[numNotes],
		cn = _note,
		c = 0;

	_selectedScale = id;
	_selectedRoot = root;
	root += 24;
	if(sequencerStatus == 0 && _midiEnabled && cn != 255) setNote(255);
	for(octave = 0; octave < numOctaves; octave++) {
		for(nt = 0; nt < numNotes; nt++) {
			if(octave == 0) n[nt] = nt == 0 ? root : n[nt - 1] + pgm_read_byte_near(scales + (offset + (nt - 1)));
			else n[nt] += 12;
			_scale[c] = n[nt];
			c++;
		}
	}
	if(sequencerStatus == 0 && cn != 255) setNote(_note);
}

void Synth::setNote(byte nt) {
	if(_note == nt) return;
	if(_midiEnabled && _note != 255) {
		for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOff(_scale[_note] + _waveNoteOffset[x], map(/*waveGain[x]*/ gain, 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	}
	_note = nt;
	if(nt == 255) return;
	for(byte x=0; x<numWaves; x++) {
		waves[x]->setFrequency(pgm_read_word_near(midiNotes + (_scale[_note] + _waveNoteOffset[x])));
		if(_midiEnabled && waveOn & (1 << x)) _midi.sendNoteOn(_scale[_note] + _waveNoteOffset[x], map(/*waveGain[x]*/ gain, 0, 1 << _sampleBits, 0, 127), _midiChannel);
	}
}

void Synth::setOctave(byte id) {
	if(_selectedOctave == id) return;
	const byte noteIndex = _note - (_selectedOctave * numNotes);
	_selectedOctave = id;
	if(_note != 255) {
		_note != 255 && (_note = (_selectedOctave * numNotes) + noteIndex);
		if(sequencerStatus == 0) setNote(_note);
	}
}

int Synth::output() {
	if(!mute && !_chainSaw && _note != 255) {
		_output = 0;
		for(byte i=0; i<numWaves; i++) {
			if(!(waveOn & (1 << i))) continue;
			//_output += ((long) (127 - waves[i]->next()) * waveGain[i]) >> _sampleBits;
			_output += 127 - waves[i]->next();
		}
	} else if(_output != 0) {
		_output += _output > 0 ? -1 : 1;
	}

	return ((long) _output * (long) gain) >> _sampleBits;
}

void Synth::chainSawTick() {
	if(_chainSawInterval == 255 || !waveOn || _note == 255) return;
	unsigned long t = millis();
	if(t >= _chainSawLastLoop + _chainSawInterval) {
		_chainSawLastLoop = t;
		_chainSaw = false;
		if(_midiEnabled && _note != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOn(_scale[_note] + _waveNoteOffset[x], map(/*waveGain[x]*/ gain, 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	} else if(!_chainSaw && t >= _chainSawLastLoop + (_chainSawInterval / (_chainSawInterval > 250 ? 4 : _chainSawInterval > 50 ? 3 : 2))) {
		_chainSaw = true;
		if(_midiEnabled && _note != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOff(_scale[_note] + _waveNoteOffset[x], map(/*waveGain[x]*/ gain, 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	}
}

void Synth::midiToggle() {
	_midiEnabled = !_midiEnabled;
	if(!_midiEnabled && !_chainSaw && _note != 255) for(byte x=0; x<numWaves; x++) {
		if(!(waveOn & (1 << x))) continue;
		_midi.sendNoteOff(_scale[_note] + _waveNoteOffset[x], map(/*waveGain[x]*/ gain, 0, 1 << _sampleBits, 0, 127), _midiChannel);
	}
}

void Synth::accelerometer(int x, int y, int z) {
	if(!_touching) return;
	const int min = 180, max = 480;
	if(axis[2] == 0 || axis[3] == 0 || axis[4] == 0) setNote(map(constrain(axis[2] == 0 ? x : axis[3] == 0 ? y : z, min, max), min, max, numNotes * _selectedOctave, (numNotes * (_selectedOctave + 2)) - 1));
	(axis[2] == 1 || axis[3] == 1 || axis[4] == 1) && (gain = map(constrain(axis[2] == 1 ? x : axis[3] == 1 ? y : z, min, max), min, max, 0, (1 << _sampleBits) / 2));
	(axis[2] == 2 || axis[3] == 2 || axis[4] == 2) && (_chainSawInterval = map(constrain(axis[2] == 2 ? x : axis[3] == 2 ? y : z, min, max), min, max, 255, 10)) && (_chainSawInterval == 255) && (_chainSaw = false);
}

void Synth::photoResistor(int read, unsigned int min, unsigned int max) {
	if(!_touching) return;
	if(axis[5] == 0) setNote(map(read, min, max, numNotes * _selectedOctave, (numNotes * (_selectedOctave + 2)) - 1));
	(axis[5] == 1) && (gain = map(read, min, max, 0, (1 << _sampleBits) / 2));
	(axis[5] == 2) && (_chainSawInterval = map(read, min, max, 255, 10));	
}

void Synth::sequencerTick(byte tempoStep) {
	switch(sequencerStatus) {
		case 1: //recording
			if(_touching) {
				(axis[0] == 0 || axis[1] == 0 || axis[2] == 0 || axis[3] == 0 || axis[4] == 0 || axis[5] == 0) && (_sequencerSteps[tempoStep].note = _note);
				(axis[0] == 1 || axis[1] == 1 || axis[2] == 1 || axis[3] == 1 || axis[4] == 1 || axis[5] == 1) && (_sequencerSteps[tempoStep].gain = gain);
				(axis[0] == 2 || axis[1] == 2 || axis[2] == 2 || axis[3] == 2 || axis[4] == 2 || axis[5] == 2) && (_sequencerSteps[tempoStep].chainSawInterval = _chainSawInterval);
				//_sequencerSteps[tempoStep].circle[0] = _circle[0];
				//_sequencerSteps[tempoStep].circle[1] = _circle[1];
			}
		case 0: //playing
			if((!_touching || (axis[0] != 0 && axis[1] != 0 && axis[2] != 0 && axis[3] != 0 && axis[4] != 0 && axis[5] != 0)) && _note != _sequencerSteps[tempoStep].note) setNote(_sequencerSteps[tempoStep].note);
			(!_touching || (axis[0] != 1 && axis[1] != 1 && axis[2] != 1 && axis[3] != 1 && axis[4] != 1 && axis[5] != 1)) && (gain = _sequencerSteps[tempoStep].gain);
			(!_touching || (axis[0] != 2 && axis[1] != 2 && axis[2] != 2 && axis[3] != 2 && axis[4] != 2 && axis[5] != 2)) && (_chainSawInterval = _sequencerSteps[tempoStep].chainSawInterval) && (_chainSawInterval == 255) && (_chainSaw = false);
			/*if(!_touching) {
				_circle[0] = _sequencerSteps[tempoStep].circle[0];
				_circle[1] = _sequencerSteps[tempoStep].circle[1];
			}*/
	}
	_tempoStep = tempoStep / 16;
}

void Synth::clearSequence() {
	for(byte x=0; x<numTempoSteps; x++) {
		_sequencerSteps[x].note = 255;
		_sequencerSteps[x].gain = (1 << _sampleBits) / 4;
		_sequencerSteps[x].chainSawInterval = 255;
		//_sequencerSteps[x].circle[0] = -1;
		//_sequencerSteps[x].circle[1] = -1;
	}
	sequencerStatus = 0; //Stop sequencer
	setNote(255);
}

void Synth::saveSequence(char * path) {
	SD.mkdir("/SYNTH/");
	SD.remove(path);
	File f = SD.open(path, FILE_WRITE);
	for(byte x=0; x<numTempoSteps; x++) {
		f.write(_sequencerSteps[x].note);
		f.write(lowByte(_sequencerSteps[x].gain));
		f.write(highByte(_sequencerSteps[x].gain));
		f.write(_sequencerSteps[x].chainSawInterval);
		//f.write(lowByte(_sequencerSteps[x].circle[0]));
		//f.write(highByte(_sequencerSteps[x].circle[0]));
		//f.write(lowByte(_sequencerSteps[x].circle[1]));
		//f.write(highByte(_sequencerSteps[x].circle[1]));
	}
	f.close();
}

void Synth::loadSequence(char * path) {
	clearSequence();

	File f = SD.open(path);
	byte x = 0,
		i = 0,
		v[2] = {0, 0};

	while(f.available() && x < numTempoSteps) {
		v[i == 2 || i == 5 || i == 7 ? 1 : 0] = f.read();
		switch(i) {
			case 0:
				_sequencerSteps[x].note = v[0];
			break;
			case 2:
				_sequencerSteps[x].gain = (unsigned int) v[0] + ((unsigned int) v[1] << 8);
			break;
			case 3:
				_sequencerSteps[x].chainSawInterval = v[0];
			break;
			/*case 5:
				_sequencerSteps[x].circle[0] = (int) v[0] + ((int) v[1] << 8);
			break;
			case 7:
				_sequencerSteps[x].circle[1] = (int) v[0] + ((int) v[1] << 8);
			break;*/
		}
		i++;
		if(i == 4 /* i == 8 */) {
			i = 0;
			x++;
		}
	}
	f.close();

	sequencerStatus = 0; //play sequence
}

void Synth::render(UTFT tft) {
	tft.clrScr();
	tft.setFont(BigFont);
	_tft = tft;
	_timelineW = ((float) (_tft.getDisplayXSize() - 1) / (float) numTempoSteps) * 16;
	for(byte i=0; i<numTempoSteps / 16; i++) {
		int x = (float) i * _timelineW;
		_tft.setColor(255, 255, 255);
		_tft.drawLine(x, 0, x, 5);
	}
	_renderedTempoStep = numTempoSteps / 16;
	_renderedNote = 255;
	_renderedScale = 255;
	_renderedCircle[0] = -1;
	_renderedCircle[1] = -1;
	update();
}

void Synth::update() {
	renderScale();
	renderNote();
	renderCircle();
	renderTimeline();
}

void Synth::onTouch(byte orientation, int x, int y) {
	if(axis[0] == 0 || axis[1] == 0) setNote(map(axis[1] == 0 ? y : x, (axis[1] == 0 ? _tft.getDisplayYSize() - 1 : 0), (axis[1] == 0 ? 0 : _tft.getDisplayXSize() - 1), numNotes * _selectedOctave, (numNotes * (_selectedOctave + 2)) - 1));
	(axis[0] == 1 || axis[1] == 1) && (gain = map(axis[0] == 1 ? x : y, (axis[0] == 1 ? 0 : _tft.getDisplayYSize() - 1), (axis[0] == 1 ? _tft.getDisplayXSize() - 1 : 0), 0, (1 << _sampleBits) / 2));
	(axis[0] == 2 || axis[1] == 2) && (_chainSawInterval = map(axis[0] == 2 ? x : y, (axis[0] == 2 ? 0 : _tft.getDisplayYSize() - 1), (axis[0] == 2 ? _tft.getDisplayXSize() - 1 : 0), 255, 10)) && (_chainSawInterval == 255) && (_chainSaw = false);
	x < 10 && (x = 10);
	x > _tft.getDisplayXSize() - 11 && (x = _tft.getDisplayXSize() - 11);
	y < 10 && (y = 10);
	y > _tft.getDisplayYSize() - 11 && (y = _tft.getDisplayYSize() - 11);
	//if(abs(x - _circle[0]) < 2 || abs(y - _circle[1]) < 2) return; 
	_circle[0] = x;
	_circle[1] = y;
	_touching = true;
}

void Synth::onTouchEnd() {
	if(!_touching) return;
	_chainSawInterval = 255;
	setNote(255);
	_circle[0] = -1;
	_circle[1] = -1;
	_touching = _chainSaw = false;
}

void Synth::renderNote() {
	if(_note == _renderedNote) return;
	_renderedNote = _note;

	int x = ((_tft.getDisplayXSize() - 1) / 2),
		y = ((_tft.getDisplayYSize() - 1) / 2);

	if(_renderedNote == 255) {
		_tft.setColor(0, 0, 0);
		_tft.fillRect(x - 40, y - 8, x + 40, y + 8);
		return;
	}
	_tft.setBackColor(0, 0, 0);
	_tft.setColor(255, 255, 255);
	String noteName = "";
	noteName += noteNames[_scale[_renderedNote] % 12];
	noteName.length() < 2 && (noteName += " ");
	noteName += "(";
	noteName += (_scale[_renderedNote] / 12) - 1;
	noteName += ")";
	_tft.print(noteName, x - 40, y - 8);
}

void Synth::renderScale(bool force) {
	if(!force && _selectedScale == _renderedScale) return;
	_renderedScale = _selectedScale;

	int x = _tft.getDisplayXSize() - 150,
		y = _tft.getDisplayYSize() - 17;

	_tft.setColor(0, 0, 0);
	_tft.fillRect(x, y, _tft.getDisplayXSize() - 1, _tft.getDisplayYSize() - 1);
	_tft.setBackColor(0, 0, 0);
	_tft.setColor(255, 255, 255);
	_tft.print(scaleNames[_renderedScale], x, y);
}

void Synth::renderCircle() {
	if(_circle[0] == _renderedCircle[0] && _circle[1] == _renderedCircle[1]) return;

	if(_renderedCircle[0] != -1 && _renderedCircle[1] != -1) {
		_tft.setColor(0, 0, 0);
		_tft.drawCircle(_renderedCircle[0], _renderedCircle[1], 10);
		if(_renderedCircle[0] > _tft.getDisplayXSize() - 160 && _renderedCircle[1] > _tft.getDisplayYSize() - 27) renderScale(true);
	}
	_renderedCircle[0] = _circle[0];
	_renderedCircle[1] = _circle[1];
	_tft.setColor(127, 255, 127);
	_tft.drawCircle(_renderedCircle[0], _renderedCircle[1], 10);
}

void Synth::renderTimeline() {
	if(_tempoStep == _renderedTempoStep) return;

	int x = (float) _renderedTempoStep * _timelineW;
	_renderedTempoStep = _tempoStep;
	_tft.setColor(0, 0, 0);
	_tft.fillRect(x, 0, x + _timelineW, 5);
	_tft.setColor(255, 255, 255);
	_tft.drawLine(x, 0, x, 5);
	x = (float) _renderedTempoStep * _timelineW;
	if(sequencerStatus == 1) _tft.setColor(255, 0, 0);
	else _tft.setColor(0, 255, 0);
	_tft.fillRect(x, 0, x + _timelineW, 5);
}

