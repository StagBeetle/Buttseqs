#pragma once

namespace drumSampler{

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

class sampleChannel{
	public:
		sampleChannel(AudioPlayMemory& obj, const int num);
		
		float checkCanPlay (smpl sampleToPlay);
		
		void play(smpl sampleToPlay, float volume);
		
		void stop();
		
	private:
		smpl currentSample = xx;
		AudioPlayMemory& playerObject;
		const int number;
	
};

int findChannelToPlayOn(smpl sample);

int findWhatChannelIsPlayingSample(smpl sample);

void stopAllSamples();

class drumUnit : public audio::audioUnit {
	public:
	
		void noteOn(int num, int velocity) override;
		
		void noteOff(int num) override ;
		
		void allNotesOff() override ;
};

extern drumUnit DRUM;

void begin();

} //end namespace drumSamples
