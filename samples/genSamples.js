#!/usr/local/bin/node

var fs = require('fs'),
	samples = fs.readdirSync('./wav'),
	sampleRate = 8000,
	sampleBits = 8,
	soxEffects = ['vol', '0.988'],
	wav2cBinary = '../../../Downloads/wav2c/wav2c',
	samplesFile = fs.createWriteStream('../Samples.h'),
	samplesCFile = fs.createWriteStream('../Samples.cpp'),
	sampleNames = [],
	numSamples = samples.length;

function genSamples() {
	if(!samples.length) {
		samplesFile.write("const unsigned int sampleSize = 5000;\n\nconst byte numSamples = " + numSamples + ";\n\nconst String sampleNames[numSamples] = {");
		sampleNames.forEach(function(name, i) {
			i > 0 && samplesFile.write(", ");
			samplesFile.write('"' + name + '"');
		});
		samplesFile.write("};\n\nextern const unsigned char * samples[numSamples];");
		samplesCFile.write("\nconst unsigned char * samples[numSamples] = {");
		sampleNames.forEach(function(name, i) {
			i > 0 && samplesCFile.write(", ");
			samplesCFile.write(name);
		});
		samplesCFile.write("};\n");
		samplesFile.end("\n\n#endif\n");

		return console.log("Done!");
	}
	f = samples.shift();
	if(f.substr(f.lastIndexOf('.') + 1) == 'wav') {
		var fn = f.substr(0, f.length - 4),
			sox = require('child_process').spawn('sox', [
				'./wav/' + f,
				'-c1',
				'-r' + sampleRate,
				'-eunsigned-integer',
				'-b' + sampleBits,
				'./wav/' + sampleRate + '-' + f
			].concat(soxEffects));

		sox.on('exit', function (code) {
			if(code !== 0) return;
			var wav2c = require('child_process').spawn(wav2cBinary, [
				'./wav/' + sampleRate + '-' + f,
				'./wav/' + fn + '.h',
				fn
			]);

			wav2c.on('exit', function (code) {
				fs.unlinkSync('./wav/' + sampleRate + '-' + f);
				if(code !== 0) return;

				var w = fs.readFileSync('./wav/' + fn + '.h', 'utf8');
				w = w.substr(w.indexOf("={") + 2);
				w = w.substr(0, w.indexOf(", }"));
				//w = eval("new Buffer([" + w + "])");
				//fs.writeFileSync('./smp/' + fn + '.smp', w);
				samplesCFile.write("prog_uchar " + fn.toUpperCase() + "[] PROGMEM = {");
				samplesCFile.write(w);
				samplesCFile.write("};\n");
				sampleNames.push(fn.toUpperCase());
				fs.unlinkSync('./wav/' + fn + '.h');
				console.log(fn);
				genSamples();
			});

			wav2c.stderr.on('data', function (data) {
				console.log('wav2c stderr: ' + data);
			});
		});

		sox.stderr.on('data', function (data) {
			console.log('sox stderr: ' + data);
		});
	} else genSamples();
}

samplesFile.write("#ifndef Samples_h\n" +
"#define Samples_h\n" +
"\n" +
"#include \"Arduino.h\"\n" +
"#include <avr/pgmspace.h>\n\n");
samplesCFile.write("#include \"Samples.h\"\n\n");

genSamples();
