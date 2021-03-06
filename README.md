<pre>
			      dP                     dP   dP                  dP                     
			      88                     88   88                  88                     
			.d888b88 .d8888b. 88d888b. d8888P 88d888b. .d8888b. d8888P .d8888b. 88d888b. 
			88'  `88 88'  `88 88'  `88   88   88'  `88 Y8ooooo.   88   88ooood8 88'  `88 
			88.  .88 88.  .88 88         88   88    88       88   88   88.  ... 88.  .88 
			`88888P8 `88888P8 dP         dP   dP    dP `88888P'   dP   `88888P' 88Y888P' 
			                                                                    88       
			                                                                    dP       
</pre>

An arduino Sampler/Synthesizer.

This started (an continues) as a learning process... but it turned out to be a pretty damn dubstep-ish music instrument. xPP

**Synth Features:**

- Two independent 3D synths. (possible even more)
- Multiple simultaneous waves per synth. (Square, Triangle, Saw or Sine)
- Multiple pre-programed scales (intervals).
- Octave/Scale selection.
- Pitch/Gain/ChainSaw alteration with touch sensor, accelerometer and/or photoresistor. (6 axes)
- Midi Out.

**Sampler Features:**

- 9 Samples. 8Bits. 8khz.
- Editable 2 bars beats. 64 steps per sample (32 each bar).
- Variable BPM.
- Multiple quantization presets.
- Midi Out.

It also has a 3 channel mixer UI with a gain slider and a mute button per channel.

Everything runs all together on an arduino mega with a 9V battery and is controlled via a graphical touch interface (in a 320x240 TFT) and a few inputs.

Both the synth and the sampler allow to record/save/load sequences into an SD Card.

The midi outs are on separate midi channels for convenience, but (like ohter config options) the channel numbers are constants at the top of the arduino sketch.

Inspiration (and some code & schematics) for this project came from:

- http://code.google.com/p/tinkerit/wiki/Auduino
- http://www.instructables.com/id/Electronic-Instrument

**License:**

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the [GNU General Public License](https://github.com/danielesteban/DarthStep/blob/master/LICENSE)
along with this program. If not, see <http://www.gnu.org/licenses/>.
