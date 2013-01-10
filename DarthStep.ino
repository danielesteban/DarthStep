//Lib
#include <AnalogInputs.h>
#include <Midi.h>
#include <Wave.h>
#include <UTFT.h>
#include <UTouch.h>
#include <SD.h>
#include <Directory.h>
#include <Menu.h>
#include "Synth.h"
#include "Sampler.h"
#include "Sequencer.h"

//Constants
const byte numSynths = 2,
	UIViewSynth1 = 0,
	UIViewSynth2 = 1,
	UIViewSampler = 2,
	UIViewMenu = 3,
	UIViewSynth1Config = 4,
	UIViewSynth2Config = 5,
	UIViewSequenceLoader = 6,
	pot1Pin = A8,
	pot2Pin = A9,
	photoResistorPin = A10;

const unsigned int sampleRate = 8000;

#define audioInterrupt (TIMER2_COMPA_vect)
#define sequencerInterrupt (TIMER3_COMPA_vect)
#define inputsInterrupt (TIMER4_COMPA_vect)
#define audioOutput (PORTF)

//Vars
byte orientation = LANDSCAPE,
	UIView = UIViewSynth1,
	sequenceLoaderSynth;

bool photoResistorEnabled = 0,
	photoResistorCalibrate = 0;

unsigned int photoResistorMax = 0,
	photoResistorMin = 1023;

unsigned long photoResistorCalibrateStart = 0;

AnalogInputs analogInputs(onChange);
Midi midi(Serial1);
Synth * synths[numSynths] = {
	new Synth(sampleRate, midi),
	new Synth(sampleRate, midi)
};
Sampler * sampler = new Sampler(midi);
Sequencer sequencer(numSynths, synths);

UTFT tft(ITDB32S, 38, 39, 40, 41);
UTouch touch(42, 43, 44, 45, 46);

//UIViews
const byte numMenuItems = 4;
String menuItems[numMenuItems] = {"Synth 1 -->", "Synth 1 Config -->", "Synth 2 -->", "Synth 2 Config -->"};

const byte numSynthConfigItems = 5;
String synthConfigItems[numSynthConfigItems] = {"Wave 1: Square", "Wave 2: Off", "Wave 3: Off", "Wave 4: Off", "MidiOut: Off"};

UI * UIViews[] = {
	(UI *) synths[0], //UIViewSynth1
	(UI *) synths[1], //UIViewSynth2
	(UI *) sampler, //UIViewSampler
	new Menu("Menu", numMenuItems, menuItems, menuOnClick), //UIViewMenu
	new Menu("Synth 1 Config", numSynthConfigItems, synthConfigItems, synthConfigOnClick), //UIViewSynth1Config
	new Menu("Synth 2 Config", numSynthConfigItems, synthConfigItems, synthConfigOnClick), //UIViewSynth2Config
	NULL //UIViewSequenceLoader
};

void setOrientation(byte o, bool redraw = true) {
	if(redraw && !UIViews[UIView]->availableOrientations[o]) return;
	orientation = o;
	tft.setOrientation(o);
	touch.setOrientation(o);
	if(redraw) {
		UIViews[UIView]->rendered = false;
		UIViews[UIView]->render(tft);
		UIViews[UIView]->rendered = true;
	}
}

void setUIView(byte view) {
	UIViews[UIView]->rendered = false;
	UIView = view;
	if(!UIViews[UIView]->availableOrientations[orientation]) return setOrientation(orientation == LANDSCAPE ? PORTRAIT : LANDSCAPE);
	UIViews[UIView]->render(tft);
	UIViews[UIView]->rendered = true;
}

//main funcs
void setup() {
	byte x;
	randomSeed(analogRead(A13));

	analogInputs.setup(pot1Pin);
	analogInputs.setup(pot2Pin);
	analogInputs.setup(photoResistorPin);

	midi.begin();

	tft.InitLCD(orientation);
	tft.clrScr();

	touch.InitTouch(orientation);
	touch.setPrecision(PREC_HI);

	SD.begin();
	
	//set PORTF to all outputs- these bits will be used to send audio data to the R2R DAC
	DDRF = 0xFF;

	cli(); //stop interrupts

	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21); //turn on CTC mode
	TCCR2B |= (1 << CS21); //Set CS11 bit for 8 prescaler
	OCR2A = (F_CPU / ((long) sampleRate * 8)) - 1; //set compare match register for 16khz increments
	TIMSK2 |= (1 << OCIE2A); //enable timer compare interrupt

	TCCR3A = 0;
	TCCR3B = 0;
	TCCR3B |= (1 << WGM32);
	TCCR3B |= (1 << CS30); //no prescaler
	OCR3A = (F_CPU / sequencer.rate) - 1;
	TIMSK3 |= (1 << OCIE3A);

	TCCR4A = 0;
	TCCR4B = 0;
	TCCR4B |= (1 << WGM42);
	TCCR4B |= (1 << CS40); //no prescaler
	OCR4A = (F_CPU / (sequencer.rate / 2)) - 1;
	TIMSK4 |= (1 << OCIE4A);

	sei(); //allow interrupts

	//debug
	Serial.begin(115200);

	//for(byte x=0; x<numSynths; x++) {
		//synths[x]->waveOn |= (1 << 0);
		//synths[x]->waveOn |= (1 << 1);
	//}

	//synth->chainSawToggle();
	//synth->waves[0]->setShape(WaveShapeSine);
	//synth->waves[1]->setShape(WaveShapeSine);
	
	setUIView(UIView);
}

void loop(void) {
	if(UIViews[UIView]->rendered) {
		UIViews[UIView]->update();
		UIViews[UIView]->readTouch(tft, touch, orientation, screenMenuOnClick);
	}
	
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

void screenMenuOnClick(byte id) {
	if(id == 0) {
		if(UIView == UIViewMenu) return;
		return setUIView(UIViewMenu);
	}
	switch(UIView) {
		case UIViewMenu:
			switch(id) {
				case 4:
					setOrientation(orientation == LANDSCAPE ? PORTRAIT : LANDSCAPE);
			}
		break;
		case UIViewSynth1:
		case UIViewSynth2:
			switch(id) {
				case 1:
					synths[UIView]->chainSawToggle();
				break;
				case 2:
					synths[UIView]->gainModToggle();
				break;
				case 3:
					renderSequenceLoader();
					
					//synths[UIView]->saveSequence();
					
					//photoResistorCalibrate = 1;
					//photoResistorEnabled = !photoResistorEnabled;
					//if(!photoResistorEnabled && !notePotEnabled) synths[UIView]->setNote(255);
				break;
				case 4:
					if(synths[UIView]->sequencerStatus == 2) synths[UIView]->clearSequencer();
					else synths[UIView]->sequencerStatus++;
			}
	}
}

void renderSequenceLoader() {
	file * dir = getDirectory("/SEQS"),
		* f = dir;

	byte count = 0,
		c = 0;

	while(f != NULL) { //this is really lame, but i'm kinda tired ;P
		strcmp(f->name, "LAST") != 0 && count++;
		f = f->next;
	}
	
	String files[count];
	
	f = dir;
	while(f != NULL) {
		if(strcmp(f->name, "LAST") != 0) {
			files[c] = f->name;
			c++;
		}
		f = f->next;
	}

	freeDirectory(dir);

	if(UIViews[UIViewSequenceLoader] != NULL) delete UIViews[UIViewSequenceLoader];
	UIViews[UIViewSequenceLoader] = new Menu("Load sequence", count, files, sequenceLoaderOnClick);
	sequenceLoaderSynth = UIView;
	setUIView(UIViewSequenceLoader);
}

void sequenceLoaderOnClick(byte id) {
	Serial.println(id);
	file * dir = getDirectory("/SEQS"),
		* f = dir;

	byte c = -1;

	while(f != NULL) {
		strcmp(f->name, "LAST") != 0 && c++;
		if(c == id) break;
		f = f->next;
	}

	synths[sequenceLoaderSynth]->loadSequence(f->name);
	setUIView(sequenceLoaderSynth);
}

void menuOnClick(byte id) {
	switch(id) {
		case 0:
			setUIView(UIViewSynth1);
		break;
		case 1:
			setUIView(UIViewSynth1Config);
		break;
		case 2:
			setUIView(UIViewSynth2);
		break;
		case 3:
			setUIView(UIViewSynth2Config);
	}
}

void synthConfigOnClick(byte id) {
	byte v;
	String s;
	Synth * synth = synths[UIView == UIViewSynth1Config ? 0 : 1];
	switch(id) {
		case 4:
			s = "MidiOut: ";
			s += synth->midiToggle() ? "On" : "Off";
			((Menu *) UIViews[UIView])->setLabel(id, s);
		break;
		default:
			v = synth->waves[id]->getShape() + 1;
			if(!(synth->waveOn & (1 << id))) {
				v = WaveShapeSquare;
				synth->waveOn |= (1 << id);
			}
			s = "Wave ";
			s += (id + 1);
			s += ": ";
			if(v > 3) {
				synth->waveOn &= ~(1 << id);
				s += "Off";
			} else {
				synth->waves[id]->setShape(v);
				s += WaveShapeNames[v];
			}
			((Menu *) UIViews[UIView])->setLabel(id, s);
	}
}

void onChange(byte pin, int read) {
	switch(UIView) {
		case UIViewSynth1:
		case UIViewSynth2:
			switch(pin) {
				case pot1Pin:
					synths[UIView]->setScale(map(read, 0, 1023, 0, synths[UIView]->numScales - 1));
				break;
				case pot2Pin:
					synths[UIView]->setOctave(map(read, 1023, 0, 0, synths[UIView]->numOctaves - 2));
				break;
				case photoResistorPin:
					if(!photoResistorEnabled || photoResistorCalibrate) return;
					read = constrain(map(constrain(read, photoResistorMin, photoResistorMax), photoResistorMin, photoResistorMax, synths[UIView]->selectedNote, synths[UIView]->selectedNote + (synths[UIView]->numNotes * 2)), 0, (synths[UIView]->numNotes * synths[UIView]->numOctaves) - 1);
					if(synths[UIView]->note == read) return;
					synths[UIView]->setNote(read);
			}
		break;
		case UIViewMenu:
			switch(pin) {
				case pot2Pin:
					sequencer.setTempo(map(read, 1023, 0, 60, 300));
				break;
				default:
					//Serial.print(pin, DEC);
					//Serial.print(": ");
					//Serial.println(read, DEC);
				break;
			}
	}
}

ISR(inputsInterrupt) {
	analogInputs.read();
	for(byte x=0; x<numSynths; x++) synths[x]->chainSawTick();
}

ISR(sequencerInterrupt) {
	sequencer.tick();
}

ISR(audioInterrupt) {
	int output = 127;
	for(byte x=0; x<numSynths; x++) output += synths[x]->output();
	audioOutput = constrain(output, 0, 255);
}
