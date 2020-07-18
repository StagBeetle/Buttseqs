#ifndef monoSynth_h
#define monoSynth_h

#include <bitset>

namespace monoSynth{
	
using namespace audio;

class monoSynthUnit : public audioUnit {
	public:
		//void audioUnit() override ;
		// monoSynthUnit() override = default;
		
		void noteOn(int num, int velocity) volatile override{
			
			std::bitset<128> heldCopy;
			typecopy(heldCopy, heldNotes);

			heldCopy[num] = true;  //std::bitset does not support volatile?
			typecopy(heldNotes, heldCopy);
			
			float secondNoteOffset = 7.0;
			
			float frequency1 = getHzFromNoteNumber(num);
			float frequency2 = getHzFromNoteNumber(num + secondNoteOffset);
			
			waveform1.frequency(frequency1);
			waveform2.frequency(frequency2);
			VCA_ADSR.noteOn();
			VCF_ADSR.noteOn();
		}
		
		void noteOff(int num) volatile override{
			std::bitset<128> heldCopy;
			typecopy(heldCopy, heldNotes);
			
			heldCopy[num] = false;
			typecopy(heldNotes, heldCopy);
			
			checkToTriggerNoteOff();
		}
		
		void allNotesOff() volatile override {
			VCA_ADSR.noteOff();
			VCF_ADSR.noteOff();
		}
		
	private:
		void checkToTriggerNoteOff() volatile {
			
			std::bitset<128> heldCopy;
			typecopy(heldCopy, heldNotes);
			
			if(heldCopy.count() == 0){
				VCA_ADSR.noteOff();
				VCF_ADSR.noteOff();
			}
		}
		std::bitset<128> heldNotes;
};

void setup(){
	waveform1.begin(1, 440, WAVEFORM_SQUARE);
	waveform2.begin(1, 440, WAVEFORM_SQUARE);
	mixerOsc.gain(0, 0.5);
	mixerOsc.gain(1, 0.5);
	
	dc1.amplitude	(1);
	
	VCF_ADSR.delay	(0	);
	VCF_ADSR.attack	(20	);
	VCF_ADSR.hold	(0	);
	VCF_ADSR.decay	(200	);
	VCF_ADSR.sustain	(0.2	);
	VCF_ADSR.release	(20	);
	
	VCA_ADSR.delay	(0	);
	VCA_ADSR.attack	(10	);
	VCA_ADSR.hold	(0	);
	VCA_ADSR.decay	(200	);
	VCA_ADSR.sustain	(0.2	);
	VCA_ADSR.release	(50	);
	
	filter1.frequency(100);
	filter1.resonance(2);
	filter1.octaveControl(3);
	
	// When controlled by a signal, the equation for the filter frequency is:
	// F = Fcenter * 2^(signal * octaves)
}

monoSynthUnit monoSynth;

} //end namespace drumSamples

#endif