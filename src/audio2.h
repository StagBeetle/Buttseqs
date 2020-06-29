#ifndef audio2_h
#define audio2_h
namespace audio{

	const int numberOfSoundEngines = 3;
	
	audio::audioUnit* soundEngines[numberOfSoundEngines] = {
		nullptr	,
		&drumSampler::DRUM	,
		&monoSynth::monoSynth	,
	};
	
	const char* const soundEngineNames[numberOfSoundEngines] = {
		"none"	,
		"DrumBandit"	,
		"30.3"	,
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
#endif
