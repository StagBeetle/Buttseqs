#ifndef drumSampler_h
#define drumSampler_h

#include "AudioSampleBd.h"
#include "AudioSampleSnare.h"
#include "AudioSampleSnare2.h"
#include "AudioSampleHatc.h"
#include "AudioSampleHato.h"
#include "AudioSampleCowbell.h"
#include "AudioSampleCrash.h"

//#include <map>

const unsigned int AudioSampleNull[1] = {0x01000000};

namespace drumSampler{

const unsigned int* samples[] = {
	AudioSampleNull,
	AudioSampleBd,
	AudioSampleSnare,
	AudioSampleSnare2,
	AudioSampleHatc,
	AudioSampleHato,
	AudioSampleCowbell,
	AudioSampleCrash,
};

enum smpl : uint8_t{
	xx = 0, //No sample
	BD = 1,
	SN = 2,
	S2 = 3,
	CH = 4,
	OH = 5,
	CB = 6,
	CR = 7,
};

smpl MIDImap [128] = {xx}; //map MIDI note number to sample.;

void fillMap(){
	MIDImap[36]=BD;
	MIDImap[38]=SN;
	MIDImap[40]=S2;
	MIDImap[42]=CH;
	MIDImap[46]=OH;
	MIDImap[49]=CR;
	MIDImap[56]=CB;
}


const int numberOfChannels = 4;

class sampleChannel{
	public:
		sampleChannel(AudioPlayMemory& obj, const int num) : playerObject(obj), number(num) {}
		
		float checkCanPlay (smpl sampleToPlay){
			if(currentSample == sampleToPlay){
				return 2.0;
			}
			else if (!playerObject.isPlaying()){
				return 1.0;
			} else {
				return 
					(float) playerObject.positionMillis() /
					(float) playerObject.lengthMillis() ;
			}
		}
		
		void play(smpl sampleToPlay, float volume){
			audio::sampleMixer.gain(number, volume);
			playerObject.play(samples[sampleToPlay]);
			currentSample = sampleToPlay;
		}
		
		void stop(){
			playerObject.stop();
		}
		
	private:
		smpl currentSample = xx;
		AudioPlayMemory& playerObject;
		const int number;
	
};

sampleChannel audioChannels [numberOfChannels] = {
	{audio::playMem1, 0},
	{audio::playMem2, 1},
	{audio::playMem3, 2},
	{audio::playMem4, 3},
};



int findChannelToPlayOn(smpl sample){
	int channelToUse = 0;
	float value = -1.0;
	
	for(int i = 0; i< numberOfChannels; i++){
		float choice = audioChannels[i].checkCanPlay(sample);
		if(choice > value){
			channelToUse = i;
			value = choice;
		}
		// Serial.println(choice);
		// Serial.println(channelToUse);
	}
	// Serial.println(channelToUse);
	// Serial.println();
	return channelToUse;
}

int findWhatChannelIsPlayingSample(smpl sample){
	int channelToUse = -1;
	float value = -1.0;
	
	for(int i = 0; i< numberOfChannels; i++){
		float choice = audioChannels[i].checkCanPlay(sample);
		channelToUse = choice > value ? i : channelToUse;
	}
	if (value == 2.0){return channelToUse;}
	else return -1; //If the sample is not playing
}

void stopAllSamples(){
	for (auto& chan: audioChannels){
		chan.stop();
	}
}
	

class drumUnit : public audio::audioUnit {
	public:
		//void audioUnit() override ;
		//monoSynthUnit() override = default;
		
		void noteOn(int num, int velocity) volatile override{
			if(MIDImap[num] != xx){
				smpl sample = MIDImap[num];
				int channel = findChannelToPlayOn(sample);
				audioChannels[channel].play(sample, (float)velocity/127.0);
			}
		}
		
		void noteOff(int num) volatile override { 
			if(MIDImap[num] != xx){
				smpl sample = MIDImap[num];
				int channel = findWhatChannelIsPlayingSample(sample);
				if(channel >= 0){
					audioChannels[channel].stop();
				}
			}
		}
		
		void allNotesOff() volatile override {
			stopAllSamples();	
		}
};

drumUnit DRUM;

void setup(){
	fillMap();
}

} //end namespace drumSamples

#endif