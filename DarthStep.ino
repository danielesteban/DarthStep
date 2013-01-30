//Comment this defines out if you don't want that functionality (or don't have the hardware conected)
#define pot1Pin (A8) 
#define pot2Pin (A9)
#define photoResistorPin (A10)
#define accelerometerXPin (A11)
#define accelerometerYPin (A13)
#define accelerometerZPin (A14)

//Constants
const byte numSynths = 2, //changing this one will require some code changes (besides i don't think it will handle more than that).
	samplerMidiChannel = 1, //set this to wharever channel you want. 0 is all channels.
	synthsMidiChannels[numSynths] = {2, 3}, //same here.
	/* STOP EDITING HERE... if you don't know what you doin', of course ;{PP */
	UIViewSynth1 = 0,
	UIViewSynth2 = 1,
	UIViewSampler = 2,
	UIViewMenu = 3,
	UIViewIntro = 4,
	UIViewMixer = 5,
	UIViewSynthConfig = 6,
	UIViewSequenceLoader = 7,
	UIViewLicense = 8;

const unsigned int sampleRate = 8000;

#define audioInterrupt (TIMER2_COMPA_vect)
#define sequencerInterrupt (TIMER3_COMPA_vect)
#define audioOutput (PORTF)

//Lib
#if defined(pot1Pin) || defined(pot2Pin) || defined(photoResistorPin) || defined(accelerometerXPin) || defined(accelerometerYPin) || defined(accelerometerZPin)
	#include <AnalogInputs.h>
#endif
#include <Midi.h>
#include <Wave.h>
#include <UTFT.h>
#include <UTouch.h>
#include <SD.h>
#include <Directory.h>
#include <Menu.h>
#include "Synth.h"
#include "SynthConfig.h"
#include "Sampler.h"
#include "Mixer.h"
#include "Sequencer.h"
#include "Samples.h"
#include "Intro.h"
#include "License.h"

//Vars
byte orientation = LANDSCAPE,
	UIView = UIViewIntro,
	sequenceLoaderSynth;

bool photoResistorEnabled = 0,
	photoResistorCalibrate = 0,
	autoOrientation = 1,
	sdStatus = 0;

unsigned int photoResistorMax = 0,
	photoResistorMin = 1023;

unsigned long photoResistorCalibrateStart = 0;

#ifdef AnalogInputs_h
	void onChange(byte pin, int read);
	AnalogInputs analogInputs(onChange, 25);
	#define inputsInterrupt (TIMER4_COMPA_vect)
	#ifdef photoResistorPin
		void photoResistorOnChange(byte pin, int read);
		void photoResistorEnable(byte enabled);
	#else
		#define photoResistorEnable (NULL)
	#endif
	#if defined(accelerometerXPin) || defined(accelerometerYPin) || defined(accelerometerZPin)
		void accelerometerOnChange(byte pin, int read);
	#endif
#endif

Midi midi(Serial1);
Synth * synths[numSynths] = {
	new Synth(sampleRate, midi, synthsMidiChannels[0]),
	new Synth(sampleRate, midi, synthsMidiChannels[1])
};
Sampler * sampler = new Sampler(midi, samplerMidiChannel);
Sequencer sequencer(numSynths, synths, sampler);

UTFT tft(ITDB32S, 38, 39, 40, 41);
UTouch touch(42, 43, 44, 45, 46);

//UIViews
const byte numMenuItems = 5;
String menuItems[numMenuItems] = {"Sampler -->", "Synth 1 -->", "Synth 2 -->", "Mixer -->", "License -->"};

void menuOnClick(byte id);
void introOnTouch(byte id);

UI * UIViews[] = {
	(UI *) synths[0], //UIViewSynth1
	(UI *) synths[1], //UIViewSynth2
	(UI *) sampler, //UIViewSampler
	new Menu("DarthStep", numMenuItems, menuItems, menuOnClick), //UIViewMenu
	(UI *) new Intro(introOnTouch), //UIViewIntro
	NULL, //UIViewMixer
	NULL, //UIViewSynthConfig
	NULL, //UIViewSequenceLoader
	NULL //UIViewLicense
};

void setOrientation(byte o, bool force = false, bool redraw = true) {
	if(!force && redraw && (!UIViews[UIView]->availableOrientations[o] || !UIViews[UIView]->rendered)) return;
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
	if(!UIViews[UIView]->availableOrientations[orientation]) return setOrientation(orientation == LANDSCAPE ? PORTRAIT : LANDSCAPE, true);
	UIViews[UIView]->render(tft);
	UIViews[UIView]->rendered = true;
}

//main funcs
void setup() {
	randomSeed(analogRead(A13));

	tft.InitLCD(orientation);
	setUIView(UIView);

	#ifdef pot1Pin
		analogInputs.setup(pot1Pin);
	#endif
	#ifdef pot2Pin
		analogInputs.setup(pot2Pin);
	#endif
	#ifdef photoResistorPin
		analogInputs.setup(photoResistorPin, photoResistorOnChange);
	#endif
	#ifdef accelerometerXPin
		analogInputs.setup(accelerometerXPin, accelerometerOnChange);
	#endif
	#ifdef accelerometerYPin
		analogInputs.setup(accelerometerYPin, accelerometerOnChange);
	#endif
	#ifdef accelerometerZPin
		analogInputs.setup(accelerometerZPin, accelerometerOnChange);
	#endif

	touch.InitTouch(orientation);
	touch.setPrecision(PREC_HI);

	midi.begin();

	sdStatus = SD.begin();
	
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

	#ifdef AnalogInputs_h
		TCCR4A = 0;
		TCCR4B = 0;
		TCCR4B |= (1 << WGM42);
		TCCR4B |= (1 << CS40); //no prescaler
		OCR4A = (F_CPU / (sequencer.rate / 2)) - 1;
		TIMSK4 |= (1 << OCIE4A);
	#endif

	sei(); //allow interrupts

	//debug
	//Serial.begin(115200);
	//UIViews[UIViewSynthConfig] = new SynthConfig(synths[UIViewSynth1], photoResistorEnable);
	//setUIView(UIViewSynthConfig);
}

void screenMenuOnClick(byte id);

void loop(void) {
	if(!UIViews[UIView]->rendered) return;
	UIViews[UIView]->update();
	UIViews[UIView]->readTouch(tft, touch, orientation, screenMenuOnClick);
}

void sequenceLoaderOnClick(byte id) {
	Directory * dir = new Directory("/SEQS");
	file * f = dir->getFiles();

	byte c = -1;

	while(f != NULL) {
		strcmp(f->name, "LAST") != 0 && c++;
		if(c == id) break;
		f = f->next;
	}

	synths[sequenceLoaderSynth]->loadSequence(f->name);
	setUIView(sequenceLoaderSynth);

	delete dir;
}

void renderSequenceLoader() {
	Directory * dir = new Directory("/SEQS");
	file * f = dir->getFiles();

	byte count = 0,
		c = 0;

	while(f != NULL) { //this is really lame, but i'm kinda tired ;P
		strcmp(f->name, "LAST") != 0 && count++;
		f = f->next;
	}
	
	String filenames[count];
	
	f = dir->getFiles();
	while(f != NULL) {
		if(strcmp(f->name, "LAST") != 0) {
			filenames[c] = f->name;
			c++;
		}
		f = f->next;
	}

	delete dir;

	if(UIViews[UIViewSequenceLoader] != NULL) delete UIViews[UIViewSequenceLoader];
	UIViews[UIViewSequenceLoader] = new Menu("Load sequence", count, filenames, sequenceLoaderOnClick);
	sequenceLoaderSynth = UIView;
	setUIView(UIViewSequenceLoader);
}

void renderSynthConfig() {
	if(UIViews[UIViewSynthConfig] != NULL) delete UIViews[UIViewSynthConfig];
	UIViews[UIViewSynthConfig] = new SynthConfig(synths[UIView], photoResistorEnable);
	setUIView(UIViewSynthConfig);
}

void screenMenuOnClick(byte id) {
	if(UIView == UIViewIntro) return;
	if(id == 0) {
		if(UIView == UIViewMenu) return;
		return setUIView(UIViewMenu);
	}
	switch(UIView) {
		case UIViewMenu:
			switch(id) {
				case 4:
					autoOrientation = !autoOrientation; //Experimental!
			}
		break;
		case UIViewSynth1:
		case UIViewSynth2:
			switch(id) {
				case 1:
					synths[UIView]->midiToggle();
				break;
				case 2:
					if(!sdStatus) return;
					renderSequenceLoader();
					
					//synths[UIView]->saveSequence();
				break;
				case 3:
					if(synths[UIView]->sequencerStatus == 2) synths[UIView]->clearSequencer();
					else synths[UIView]->sequencerStatus++;
				break;
				case 4:
					renderSynthConfig();
			}
		break;
		case UIViewSynthConfig:
			switch(id) {
				#if defined(accelerometerXPin) || defined(accelerometerYPin) || defined(accelerometerZPin)
					case 3:
						((SynthConfig *) UIViews[UIViewSynthConfig])->toggleMode();
					break;
				#endif
				case 4:
					setUIView(((SynthConfig *) UIViews[UIViewSynthConfig])->_synth == synths[0] ? UIViewSynth1 : UIViewSynth2); //Lame!
					delete UIViews[UIViewSynthConfig];
					UIViews[UIViewSynthConfig] = NULL;
			}
		break;
		case UIViewSampler:
			switch(id) {
				case 1:
					sampler->midiToggle();
				break;
				case 2:
					sampler->toggleSteps();
				break;
				case 3:
					sampler->clearSample();
				break;
				case 4:
					sampler->clearSampler();
			}
		break;
		case UIViewSequenceLoader:
			switch(id) {
				case 4:
					setUIView(sequenceLoaderSynth);
					delete UIViews[UIViewSequenceLoader];
					UIViews[UIViewSequenceLoader] = NULL;
			}
	}
}

void menuOnClick(byte id) {
	switch(id) {
		case 0:
			setUIView(UIViewSampler);
		break;
		case 1:
			setUIView(UIViewSynth1);
		break;
		case 2:
			setUIView(UIViewSynth2);
		break;
		case 3:
			UIViews[UIViewMixer] == NULL && (UIViews[UIViewMixer] = new Mixer(numSynths, synths, sampler));
			setUIView(UIViewMixer);
		break;
		case 4:
			UIViews[UIViewLicense] == NULL && (UIViews[UIViewLicense] = new License());
			setUIView(UIViewLicense);
	}
}

void introOnTouch(byte id) {
	setUIView(UIViewMenu);
}

#ifdef AnalogInputs_h
	void onChange(byte pin, int read) {
		switch(UIView) {
			case UIViewSynth1:
			case UIViewSynth2:
				switch(pin) {
					#ifdef pot1Pin
						case pot1Pin:
							synths[UIView]->setScale(map(read, 1023, 0, 0, synths[UIView]->numScales - 1));
						break;
					#endif
					#ifdef pot2Pin
						case pot2Pin:
							synths[UIView]->setOctave(map(read, 0, 1023, 0, synths[UIView]->numOctaves - 2));
						break;
					#endif
				}
			break;
			case UIViewSampler:
				switch(pin) {
					#ifdef pot1Pin
						case pot1Pin:
							sampler->sampleQuantization[sampler->selectedSample] = pow(2, map(read, 1023, 0, 2, 5)) + 1;
						break;
					#endif
					#ifdef pot2Pin
						case pot2Pin:
							sampler->selectedSample = map(read, 0, 1023, 0, numSamples - 1);
					#endif
				}
			break;
			case UIViewMenu:
				switch(pin) {
					#ifdef pot2Pin
						case pot2Pin:
							sequencer.setTempo(map(read, 0, 1023, 60, 300));
					#endif
				}
		}
	}
	ISR(inputsInterrupt) {
		analogInputs.read();
	}
	#ifdef photoResistorPin
		void photoResistorOnChange(byte pin, int read) {
			if(!photoResistorEnabled) return;
			read = 1023 - read;
			if(photoResistorCalibrate) {
				if(photoResistorCalibrateStart == 0) {
					photoResistorCalibrateStart = millis();
					photoResistorMax = 0;
					photoResistorMin = 1023;
				}
				photoResistorMax < read && (photoResistorMax = read);
				photoResistorMin > read && (photoResistorMin = read);
				photoResistorCalibrateStart <= millis() - 2000 && (photoResistorCalibrateStart = photoResistorCalibrate = 0);
				return;
			} else if(UIView >= numSynths) return;
			if(synths[UIView]->axis[5] != 255) synths[UIView]->photoResistor(constrain(read, photoResistorMin, photoResistorMax), photoResistorMin, photoResistorMax);
		}

		void photoResistorEnable(byte enabled) {
			if(enabled && !photoResistorEnabled) photoResistorCalibrate = 1;
			photoResistorEnabled = enabled;
		}
	#endif
	#if defined(accelerometerXPin) || defined(accelerometerYPin) || defined(accelerometerZPin)
		void accelerometerOnChange(byte pin, int read) {
			bool ao = autoOrientation;

			#ifdef accelerometerXPin
				const int x = analogInputs.get(accelerometerXPin)->read;
			#else
				const int x = -1;
			#endif
			#ifdef accelerometerYPin
				const int y = analogInputs.get(accelerometerYPin)->read;
			#else
				const int y = -1;
			#endif
			#ifdef accelerometerZPin
				const int z = analogInputs.get(accelerometerZPin)->read;
			#else
				const int z = -1;
			#endif

			if(UIView < numSynths && (synths[UIView]->axis[2] != 255 || synths[UIView]->axis[3] != 255 || synths[UIView]->axis[4] != 255)) {
				synths[UIView]->accelerometer(x, y, z);
				ao = false;
			}

			if(!ao) return;
			if(y < 450) {
				if(orientation != LANDSCAPE) setOrientation(LANDSCAPE);
			} else if(orientation != PORTRAIT) setOrientation(PORTRAIT);
		}
	#endif
#endif

ISR(sequencerInterrupt) {
	sequencer.tick();
	for(byte x=0; x<numSynths; x++) synths[x]->chainSawTick();
}

ISR(audioInterrupt) {
	int output = 127;
	output += sampler->output();
	for(byte x=0; x<numSynths; x++) output += synths[x]->output();
	audioOutput = constrain(output, 0, 255);
}
