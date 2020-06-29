#include "audio.h"	
#include "drumSampler.h"
#include "buttonsSimple.h"

#include <vector>

unsigned __exidx_start;
unsigned __exidx_end;



template <typename T>
void lg(T toPrint){
	Serial.print(toPrint);
}

struct noteRec{
	int note;
	int velocity;
	noteRec (int n=0, int v=0) : note(n) , velocity(v) {}
};

struct step{
	std::vector<noteRec> notes;
	
	step (std::vector<noteRec> c_notes) : notes(c_notes.begin(), c_notes.end()) {}
};

const int patternLength = 16;
step timeLine[patternLength] = {
	{{{36, 99},{49, 60}}},
	{{{42, 1}}},
	{{{42, 67}}},
	{{{42, 120}}},
	
	{{{38, 99}}},
	{{{42, 33}}},
	{{{42, 67}}},
	{{{42, 99}}},
	
	{{{36, 99}}},
	{{{42, 33}}},
	{{{42, 67}}},
	{{{42, 99},{56, 60}}},
	
	{{{38, 99}}},
	{{{42, 33}}},
	{{{46, 67}}},
	{{{42, 99}}},
};
	
int timePoint = 0;

const int LEDpin = 13;
long long timeAtLast = 0;
long int stepLength = 200;

int activeNote = 36;

bool isPlaying = false;

void setActiveNote(int n){
	activeNote = 36 + n;
}

void alterStep(int s){
	//bool hasDeleted = false;
	int counter = 0;
	int noteToDelete = -1;
	for(auto& noteR : timeLine[s].notes){
		if(noteR.note == activeNote){
			noteToDelete = counter;
		}
		counter++;
	}
	if(noteToDelete >= 0){
		timeLine[s].notes.erase(timeLine[s].notes.begin()+noteToDelete);
	} else {
		timeLine[s].notes.push_back({activeNote,100});
	}
}

void controlButtons(int c){
	switch(c){
		default:
			break;
		case 0:
			isPlaying = true;
			break;
		case 1:
			isPlaying = false;
			break;
		
	}
}

void setup(){
	audio::setup();
	buttonsSimple::setup();
	pinMode(LEDpin, OUTPUT);
	
	buttonsSimple::buttonFunctions [buttonsSimple::keySet::step][buttonsSimple::buttonEvent::press] = alterStep;
	buttonsSimple::buttonFunctions [buttonsSimple::keySet::note][buttonsSimple::buttonEvent::press] = setActiveNote;
	buttonsSimple::buttonFunctions [buttonsSimple::keySet::extra][buttonsSimple::buttonEvent::press] = controlButtons;
	
}



void loop(){
	//Control 
	while(Serial.available()){
		char controlChar = Serial.read();
		if(controlChar == 'p'){isPlaying = true;}
		if(controlChar == 's'){isPlaying = false;}
		lg(controlChar);
	}
		
	//Volume adjust
	audio::sgtl5000_1.volume(audio::usb2.volume());
	
	//Button loop
	buttonsSimple::loop();
	
	//isPlaying = buttonsSimple::getButtonHeld(77);
	
	//play
	if(isPlaying){
		if(timeAtLast + stepLength < millis()){
			
			for(noteRec n : timeLine[timePoint%16].notes){
				digitalWrite(LEDpin, HIGH);
				drumSampler::DRUM.noteOn(n.note, n.velocity);
				digitalWrite(LEDpin, LOW);
			}
			
			timeAtLast = millis();
			timePoint ++;
		}
	}
};
