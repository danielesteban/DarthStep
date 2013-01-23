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

**License:**

- 9 Samples. 8Bits. 8khz.
- Editable 2 bars beats. 64 steps per sample (32 each bar).
- Variable BPM
- Multiple quantization presets.
- Midi Out
- I will add an SD on the near future for more samples, recording and loading/saving beats

Everything runs all together on an arduino mega with a 9V battery and is controlled via a graphical interface (in a 128x64 GLCD) and a few inputs.

The midi outs are on separate midi channels for convenience, but (like almost everything) the channel numbers are constants at the top of the arduino sketch.

**Sampler Features:**




Inspiration (and some code & schematics) for this project came from:

- http://code.google.com/p/tinkerit/wiki/Auduino
- http://www.instructables.com/id/Electronic-Instrument
