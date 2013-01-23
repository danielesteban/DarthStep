DarthStep
=========

An arduino Sampler/Synthesizer.

This started (an continues) as a learning process... but it turned out to be a pretty damn dubstep-ish music instrument. xPP

**Synth Features:**

- Multiple simultaneous waves (Square, Triangle, Saw or Sine).
- Multiples scales (intervals) pre-programed.
- Note selection potenciometer
- Quick pitch alteration light resistor
- ChainSaw
- Midi Out

**Sampler Features:**

- 9 Samples. 8Bits. 8khz.
- Editable 2 bars beats. 64 steps per sample (32 each bar).
- Variable BPM
- Multiple quantization presets.
- Midi Out
- I will add an SD on the near future for more samples, recording and loading/saving beats

Everything runs all together on an arduino mega with a 9V battery and is controlled via a graphical interface (in a 128x64 GLCD) and a few inputs.

The midi outs are on separate midi channels for convenience, but (like almost everything) the channel numbers are constants at the top of the arduino sketch.

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

You should have received a copy of the [GNU General Public License](LICENSE)
along with this program. If not, see <http://www.gnu.org/licenses/>.
