#ifndef monoSynth_h
#define monoSynth_h

#include <bitset>

namespace monoSynth{
	
using namespace audio;

class monoSynthUnit : public audioUnit {
	public:
		void noteOn(int num, int velocity) override;
		
		void noteOff(int num) override;
		
		void allNotesOff() override ;
		
	private:
		void checkToTriggerNoteOff() ;
		std::bitset<128> heldNotes;
};

void begin();

extern monoSynthUnit monoSynth;

} //end namespace drumSamples

#endif