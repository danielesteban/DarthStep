//Lib
#include <AnalogInputs.h>
#include <Buttons.h>
#include <Midi.h>
#include <Wave.h>
#include <glcd.h>
#include <fonts/allFonts.h>
#include "Samples.h"

//Constants
prog_uint16_t midiNotes[] PROGMEM = {8,9,9,10,10,11,12,12,13,14,15,15,16,17,18,19,21,22,23,24,26,28,29,31,33,35,37,39,41,44,46,49,52,55,58,62,65,69,73,78,82,87,92,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902,8372,8870,9397,9956,10548,11175,11840,12544,13290,14080,14917,15804,16744};

const byte numSamplerTextAreas = 4,
	numMenuTextAreas = 8,
	numWaveConfTextAreas = 9,
	numTriggersTextAreas = 1,
	numWaves = 6,
	numScales = 13,
	numNotes = 7,
	numOctaves = 7,
	samplerMidiChannel = 1,
	synthMidiChannel = 2,
	sampleText = 0,
	tempoText = 1,
	sampleSpeedText = 2,
	sampleGainText = 3,
	numTempoSteps = 64,
	UIViewMenu = 0,
	UIViewSampler = 1,
	UIViewWaveConf = 2,
	UIViewTriggers = 3,
	photoResistorPin = A0,
	pot1Pin = A1,
	pot2Pin = A2,
	button1Pin = 44,
	button2Pin = 45,
	button3Pin = 46,
	button4Pin = 47;

prog_uchar scales[(numNotes - 1) * numScales] PROGMEM = {
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
	scaleNames[numScales] = {"Aeolian", "Locrian", "Ionian", "Dorian", "Phrygian", "Lydian", "Mixolydia", "MAscMinor", "Raised6th", "Raised5th", "MjrMinor", "Altered", "Arabic"};

const unsigned int tempoRate = 1024;

#define samplerInterrupt (TIMER2_COMPA_vect)
#define tempoInterrupt (TIMER3_COMPA_vect)
#define inputsInterrupt (TIMER4_COMPA_vect)
#define audioOutput (PORTA)

//Vars
byte tempoStep = 0,
	selectedTempoStep = 0,
	selectedWave = 0,
	lastSelectedWave = 0,
	selectedSample = 0,
	lastSelectedSample = 0,
	selectedMenuItem = 0,
	lastSelectedMenuItem = 0,
	ledMatrix = 0,
	waveOn = 0,
	waveNoteOffset[numWaves] = {
		0,
		7,
		12,
		16,
		19,
		24
	},
	synthNote = 255,
	notePotValue = 0,
	selectedScale = 0,
	selectedRoot = 2,
	selectedUIView = UIViewMenu,
	UIView;

bool sampleSteps[numSamples][numTempoSteps],
	selectedTempoStepBlink = 0,
	photoResistorEnabled = 0,
	photoResistorCalibrate = 0,
	chainSaw = 0,
	chainSawEnabled = 0,
	notePotEnabled = 0,
	samplerMidiEnabled = 0,
	synthMidiEnabled = 0,
	invertGLCD = 0;

unsigned int tempoBpm = 120,
	chainSawInterval = 0,
	photoResistorMax = 0,
	photoResistorMin = 1023,
	waveGain[numWaves] = {
		52,
		52,
		52,
		52,
		52,
		52
	},
	scale[numOctaves * numNotes];

float tempoStepFloat,
	tempoIncrement = (((float) numTempoSteps / 8) / (float) tempoRate) * ((float) tempoBpm / 60);

unsigned long updateUILastLoop = 0,
	chainSawLastLoop = 0,
	selectedTempoStepLastBlink = 0,
	photoResistorCalibrateStart = 0;

Wave waves[numWaves] = {
	Wave(WaveShapeSquare, SAMPLE.rate),
	Wave(WaveShapeSquare, SAMPLE.rate),
	Wave(WaveShapeSquare, SAMPLE.rate),
	Wave(WaveShapeSquare, SAMPLE.rate),
	Wave(WaveShapeSquare, SAMPLE.rate),
	Wave(WaveShapeSquare, SAMPLE.rate)
};

gText samplerTextAreas[numSamplerTextAreas];
String samplerTextAreaValues[numSamplerTextAreas];

gText menuTextAreas[numMenuTextAreas];
String menuTextAreaValues[numMenuTextAreas];

gText waveConfTextAreas[numWaveConfTextAreas];
String waveConfTextAreaValues[numWaveConfTextAreas];

gText triggersTextAreas[numTriggersTextAreas];
String triggersTextAreaValues[numTriggersTextAreas];

AnalogInputs analogInputs(onChange);
Buttons buttons(onPush);
Midi midi(Serial3);

void setup() {
	byte x;
	randomSeed(analogRead(A13));

	analogInputs.setup(photoResistorPin);
	analogInputs.setup(pot1Pin);
	analogInputs.setup(pot2Pin);
	buttons.setup(button1Pin);
	buttons.setup(button2Pin);
	buttons.setup(button3Pin);
	buttons.setup(button4Pin, NULL, onDown);

	setScale(selectedScale, selectedRoot);

	midi.begin();

	GLCD.Init();
	for(x=0; x<numSamplerTextAreas; x++) {
		samplerTextAreas[x].SelectFont(SystemFont5x7);
		//samplerTextAreas[x].SetFontColor(WHITE); 
		predefinedArea area;
		switch(x) {
			case sampleText:
				area = (predefinedArea) MK_TareaToken(1, 0, 84, 14);
				samplerTextAreas[x].SelectFont(Arial_bold_14);
				samplerTextAreas[x].SetFontColor(WHITE);
			break;
			case tempoText:
				area = (predefinedArea) MK_TareaToken(GLCD.Width - 39, 2, GLCD.Width - 1, 10);
			break;
			case sampleSpeedText:
				area = (predefinedArea) MK_TareaToken(3, GLCD.Height - 9, 62, GLCD.Height - 3);
			break;
			case sampleGainText:
				area = (predefinedArea) MK_TareaToken(GLCD.Width - 59, GLCD.Height - 9, GLCD.Width - 6, GLCD.Height - 3);
			break;
		}
		samplerTextAreas[x].DefineArea(area);
	}
	
	byte w = (GLCD.Width - 3) / 2,
		xo = 0,
		c = 0;

	for(x=0; x<numMenuTextAreas; x++) {
		menuTextAreas[x].SelectFont(SystemFont5x7);
		menuTextAreas[x].DefineArea((predefinedArea) MK_TareaToken(xo, (c * 16) + 1, xo + w, (c * 16) + 17));
		c++;
		c == 4 && (xo = w + 2) && (c = 0);
	}

	w = (GLCD.Width - 6) / 3;
	byte h = (GLCD.Height - 3) / 3;
	xo = ((GLCD.Width - 6) % 3) / 2;
	byte gx = xo,
		gy = 0;

	for(x=0; x<numWaveConfTextAreas; x++) {
		waveConfTextAreas[x].SelectFont(SystemFont5x7);
		waveConfTextAreas[x].DefineArea((predefinedArea) MK_TareaToken(gx + 3, gy + 7, gx + w - 2, gy + h - 9));
		gx += w + 2;
		(x + 1) % 3 == 0 && (gy += h + 1) && (gx = xo);
	}

	triggersTextAreas[0].SelectFont(SystemFont5x7);
	triggersTextAreas[0].DefineArea((predefinedArea) MK_TareaToken(GLCD.Width - 27, GLCD.Height - 27, GLCD.Width - 1, GLCD.Height - 1));
	
	//set PORTA to all outputs- these bits will be used to send audio data to the R2R DAC
	DDRA = 0xFF;

	cli(); //stop interrupts

	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21); //turn on CTC mode
	TCCR2B |= (1 << CS21); //Set CS11 bit for 8 prescaler
	OCR2A = (F_CPU / ((long) SAMPLE.rate * 8)) - 1; //set compare match register for 16khz increments
	TIMSK2 |= (1 << OCIE2A); //enable timer compare interrupt

	TCCR3A = 0;
	TCCR3B = 0;
	TCCR3B |= (1 << WGM32);
	TCCR3B |= (1 << CS30); //no prescaler
	OCR3A = (F_CPU / tempoRate) - 1;
	TIMSK3 |= (1 << OCIE3A);

	TCCR4A = 0;
	TCCR4B = 0;
	TCCR4B |= (1 << WGM42);
	TCCR4B |= (1 << CS40); //no prescaler
	OCR4A = (F_CPU / (tempoRate / 2)) - 1;
	TIMSK4 |= (1 << OCIE4A);

	sei(); //allow interrupts

	//debug
	/*sampleSteps[0][0] = 1;
	sampleSteps[0][16] = 1;
	sampleSteps[0][32] = 1;
	sampleSteps[0][48] = 1;
	sampleSteps[1][8] = 1;
	sampleSteps[1][24] = 1;
	sampleSteps[1][40] = 1;
	sampleSteps[1][56] = 1;
	sampleSteps[4][0] = 1;
	sampleSteps[4][8] = 1;
	sampleSteps[4][16] = 1;
	sampleSteps[4][24] = 1;
	sampleSteps[4][32] = 1;
	sampleSteps[4][40] = 1;
	sampleSteps[4][48] = 1;
	sampleSteps[4][56] = 1;*/
	//SAMPLE.quants[0] = 16;
	//SAMPLE.quants[2] = 4;
	SAMPLE.quants[3] = 32;
	SAMPLE.quants[4] = 32;
	//SAMPLE.quants[5] = 32;
	//tempoBpm = 120;
	//tempoIncrement = (((float) numTempoSteps / 8) / (float) tempoRate) * ((float) tempoBpm / 60);

	//Serial.begin(9600);
	//Serial.println(tempoIncrement, DEC);

	setUIView(selectedUIView);
}

void setScale(byte id, byte root) {
	const byte offset = (numNotes - 1) * id;
	byte octave, note,
		n[numNotes],
		cn = synthNote,
		c = 0;

	selectedScale = id;
	selectedRoot = root;
	root += 24;
	if(synthMidiEnabled && cn != 255) setNote(255);
	for(octave = 0; octave < numOctaves; octave++) {
		for(note = 0; note < numNotes; note++) {
			if(octave == 0) n[note] = note == 0 ? root : n[note - 1] + pgm_read_byte_near(scales + (offset + (note - 1)));
			else n[note] += 12;
			scale[c] = n[note];
			c++;
		}
	}
	if(cn != 255) setNote(synthNote);
}

void setNote(byte note) {
	if(synthMidiEnabled && synthNote != 255) {
		for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			midi.sendNoteOff(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
		}
	}
	synthNote = note;
	if(note == 255) return;
	for(byte x=0; x<numWaves; x++) {
		waves[x].setFrequency(pgm_read_word_near(midiNotes + (scale[synthNote] + waveNoteOffset[x])));
		if(synthMidiEnabled && waveOn & (1 << x)) midi.sendNoteOn(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
	}
}

void updateUI(bool force = false);

void loop() {
	if(selectedUIView != UIView) setUIView(selectedUIView);
	else updateUI();
	if(photoResistorEnabled && photoResistorCalibrate) {
		if(photoResistorCalibrateStart == 0) {
			photoResistorCalibrateStart = millis();
			photoResistorMax = 0;
			photoResistorMin = 1023;
		}
		int read = analogInputs.get(photoResistorPin)->read;
		photoResistorMax < read && (photoResistorMax = read);
		photoResistorMin > read && (photoResistorMin = read);
		photoResistorCalibrateStart <= millis() - 1000 && (photoResistorCalibrateStart = photoResistorCalibrate = 0);
	}
}

void setUIView(byte id) {
	UIView = id;
	GLCD.ClearScreen();
	byte x, w, h, gx, gy, xo;
	switch(id) {
		case UIViewSampler:
			GLCD.DrawHLine(0, GLCD.Height - 12, GLCD.Width);
			GLCD.DrawVLine(64, GLCD.Height - 11, 10);
			GLCD.SetDot(64, GLCD.Height - 12, WHITE);
			GLCD.SetDot(0, GLCD.Height - 12, WHITE);
			GLCD.DrawVLine(0, GLCD.Height - 11, 10);
			GLCD.SetDot(GLCD.Width - 1, GLCD.Height - 12, WHITE);
			GLCD.DrawVLine(GLCD.Width - 1, GLCD.Height - 11, 10);
		break;
		case UIViewMenu:
			selectedMenuItem = 0;
		break;
		case UIViewWaveConf:
			selectedWave = 0;
			w = (GLCD.Width - 6) / 3;
			h = (GLCD.Height - 3) / 3;
			xo = ((GLCD.Width - 6) % 3) / 2;
			gx = xo;
			gy = 0;
			for(x=0; x<numWaveConfTextAreas; x++) {
				GLCD.DrawHLine(gx, gy, w);
				gx += w + 2;
				if((x + 1) % 3 == 0) {
					GLCD.DrawVLine(gx - 1, gy, h);
					(gy += h + 1) && (gx = xo);
					x == numWaveConfTextAreas - 1 && (gy = 0);
					GLCD.DrawVLine(gx, gy, h);
				} else GLCD.DrawVLine(gx - 1, gy, h);
			}
			GLCD.DrawHLine(xo, GLCD.Height - 1, (w * 3) + 5);
		break;
		case UIViewTriggers:
			
		break;
	}
	updateUI(true);
}

void updateUI(bool force) {
	unsigned long t = millis();
	if(!force && t < updateUILastLoop + 100) return;
	updateUILastLoop = t;
	switch(UIView) {
		case UIViewSampler:
			samplerUI(force);
		break;
		case UIViewMenu:
			menuUI(force);
		break;
		case UIViewWaveConf:
			waveConfUI(force);
		break;
		case UIViewTriggers:
			triggersUI(force);
		break;
	}
}

void samplerUI(bool force) {
	byte x, v;
	String value;
	for(x=0; x<numSamplerTextAreas; x++) {
		value = "";
		switch(x) {
			case sampleText:
				value = sampleNames[selectedSample];
			break;
			case tempoText:
				tempoBpm < 100 && (value = " ");
				value += tempoBpm;
				value += "BPM";
			break;
			case sampleSpeedText:
				v = SAMPLE.quants[selectedSample] / 2;
				value = "Quant:";
				value += v;
				value += "th";
			break;
			case sampleGainText:
				v = map(SAMPLE.gain[selectedSample], 0, 1 << SAMPLE.bits, 0, 100);
				value = "Gain:";
				value += v;
				value += "%";
			break;
		}
		if(force || samplerTextAreaValues[x] != value) {
			samplerTextAreaValues[x] = value;
			samplerTextAreas[x].ClearArea();
			samplerTextAreas[x].print(value);
			switch(x) {
				case sampleText:
					GLCD.DrawVLine(0, 0, 11);
					GLCD.FillRect(1, 12, 84, 2, WHITE);
					GLCD.SetDot(84, 0, WHITE);
				break;
				case tempoText:
					GLCD.DrawHLine(GLCD.Width - 43, 11, 42);
				break;
			}
		}
	}

	const byte numSteps = SAMPLE.quants[selectedSample];
	
	const float stepRatio = numTempoSteps / numSteps;

	const byte stepWidth = GLCD.Width / (float) numSteps,
		boxWidth = stepWidth * 0.8,
		current = round(tempoStep / stepRatio);

	byte offset = ((GLCD.Width + (stepWidth - boxWidth)) % numSteps) / 3;

	if(selectedSample != lastSelectedSample) {
		lastSelectedSample = selectedSample;
		GLCD.FillRect(0, 20, GLCD.Width - 1, 17, WHITE);
	}

	for(x=0; x<numSteps; x++) {
		x == (numSteps / 2) && (offset += offset);
		byte gx = offset + (stepWidth * x);
		const bool bg = boxWidth > 4,
			blink = selectedTempoStepBlink && (x * stepRatio) == selectedTempoStep;

		GLCD.FillRect(gx, 25, boxWidth, 12, !blink && sampleSteps[selectedSample][(byte) (x * stepRatio)] ? BLACK : WHITE);
		if(bg) {
			GLCD.SetDot(gx, 25, WHITE);
			GLCD.SetDot(gx, 37, WHITE);
			GLCD.SetDot(gx + boxWidth, 25, WHITE);
			GLCD.SetDot(gx + boxWidth, 37, WHITE);
		}
		GLCD.DrawRoundRect(gx, 25, boxWidth, 12, 2, blink ? WHITE : BLACK);
		if(!bg) {
			GLCD.DrawVLine(gx, 25, 12, WHITE);
			GLCD.DrawVLine(gx + boxWidth, 25, 12, WHITE);
		}
		gx += bg ? round(((float) boxWidth - 4) / 2) : 0;
		GLCD.DrawLine(gx, 20 + (bg ? 0 : 1), gx + (bg ? 2 : 1), 22, current == x ? BLACK : WHITE);
		GLCD.DrawLine(gx + (bg ? 2 : 1), 22, gx + (bg ? 4 : 3), 20 + (bg ? 0 : 1), current == x ? BLACK : WHITE);
	}
	unsigned long t = millis();
	if(t >= selectedTempoStepLastBlink + (selectedTempoStepBlink ? 125 : 500)) {
		selectedTempoStepLastBlink = t;
		selectedTempoStepBlink = !selectedTempoStepBlink;
	}
}

void triggersUI(bool force) {
	byte halfSamples = ceil((float) numSamples / 2), 
		gx = 5,
		gy = 5,
		gw = (GLCD.Width - 11) / halfSamples,
		gh = (GLCD.Height - 11) / 2,
		x;
	
	for(x=0; x<numSamples; x++) {
		if(x == halfSamples) {
			gx = 5;
			gy += gh;
		}
		GLCD.FillRect(gx, gy, gw, gh, ledMatrix & (1 << x) ? BLACK : WHITE);
		GLCD.DrawRoundRect(gx, gy, gw, gh, 2);
		GLCD.SetDot(gx, gy, WHITE);
		GLCD.SetDot(gx, gy + gh, WHITE);
		GLCD.SetDot(gx + gw, gy, WHITE);
		GLCD.SetDot(gx + gw, gy + gh, WHITE);
		gx += gw;
	}

	String value = tempoBpm < 100 ? " " : "";
	value += tempoBpm;
	value += "\nBPM";
	
	if(force || triggersTextAreaValues[0] != value) {
		triggersTextAreaValues[0] = value;
		triggersTextAreas[0].ClearArea();
		triggersTextAreas[0].print(value);
	}
}

void menuUI(bool force) {
	byte w = (GLCD.Width - 3) / 2,
		c = 0,
		wo = 0;
	
	String value;

	for(byte x=0; x<numMenuTextAreas; x++) {
		value = "";	
		switch(x) {
			case 0:
				value = "Sampler\n       ->>";
			break;
			case 1:
				value = "PhotoRes:\n ";
				value += (photoResistorEnabled ? "On" : "Off");
			break;
			case 2:
				value = "ChainSaw:\n ";
				value += (chainSawEnabled ? "On" : "Off");
			break;
			case 3:
				value = "NotePot:\n ";
				if(notePotEnabled) {
					value += noteNames[scale[notePotValue] % 12];
					value += "(";
					value += (scale[notePotValue] / 12) - 1;
					value += ")";
				} else value += "Off";
			break;
			case 4:
				value = "Scale: ";
				value += noteNames[selectedRoot];
				value += "\n ";
				value += scaleNames[selectedScale];
			break;
			case 5:
				value = "MidiOutSa\n ";
				value += (samplerMidiEnabled ? "On" : "Off");
			break;
			case 6:
				value = "MidiOutSy\n ";
				value += (synthMidiEnabled ? "On" : "Off");
			break;
			case 7:
				value = "Wave Conf\n       ->>";
			break;
		}
		if(force || selectedMenuItem != lastSelectedMenuItem || menuTextAreaValues[x] != value) {
			menuTextAreaValues[x] = value;
			menuTextAreas[x].SetFontColor(x == selectedMenuItem ? WHITE : BLACK);
			menuTextAreas[x].ClearArea();
			menuTextAreas[x].print(" " + value);
			GLCD.DrawHLine(wo, (c * 16), w);
			GLCD.DrawVLine(wo, (c * 16), 15);
			GLCD.DrawVLine(wo + w, (c * 16), 15);
		}
		c++;
		c == 4 && (wo = w + 2) && (c = 0);
	}
	lastSelectedMenuItem = selectedMenuItem;
}

void waveConfUI(bool force) {
	String value;
	byte x,
		wo = selectedWave - (selectedWave % 3),
		w = (GLCD.Width - 6) / 3,
		h = (GLCD.Height - 3) / 3,
		xo = ((GLCD.Width - 6) % 3) / 2,
		gx = xo,
		gy = 0,
		c;

	for(x=0; x<numWaveConfTextAreas; x++) {
		value = "";	
		switch(x) {
			case 0:
				value = waveOn & (1 << wo) ? WaveShapeNames[waves[wo].getShape()] : "Off";
			break;
			case 1:
				value = "+";
				value += waveNoteOffset[wo];
			break;
			case 2:
				value += map(waveGain[wo], 0, 1 << SAMPLE.bits, 0, 100);
				value += "%";
			break;
			case 3:
				value = waveOn & (1 << wo + 1) ? WaveShapeNames[waves[wo + 1].getShape()] : "Off";
			break;
			case 4:
				value = "+";
				value += waveNoteOffset[wo + 1];
			break;
			case 5:
				value += map(waveGain[wo + 1], 0, 1 << SAMPLE.bits, 0, 100);
				value += "%";
			break;
			case 6:
				value = waveOn & (1 << wo + 2) ? WaveShapeNames[waves[wo + 2].getShape()] : "Off";
			break;
			case 7:
				value = "+";
				value += waveNoteOffset[wo + 2];
			break;
			case 8:
				value += map(waveGain[wo + 2], 0, 1 << SAMPLE.bits, 0, 100);
				value += "%";
		}
		if(force || selectedWave != lastSelectedWave || waveConfTextAreaValues[x] != value) {
			waveConfTextAreaValues[x] = value;
			waveConfTextAreas[x].ClearArea();
			c = x / 3 == selectedWave - wo ? BLACK : WHITE;
			GLCD.FillRect(gx + 1, gy + 1, w - 1, h - 1, c);
			if(x % 3 != 0) GLCD.DrawVLine(gx, gy + 1, h - 1, c);
			waveConfTextAreas[x].SetFontColor(!c);	
			waveConfTextAreas[x].print(value);
		}
		gx += w + 2;
		(x + 1) % 3 == 0 && (gy += h + 1) && (gx = xo);
	}
	lastSelectedWave = selectedWave;
}

void onChange(byte pin, int read) {
	switch(UIView) {
		case UIViewSampler:
			switch(pin) {
				case pot1Pin:
					if(buttons.get(button4Pin)->status == LOW) { //Experimental.. will be buggy when shrinking down
						read = pow(2, map(read, 0, 1023, 2, 5)) + 1;
						if(read == SAMPLE.quants[selectedSample]) return;
						SAMPLE.quants[selectedSample] = read;
						lastSelectedSample = 255;
					} else SAMPLE.gain[selectedSample] = map(read, 0, 1023, 0, 1 << SAMPLE.bits); 
				break;
				case pot2Pin:
					read = (numTempoSteps / SAMPLE.quants[selectedSample]) * map(read, 0, 1023, 0, SAMPLE.quants[selectedSample] - 1);
					if(read == selectedTempoStep) return;
					selectedTempoStep = read;
					buttons.get(button4Pin)->status == LOW && (sampleSteps[selectedSample][read] = !sampleSteps[selectedSample][read]);
				break;
			}
		break;
		case UIViewMenu:
			if(pin == pot2Pin) {
				if(!notePotEnabled) return;
				notePotValue = map(read, 0, 1023, 0, (numNotes * numOctaves) - 1);
				if(synthNote == notePotValue) return;
				setNote(notePotValue);
			}
		break;
		case UIViewWaveConf:
			switch(pin) {
				case pot1Pin:
					read = map(read, 0, 1023, 0, 24);
					if(waveNoteOffset[selectedWave] == read) return;
					waveNoteOffset[selectedWave] = read; 
					setNote(synthNote);
				break;
				case pot2Pin:
					waveGain[selectedWave] = map(read, 0, 1023, 0, 1 << SAMPLE.bits);
				break;
			}
		break;
		case UIViewTriggers:
			if(pin == pot2Pin) {
				read = map(read, 0, 1023, 60, 300);
				if(tempoBpm == read) return;
				tempoBpm = read;
				tempoIncrement = (((float) numTempoSteps / 8) / (float) tempoRate) * ((float) tempoBpm / 60);
			}
		break;
	}
	if(pin == photoResistorPin && photoResistorEnabled && !photoResistorCalibrate) {
		read = constrain(map(constrain(read, photoResistorMin, photoResistorMax), photoResistorMin, photoResistorMax, notePotEnabled ? notePotValue : 0, notePotEnabled ? notePotValue + (numNotes * 2) : (numNotes * numOctaves) - 1), 0, (numNotes * numOctaves) - 1);
		if(synthNote == read) return;
		setNote(read);
	}
	if((UIView == UIViewMenu || UIView == UIViewTriggers) && pin == pot1Pin) {
		if(!chainSawEnabled && selectedMenuItem == 4) setScale(selectedScale, map(read, 0, 1023, 0, 11));
		else if(!chainSawEnabled) return;
		chainSawInterval = map(read, 0, 1023, 500, 10);
	}
}

void onPush(byte pin) {
	byte v;
	switch(UIView) {
		case UIViewSampler:
			switch(pin) {	
				case button2Pin:
					if(selectedSample == 0) selectedSample = numSamples - 1;
					else selectedSample--;
					selectedTempoStep = selectedTempoStep - (selectedTempoStep % (numTempoSteps / SAMPLE.quants[selectedSample]));
				break;
				case button3Pin:
					if(selectedSample == numSamples - 1) selectedSample = 0;
					else selectedSample++;
					selectedTempoStep = selectedTempoStep - (selectedTempoStep % (numTempoSteps / SAMPLE.quants[selectedSample]));
				break;
				case button4Pin:
					if(buttons.get(button2Pin)->status == LOW) { //reset current sample
						if(buttons.get(button3Pin)->status == HIGH) {
							for(byte y=0; y<numTempoSteps; y++) sampleSteps[selectedSample][y] = 0;
							//lame hack so it stays on the same sample when button 2 goes up
							if(selectedSample == numSamples - 1) selectedSample = 0;
							else selectedSample++;
						} else { //reset all samples
							for(byte x=0; x<numSamples; x++) {
								for(byte y=0; y<numTempoSteps; y++) sampleSteps[x][y] = 0;
							}
						}
					}
				break;
			}
		break;
		case UIViewMenu:
			switch(pin) {	
				case button2Pin:
					if(selectedMenuItem == 0) selectedMenuItem = 7;
					else selectedMenuItem--;
				break;
				case button3Pin:
					if(selectedMenuItem == 7) selectedMenuItem = 0;
					else selectedMenuItem++;
				break;
				case button4Pin:
					switch(selectedMenuItem) {
						case 0:
							selectedUIView = UIViewSampler;
						break;
						case 1:
							photoResistorCalibrate = 1;
							photoResistorEnabled = !photoResistorEnabled;
							if(!photoResistorEnabled && !notePotEnabled) setNote(255);
						break;
						case 2:
							chainSawEnabled = !chainSawEnabled;
							if(chainSawEnabled) onChange(pot1Pin, analogInputs.get(pot1Pin)->read);
							else {
								chainSaw = false;
								if(synthMidiEnabled && synthNote != 255) for(byte x=0; x<numWaves; x++) {
									if(!(waveOn & (1 << x))) continue;
									midi.sendNoteOn(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
								}
							}
						break;
						case 3:
							notePotEnabled = !notePotEnabled;
							if(notePotEnabled) onChange(pot2Pin, analogInputs.get(pot2Pin)->read);
							else if(!photoResistorEnabled) setNote(255);
						break;
						case 4:
							setScale(selectedScale == numScales - 1 ? 0 : ++selectedScale, selectedRoot); 
						break;
						case 5:
							samplerMidiEnabled = !samplerMidiEnabled;
							if(!samplerMidiEnabled) for(byte i=0; i<numSamples; i++) {
								if(SAMPLE.on & (1 << i)) midi.sendNoteOff(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
							}
						break;
						case 6:
							synthMidiEnabled = !synthMidiEnabled;
							if(!synthMidiEnabled && !chainSaw && synthNote != 255) for(byte x=0; x<numWaves; x++) {
								if(!(waveOn & (1 << x))) continue;
								midi.sendNoteOff(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
							}
						break;
						case 7:
							selectedUIView = UIViewWaveConf;
						break;
					}
				break;
			}
		break;
		case UIViewWaveConf:
			switch(pin) {
				case button2Pin:
					if(selectedWave == 0) selectedWave = numWaves - 1;
					else selectedWave--;
				break;
				case button3Pin:
					if(selectedWave == numWaves - 1) selectedWave = 0;
					else selectedWave++;
				break;
				case button4Pin:
					v = waves[selectedWave].getShape() + 1;
					if(!(waveOn & (1 << selectedWave))) {
						v = WaveShapeSquare;
						waveOn |= (1 << selectedWave);
					}
					if(v > 3) waveOn &= ~(1 << selectedWave);
					else waves[selectedWave].setShape(v);
				break;
			}
		break;
		case UIViewTriggers:
			switch(pin) {
				case button2Pin:
					invertGLCD = !invertGLCD;
					GLCD.SetDisplayMode(invertGLCD);
				break;
			}
		break;
	}
	switch(pin) {
		case button1Pin:
			selectedUIView = UIView == UIViewMenu ? UIViewTriggers : UIViewMenu;
		break;
	}
}

void onDown(byte pin) {
	if(UIView != UIViewSampler || pin != button4Pin) return;
	sampleSteps[selectedSample][selectedTempoStep] = !sampleSteps[selectedSample][selectedTempoStep];
}

ISR(inputsInterrupt) {
	analogInputs.read();
	buttons.read();
	if(!chainSawEnabled || !waveOn) return;
	unsigned long t = millis();
	if(t >= chainSawLastLoop + chainSawInterval) {
		chainSawLastLoop = t;
		chainSaw = false;
		if(synthMidiEnabled && synthNote != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			midi.sendNoteOn(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
		}
	} else if(!chainSaw && t >= chainSawLastLoop + (chainSawInterval / (chainSawInterval > 250 ? 4 : chainSawInterval > 50 ? 3 : 2))) {
		chainSaw = true;
		if(synthMidiEnabled && synthNote != 255) for(byte x=0; x<numWaves; x++) {
			if(!(waveOn & (1 << x))) continue;
			midi.sendNoteOff(scale[synthNote] + waveNoteOffset[x], map(waveGain[x], 0, 1 << SAMPLE.bits, 0, 127), synthMidiChannel);
		}
	}
}

ISR(samplerInterrupt) {
	long output = 0;
	byte i;
	for(i=0; i<numSamples; i++) {
		if(!(SAMPLE.on & (1 << i))) continue;
		if(SAMPLE.index[i] == SAMPLE.halfSize) ledMatrix &= ~(1 << i);
		if(SAMPLE.index[i] == SAMPLE.size){
			SAMPLE.index[i] = 0;
			SAMPLE.on &= ~(1 << i);
			if(samplerMidiEnabled) midi.sendNoteOff(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
		} else {
			output += (long) (127 - pgm_read_byte_near(samples[SAMPLE.alt & (1 << i) ? 1 : 0][i] + SAMPLE.index[i])) * SAMPLE.gain[i];
			SAMPLE.index[i]++;
		}
	}
	if(!chainSaw && synthNote != 255) for(i=0; i<numWaves; i++) {
		if(!(waveOn & (1 << i))) continue;
		output += (long) (127 - waves[i].next()) * waveGain[i];
	}
	audioOutput = constrain(127 + ((int) (output >> SAMPLE.bits)), 0, 255);
}

ISR(tempoInterrupt) {
	tempoStepFloat += tempoIncrement;
	if(tempoStep != (int) tempoStepFloat) {
		tempoStep = (int) tempoStepFloat;
		tempoStep >= numTempoSteps && (tempoStepFloat = tempoStep = 0);
		for(byte i=0; i<numSamples; i++) {
			if(sampleSteps[i][tempoStep]) {
				SAMPLE.index[i] = 0;
				if(samplerMidiEnabled && SAMPLE.on & (1 << i)) midi.sendNoteOff(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
				SAMPLE.on |= (1 << i);
				if((SAMPLE.hasAlt & (1 << i)) && random(2)) SAMPLE.alt |= (1 << i);
				else SAMPLE.alt &= ~(1 << i);
				ledMatrix |= (1 << i);
				if(samplerMidiEnabled) midi.sendNoteOn(sampleMidi[i], map(SAMPLE.gain[i], 0, 1 << SAMPLE.bits, 0, 127), samplerMidiChannel);
			}
		}
	}
}
