#ifndef processData_h
#define processData_h
//#include "patternStorage.h"

namespace process{

namespace f{

//Functions:
patMem::noteList none(volatile processStack* stack, patMem::position pos, dataArray data){
	patMem::noteList notes = stack->getTrack().getActivePattern().getNotesOnStep(pos);//Get the notes straight from the pattern
	return notes;
}
//Remember:
//Call getLastStage which will decrement the stack counter and give us the stage
//You must call getNotesOnStep
//The data array will be written 
patMem::noteList transpose(volatile processStack* stack, patMem::position pos, dataArray data){
	process_t lastStage = stack->getLastStage();
	patMem::noteList notes = lastStage.getNotesOnStep(stack, pos, lastStage.getParams());
	for(patMem::note& n: notes){
		int pitch = n.getPitch();
		pitch += utl::makeSigned(data[0]) * 12 ;
		pitch += utl::makeSigned(data[1]) * 1 ;
		//Keep pitches the same note:
		while(pitch < 0){
			pitch += 12;
		}
		while(pitch > 127){
			pitch -= 12;
		}
		n.setPitch(pitch);
	}
	return notes;
}

patMem::noteList shift(volatile processStack* stack, patMem::position pos, dataArray data){
	process_t lastStage = stack->getLastStage();
	patMem::noteList notes = lastStage.getNotesOnStep(
		stack,
		pos.offset(	utl::makeSigned(data[0]),
			utl::makeSigned(data[1]),
			utl::makeSigned(data[2])
			),
		lastStage.getParams()
	);
	return notes;
}

enum class arpStyle{
	up	,
	down	,
	upanddown	,
	random	,
	pattern	,
};

enum class arpSource{
	pattern	= 1,
	internal	= 2,
	MIDIIn	= 4,
};

namespace arpData{ enum AD {
		styleAndSource	= 0,	// arpStyle (4bits MSB is style and 4bits LSB is source)
		pattern	= 1,	// MSB of the patternBlock
		pattern2	= 2,	// LSB of the patternBlock
		octave	= 3,	//	
		length	= 4,	// Arp length
		steps	= 5,	// On or off for each step 0 - 7
		steps2	= 6,	// On or off for each step 8 - 15
		rate	= 7,	// in sixteenths 
		gate	= 8,	// gate for each arp
		counter	= 9,	// 
		pitches	= 10,	//
		lengths1	= 18,	//
		lengths2	= 26,	//
		lastPlayedNote	= 34,	// Which note was played last
	}; }
	
patMem::noteList arpeggiate(volatile processStack* stack, patMem::position pos, dataArray data){
	using namespace arpData;
	constexpr int arpNotes = 8;
	//Sequencing::track track =	stack->getTrack();
	/*
	//pitch	[8] held notes
	//lengths	[2x8] held lengths
	//lastNotePlayed	(1) 
	//arpPatternPos	(2) where the current arp pattern is, ++ each time
	//style	(1) what pattern_t / style to use (ignores pitch)
		pattern
		random	
		up
		down
		up/down
	//octave	(1) 
	//pattern	(2) arp pattern
	//steps	(2) 16
	//length	(1) 1-16
	//timing	(1) 1-16 how many sixteenths
	//Gate	(1) 
	//Input	(1) pattern, live, both
	*/
	process_t lastStage = stack->getLastStage();
	patMem::noteList notes = lastStage.getNotesOnStep(stack, pos, lastStage.getParams());
	
	//Add them to the internal noteOnStorage:
	for(auto& note: notes){
		bool hasPlacedNote = false;
		//Look for note:
		for(int i=0; i<arpNotes; i++){
			if(data[pitches+i] == note.getPitch()){
				data[lengths1+i] = note.getLengthInSubsteps() / 256;
				data[lengths2+i] = note.getLengthInSubsteps() % 256;
				hasPlacedNote = true;
				break;
			}
		}
		if(hasPlacedNote){break;}
		//Add note in empty spots:
		for(int i=0; i<arpNotes; i++){
			if(data[pitches+i] == 128){
				data[pitches+i] = note.getPitch();
				data[lengths1+i] = note.getLengthInSubsteps() / 256;
				data[lengths2+i] = note.getLengthInSubsteps() % 256;
				hasPlacedNote = true;
				break;
			}
		}
		if(hasPlacedNote){break;}
		//Add note in any that are due to finish soon
		//I could check thisin the for above but to keep it clean, I haven't
		uint16_t shortestValue = - 1; //Great value!
		int shortestPos = -1;
		for(int i=0; i<arpNotes; i++){
			uint16_t noteLength = data[lengths1+i] * 256 + data[lengths2+i];
			shortestValue = min(shortestValue, noteLength);
			if(shortestValue == noteLength){
				shortestPos = i;
			}
		}
		data[pitches+shortestPos] = note.getPitch();
		data[lengths1+shortestPos] = note.getLengthInSubsteps() / 256;
		data[lengths2+shortestPos] = note.getLengthInSubsteps() % 256;
	}
	
	const int twoHundredAndFiftySixthNotes = pos.getValue();
	//Every step (16 substeps)
	if(twoHundredAndFiftySixthNotes % 16 == 0){//This will trigger each master step
		data[counter]++;
		if(data[counter] % data[rate] == 0){
			//int lastNote = data[lastPlayedNote];
			std::vector<uint8_t> heldNotes;
			for(int i=0; i<arpNotes; i++){
				uint8_t pitch = data[pitches+i];
				if (pitch != 128){
					heldNotes.push_back(pitch);
				}
			}
			arpStyle style = static_cast<arpStyle>(data[styleAndSource] >> 4);
			//int noteToPlay = -1;
			switch (style){
				case arpStyle::up:
					// for(auto p: heldNotes){
						
					// }
					break;
				case arpStyle::down:
					break;
				case arpStyle::upanddown:
					break;
				case arpStyle::random:
					break;
				case arpStyle::pattern:
					break;
				
			}
			
		}
	}
	return notes;
}

}//End functions


const processType processes[] = {
	{"none" , 5 ,
		process::f::none,
		{0},
		{"none"}
	},
	{"transpose" , 5 ,
		process::f::transpose,
		{127, 127},
		{"octave", "semitone"}
	},
	{"shift" , 5 ,
		process::f::shift,
		{127, 127, 127},
		{"bar", "step", "substep"}
	},
	{"arpeggiate" , 5 ,
		process::f::arpeggiate,
		{0, 255, 255, 0, 16, 255, 255, 1, 50,
		128, 128, 128, 128, 128, 128, 128, 128, 
		0, 0, 0, 0, 0, 0, 0, 0},
		{"style", "pattern", "octave", "length", "steps", "rate", "gate"}
	},
};


}//End namespace
#endif