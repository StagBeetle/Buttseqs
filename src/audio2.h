#ifndef audio2_h
#define audio2_h

#include "audio.h"
#include "drumSampler.h"
#include "monoSynth.h"

namespace audio{

	const int numberOfSoundEngines = 3;
	
	const char* const soundEngineNames[numberOfSoundEngines] = {
		"none"	,
		"DrumBandit"	,
		"30.3"	,
	};
	
	audio::audioUnit* getSoundEngine(const int num);
	
	const char* getSoundEngineName(const int num);
	
	bool isValidIndex(const int index);//Allows zero for none
	
	bool isValidEngine(const int index);//Does not allow zero
	
	void sendAllNotesOff();

}//end namespace
#endif
