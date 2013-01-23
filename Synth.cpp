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

Synth::Synth(int sampleRate, Midi midi) : UI() {
	byte x;
	note = 255;
	waveOn = 1;
	_output = 0;
	gain = (1 << _sampleBits) / 8;
	for(x=0; x<numWaves; x++) {
		waveGain[x] = 1 << _sampleBits;
 		waves[x] = new Wave(WaveShapeSquare, sampleRate);
 	}
 	selectedNote = note;
 	_chainSaw = 0;
 	_chainSawEnabled = 0;
 	_chainSawInterval = 0;
 	_chainSawLastLoop = 0;
 	_midiEnabled = 0;
 	_hold = 0;
 	//_holding = 0;
 	_gainModEnabled = 0;
 	_selectedOctave = 1;
	_selectedScale = 1;
	_selectedRoot = 0;
	_waveNoteOffset[0] = 0;
	_waveNoteOffset[1] = 7;
	_waveNoteOffset[2] = 12;
	_waveNoteOffset[3] = 16;
	_tempoStep = 0;
 	_midi = midi;
 	clearSequencer();
 	setScale(_selectedScale, _selectedRoot);
 	addButton(NULL);
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
		cn = note,
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
	if(sequencerStatus == 0 && cn != 255) setNote(note);
}

void Synth::setNote(byte nt) {
	if(_midiEnabled && note != 255) {
		for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOff(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	}
	note = nt;
	if(nt == 255) return;
	for(byte x=0; x<numWaves; x++) {
		waves[x]->setFrequency(pgm_read_word_near(midiNotes + (_scale[note] + _waveNoteOffset[x])));
		if(_midiEnabled && waveOn & (1 << x)) _midi.sendNoteOn(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
	}
}

void Synth::setOctave(byte id) {
	if(_selectedOctave == id) return;
	const byte noteIndex = selectedNote - (_selectedOctave * numNotes);
	_selectedOctave = id;
	if(note != 255) {
		selectedNote != 255 && (selectedNote = (_selectedOctave * numNotes) + noteIndex);
		if(sequencerStatus == 0) setNote(selectedNote);
	}
}

int Synth::output() {
	if(!_chainSaw && note != 255) {
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
	//if(!_chainSawEnabled || !waveOn || (!_hold && !_holding)) return;
	if(!_chainSawEnabled || !waveOn || note == 255) return;
	unsigned long t = millis();
	if(t >= _chainSawLastLoop + _chainSawInterval) {
		_chainSawLastLoop = t;
		_chainSaw = false;
		if(_midiEnabled && note != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOn(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	} else if(!_chainSaw && t >= _chainSawLastLoop + (_chainSawInterval / (_chainSawInterval > 250 ? 4 : _chainSawInterval > 50 ? 3 : 2))) {
		_chainSaw = true;
		if(_midiEnabled && note != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			_midi.sendNoteOff(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
		}
	}
}

void Synth::chainSawToggle() {
	_chainSawEnabled = !_chainSawEnabled;
	if(_chainSawEnabled) return;
	_chainSaw = false;
	if(_midiEnabled && note != 255) for(byte x=0; x<numWaves; x++) {
		if(!(waveOn & (1 << x))) continue;
		_midi.sendNoteOn(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
	}
}

bool Synth::midiToggle() {
	_midiEnabled = !_midiEnabled;
	if(!_midiEnabled && !_chainSaw && note != 255) for(byte x=0; x<numWaves; x++) {
		if(!(waveOn & (1 << x))) continue;
		_midi.sendNoteOff(_scale[note] + _waveNoteOffset[x], map(waveGain[x], 0, 1 << _sampleBits, 0, 127), _midiChannel);
	}
	return _midiEnabled;
}

void Synth::holdToggle() {
	_hold = !_hold;
	_chainSaw = !_hold;
	_holding = _hold;
	setHoldArea();
}

void Synth::setHoldArea() {
	if(_hold) {
		_buttons->x = _buttons->y = _buttons->width = _buttons->height = -1;
		return;
	}
	_buttons->x = _buttons->y = 0;
	_buttons->width = _tft.getDisplayXSize() - 1;
	_buttons->height = _tft.getDisplayYSize() - 1;	
}

void Synth::gainModToggle() {
	_gainModEnabled = !_gainModEnabled;
}

void Synth::sequencerTick(byte tempoStep) {
	switch(sequencerStatus) {
		case 0: //stopped
		
		break;
		case 1: //recording
			//if(_hold || _holding) {
			if(selectedNote != 255) {
				_sequencerSteps[tempoStep].note = selectedNote;
				_gainModEnabled && (_sequencerSteps[tempoStep].gain = gain);
				_chainSawEnabled && (_sequencerSteps[tempoStep].chainSawInterval = _chainSawInterval);
				_sequencerSteps[tempoStep].circle[0] = _circle[0];
				_sequencerSteps[tempoStep].circle[1] = _circle[1];
				break;
			}
		case 2: //playing
			if(selectedNote == 255) {
				if(_sequencerSteps[tempoStep].note != note) setNote(_sequencerSteps[tempoStep].note);
				_gainModEnabled && (gain = _sequencerSteps[tempoStep].gain);
				_chainSawEnabled && (_chainSawInterval = _sequencerSteps[tempoStep].chainSawInterval);
				_circle[0] = _sequencerSteps[tempoStep].circle[0];
				_circle[1] = _sequencerSteps[tempoStep].circle[1];
			}
	}
	_tempoStep = tempoStep / 16;
}

void Synth::clearSequencer() {
	for(byte x=0; x<numTempoSteps; x++) {
		_sequencerSteps[x].note = 255;
		_sequencerSteps[x].gain = (1 << _sampleBits) / 8;
		_sequencerSteps[x].chainSawInterval = 0;
		_sequencerSteps[x].circle[0] = -1;
		_sequencerSteps[x].circle[1] = -1;
	}
	sequencerStatus = 0; //Stop sequencer
	setNote(255);
}

void Synth::saveSequence() {
	SD.mkdir("/SEQS");
	File f = SD.open("/SEQS/LAST");
	char buf[4];
	byte c = 0;
	while(f.available()) {
		buf[c] = f.read();
		c++;
	}
	f.close();
	String last = "";
	last += atoi(buf) + 1;
	while(last.length() < 4) last = "0" + last;
	String path = "/SEQS/";
	path += last;
	path += ".seq";
	char p[path.length()];
	path.toCharArray(p, path.length() + 1);
	f = SD.open(p, FILE_WRITE);
	for(byte x=0; x<numTempoSteps; x++) {
		f.print(_sequencerSteps[x].note);
		f.print(",");
		f.print(_sequencerSteps[x].gain);
		f.print(",");
		f.print(_sequencerSteps[x].chainSawInterval);
		f.print(",");
		f.print(_sequencerSteps[x].circle[0]);
		f.print(",");
		f.println(_sequencerSteps[x].circle[1]);
	}
	f.close();
	SD.remove("/SEQS/LAST");
	f = SD.open("/SEQS/LAST", FILE_WRITE);
	f.print(last);
	f.close();
}

void Synth::loadSequence(String name) {
	clearSequencer();
	
	String path = "/SEQS/";
	path += name;
	char p[path.length()];
	path.toCharArray(p, path.length() + 1);

	File f = SD.open(p);
	byte c = 0,
		x = 0,
		i = 0;

	char buf[4],
		ch;

	while(f.available() && x < numTempoSteps) {
		ch = f.read();
		if(ch == ',' || ch == '\n') {
			switch(i) {
				case 0:
					_sequencerSteps[x].note = atoi(buf);
				break;
				case 1:
					_sequencerSteps[x].gain = atoi(buf);
				break;
				case 2:
					_sequencerSteps[x].chainSawInterval = atoi(buf);
				break;
				case 3:
					_sequencerSteps[x].circle[0] = atoi(buf);
				break;
				case 4:
					_sequencerSteps[x].circle[1] = atoi(buf);
			}
			c = 0;
			for(byte y=0; y<4; y++) buf[y] = NULL;
			i++;
			if(i == 5 || ch == '\n') {
				i = 0;
				x++;
			}
		} else {
			buf[c] = ch;
			c++;
		}
	}
	f.close();

	sequencerStatus = 2; //play sequence
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
	setHoldArea();
	update();
}

void Synth::update() {
	renderScale();
	renderNote();
	renderCircle();
	renderTimeline();
}

void Synth::onTouch(byte orientation, int x, int y) {
	selectedNote = map(orientation == PORTRAIT ? y : x, (orientation == PORTRAIT ? _tft.getDisplayYSize() - 1 : 0), (orientation == PORTRAIT ? 0 : _tft.getDisplayXSize() - 1), numNotes * _selectedOctave, (numNotes * (_selectedOctave + 2)) - 1);
	if(note != selectedNote) setNote(selectedNote);
	_chainSawEnabled && (_chainSawInterval = map(orientation == PORTRAIT ? x : y, (orientation == PORTRAIT ? 0 : _tft.getDisplayYSize() - 1), (orientation == PORTRAIT ? _tft.getDisplayXSize() - 1 : 0), 250, 10));
	_gainModEnabled && (gain = map(orientation == PORTRAIT ? x : y, (orientation == PORTRAIT ? 0 : _tft.getDisplayYSize() - 1), (orientation == PORTRAIT ? _tft.getDisplayXSize() - 1 : 0), 0, (1 << _sampleBits) / 4));
	x < 10 && (x = 10);
	x > _tft.getDisplayXSize() - 11 && (x = _tft.getDisplayXSize() - 11);
	y < 10 && (y = 10);
	y > _tft.getDisplayYSize() - 11 && (y = _tft.getDisplayYSize() - 11);
	//if(abs(x - _circle[0]) < 2 || abs(y - _circle[1]) < 2) return; 
	_circle[0] = x;
	_circle[1] = y;
}

void Synth::renderNote() {
	//Maybe is better do this with selectedNote (for the photoresistor case)
	if(note == _renderedNote) return;
	_renderedNote = note; //same here

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

/*void Synth::onDown(byte id) {
	_chainSaw = false;
	_holding = true;
}*/

void Synth::onClick(byte id) {
	selectedNote = 255;
	setNote(selectedNote);
	_circle[0] = -1;
	_circle[1] = -1;
	//_chainSaw = true;
	//_holding = false;
}