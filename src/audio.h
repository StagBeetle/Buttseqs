#ifndef audio_h
#define audio_h

#include <AudioFat.h>

namespace audio{
	
extern AudioSynthWaveform       waveform2	; //xy=64,589.6000366210938
extern AudioSynthWaveform       waveform1	;      //xy=66,553.2000732421875
extern AudioPlayMemory          playMem1	;       //xy=85.19999694824219,344.20001220703125
extern AudioPlayMemory          playMem3	; //xy=85.19999694824219,422.20001220703125
extern AudioPlayMemory          playMem2	;  //xy=86.19999694824219,383.20001220703125
extern AudioPlayMemory          playMem4	;  //xy=86.19999694824219,461.20001220703125
extern AudioSynthWaveformDc     dc1	;            //xy=179,694.2000579833984
extern AudioMixer4              mixerOsc	; //xy=216.00001525878906,577.6000366210938
extern AudioEffectEnvelope      VCF_ADSR	;      //xy=330.00001525878906,692.2000732421875
extern AudioMixer4              sampleMixer	; //xy=352.20001220703125,406.20001220703125
extern AudioInputUSB            usb2	;           //xy=382.20001220703125,288.20001220703125
extern AudioFilterStateVariable filter1	;        //xy=448.00001525878906,579.2000732421875
extern AudioEffectEnvelope      VCA_ADSR	; //xy=590.0000152587891,563.8000793457031
extern AudioMixer4              mixerR	; //xy=813.2000122070312,440.20001220703125
extern AudioMixer4              mixerL	;         //xy=816.2000122070312,376.20001220703125
extern AudioOutputI2S           i2s2	;           //xy=1010.2000122070312,439.20001220703125
extern AudioOutputUSB           usb1	;           //xy=1013.2000122070312,354.20001220703125
extern AudioConnection          patchCord1	;
extern AudioConnection          patchCord2	;
extern AudioConnection          patchCord3	;
extern AudioConnection          patchCord4	;
extern AudioConnection          patchCord5	;
extern AudioConnection          patchCord6	;
extern AudioConnection          patchCord7	;
extern AudioConnection          patchCord8	;
extern AudioConnection          patchCord9	;
extern AudioConnection          patchCord10	;
extern AudioConnection          patchCord11	;
extern AudioConnection          patchCord12	;
extern AudioConnection          patchCord13	;
extern AudioConnection          patchCord14	;
extern AudioConnection          patchCord15	;
extern AudioConnection          patchCord16	;
extern AudioConnection          patchCord17	;
extern AudioConnection          patchCord18	;
extern AudioConnection          patchCord19	;
extern AudioConnection          patchCord20	;
extern AudioControlSGTL5000     sgtl5000_1	;     //xy=350.20001220703125,185.1999969482422
// GUItool: end automatically generated code

class audioUnit {
	public:
		// virtual audioUnit();
		virtual void noteOn(int num, int velocity) volatile;
		virtual void noteOff(int num) volatile;
		virtual void allNotesOff() volatile;
};
extern float getHzFromNoteNumber(float noteNumber, int frequencyOfA = 440);
extern void setup();

}// end namespace audio

#endif