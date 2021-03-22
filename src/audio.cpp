#ifndef audio_h
#define audio_h

#include <cmath>

#include "AudioFat.h"
#include "SdFat.h"
#include "audio.h"
#include <Wire.h>
#include <SPI.h>

#include <SerialFlash.h>


namespace audio{

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform2; //xy=64,589.6000366210938
AudioSynthWaveform       waveform1;      //xy=66,553.2000732421875
AudioPlayMemory          playMem1;       //xy=85.19999694824219,344.20001220703125
AudioPlayMemory          playMem3; //xy=85.19999694824219,422.20001220703125
AudioPlayMemory          playMem2;  //xy=86.19999694824219,383.20001220703125
AudioPlayMemory          playMem4;  //xy=86.19999694824219,461.20001220703125
AudioSynthWaveformDc     dc1;            //xy=179,694.2000579833984
AudioMixer4              mixerOsc; //xy=216.00001525878906,577.6000366210938
AudioEffectEnvelope      VCF_ADSR;      //xy=330.00001525878906,692.2000732421875
AudioMixer4              sampleMixer; //xy=352.20001220703125,406.20001220703125
AudioInputUSB            usb2;           //xy=382.20001220703125,288.20001220703125
AudioFilterStateVariable filter1;        //xy=448.00001525878906,579.2000732421875
AudioEffectEnvelope      VCA_ADSR; //xy=590.0000152587891,563.8000793457031
AudioMixer4              mixerR; //xy=813.2000122070312,440.20001220703125
AudioMixer4              mixerL;         //xy=816.2000122070312,376.20001220703125
AudioOutputI2S           i2s2;           //xy=1010.2000122070312,439.20001220703125
AudioOutputUSB           usb1;           //xy=1013.2000122070312,354.20001220703125
AudioConnection          patchCord1(waveform2, 0, mixerOsc, 1);
AudioConnection          patchCord2(waveform1, 0, mixerOsc, 0);
AudioConnection          patchCord3(playMem1, 0, sampleMixer, 0);
AudioConnection          patchCord4(playMem3, 0, sampleMixer, 2);
AudioConnection          patchCord5(playMem2, 0, sampleMixer, 1);
AudioConnection          patchCord6(playMem4, 0, sampleMixer, 3);
AudioConnection          patchCord7(dc1, VCF_ADSR);
AudioConnection          patchCord8(mixerOsc, 0, filter1, 0);
AudioConnection          patchCord9(VCF_ADSR, 0, filter1, 1);
AudioConnection          patchCord10(sampleMixer, 0, mixerL, 0);
AudioConnection          patchCord11(sampleMixer, 0, mixerR, 0);
AudioConnection          patchCord12(usb2, 0, mixerL, 1);
AudioConnection          patchCord13(usb2, 1, mixerR, 1);
AudioConnection          patchCord14(filter1, 0, VCA_ADSR, 0);
AudioConnection          patchCord15(VCA_ADSR, 0, mixerL, 2);
AudioConnection          patchCord16(VCA_ADSR, 0, mixerR, 2);
AudioConnection          patchCord17(mixerR, 0, usb1, 1);
AudioConnection          patchCord18(mixerR, 0, i2s2, 1);
AudioConnection          patchCord19(mixerL, 0, usb1, 0);
AudioConnection          patchCord20(mixerL, 0, i2s2, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=350.20001220703125,185.1999969482422
// GUItool: end automatically generated code

float getHzFromNoteNumber(float noteNumber, int frequencyOfA = 440){
	return frequencyOfA * std::pow (2.0, (noteNumber - 69) / 12.0);
};

PROGMEM void begin(){
	AudioMemory(256);
	sgtl5000_1.enable();
	sgtl5000_1.volume(0.8);
	
	mixerL.gain(0,1);
	mixerL.gain(1,1);
	mixerL.gain(2,0.3);
	mixerL.gain(3,1);
	mixerR.gain(0,1);
	mixerR.gain(1,1);
	mixerR.gain(2,0.3);
	mixerR.gain(3,1);
}

}// end namespace audio

#endif