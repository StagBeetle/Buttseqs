#include "audio.h"
#include "drumSampler.h"
#include "monoSynth.h"

#include "audio2.h"

namespace audio{
	audio::audioUnit* soundEngines[numberOfSoundEngines] = {
		nullptr	,
		&drumSampler::DRUM	,
		&monoSynth::monoSynth	,
	};
	
	audio::audioUnit* getSoundEngine(const int num){
		if(num < numberOfSoundEngines){
			return soundEngines[num];
		}
		return nullptr;
	}
	
	const char* getSoundEngineName(const int num){ 
		if(num < numberOfSoundEngines){
			return soundEngineNames[num];
		}
		return "invalid";
	}
	
	bool isValidIndex(const int index){//Allows zero for none
		return (index < numberOfSoundEngines);
	}
	
	bool isValidEngine(const int index){//Does not allow zero
		return (index > 0 && index < numberOfSoundEngines);
	}
	
	void sendAllNotesOff(){
		for(int i = 1; i< numberOfSoundEngines; i++){
			soundEngines[i]->allNotesOff();
		}
	}

}//end namespace