/**
 * @file sequencer.ino
 *
 * @mainpage Hardware Sequencer
 * 
 */
 
#include "TeensyDebug.h"
#pragma GCC optimize ("O0")
 
// #define USESERIAL

#include "compatibility.h"	//Stuff to make libraries work
#include "forwarddec.h"	//Forward declarations and global parameters
#include "scheduled.h"	//Scheduling library
#include "Utility.h"	//Little helpful tools 
//#include "Midi.h"	//All that MIDI shit
//#include "clockReceive.h"	//Midi clock receiving - faulty

#include "debug.h"	//debug routines

//Hardware:
#include "encoder.h"	//Rotary encoders

#include "src/audio.h"	//audio stuff
#include "src/drumSampler.h"	//drumSampler
#include "src/monoSynth.h"	//Basic Monosynth
#include "src/audio2.h"	//audio stuff that applies to all the engines

#include "mode.h"	//Declare the modes
#include "list.h"	//Drawing lists
#include "notifications.h"	//Popups
#include "card.h"	//SD card
#include "screen.h"	//Low level Screen functions
#include "modal.h"	//Modal popups
#include "blocks.h"	//Block-based storage used throughout
#include "patternStorage.h"	//How the patterns are saved in memory
#include "processes.h"	//Pattern Processes
#include "arrange.h"	//Memory stuff for arrangements
#include "sequencing.h"	//For organising and controlling sequence data
#include "forwarddec2.h"	//Forward declarations
#include "buttons.h"	//To check the buttons
#include "processData.h"	//The functions and intiialisers and parameter names for the processes
#include "MIDIInputOutput.h"	//MIDI ports

//Below here, they need .cpp files
#include "interface.h"	//The functions triggered by the buttons
#include "functionDescriptions.h"	//Description of Functions
#include "functionlists.h"	//Lists of functions with names for certains modes
#include "LED.h"	//Controlling the LEDs
#include "draw.h"	//Drawing things on the screen
#include "modeDefinitions.h"	//Define the modes
#include "screenvars.h"	//Define the screenvars
#include "midi_handlers.h"	//MIDI handlers

// jmp_buf env;

PROGMEM void setup(){
	debug.begin(Serial);
	
	#ifdef USESERIAL
	Serial.println("Booting up...");
	Serial.println("Intialising screen...");
	#endif
	scrn::begin();
	scrn::setTextColor({255,255,255});
	scrn::print("Lynxwave Buttseqs");
	scrn::print("Screen OK");
	#ifdef USESERIAL
	Serial.println("Intialising buttons...");
	#endif
	scrn::print("Intialising buttons...");
	buttons::begin(LEDfeedback::updateLEDs);
	// //pythonButtons::setup();
	
	// updateLEDs = LEDfeedback::updateLEDs;
	#ifdef USESERIAL
	Serial.println("Intialising card...");
	#endif
	scrn::print("Intialising card...");
	card::begin();
	#ifdef USESERIAL
	Serial.println("Intialising MIDI and sequencing...");
	#endif
	scrn::print("Intialising MIDI and sequencing...");
	MIDIhandlers::begin();
	MIDIports::begin(); // sort out new MIDI ports
	Sequencing::begin();
	
	interface::modeSelect::begin();
	#ifdef USESERIAL
	Serial.println("Intialising LEDs...");
	#endif
	scrn::print("Intialising LEDs...");
	LEDfeedback::begin();
	//Serial.println("Intialising Rotary Encoders...");
	//encoders::setup(modes::encoderAllocator);
	
//Add a load of test notes:
//This should load from an SD card
Sequencing::getTrack(0).addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::getTrack(0).addOrUpdateNote({38, 100, {0,0,12}}, {0, 1}, false);
Sequencing::getTrack(0).addOrUpdateNote({41, 100, {0,0,12}}, {0, 2}, false);
Sequencing::getTrack(0).addOrUpdateNote({45, 100, {0,0,12}}, {0, 3}, false);
Sequencing::getTrack(0).addOrUpdateNote({33, 100, {0,0,12}}, {0, 4}, false);
Sequencing::getTrack(0).addOrUpdateNote({38, 100, {0,0,12}}, {0, 5}, false);
Sequencing::getTrack(0).addOrUpdateNote({41, 100, {0,0,12}}, {0, 6}, false);
Sequencing::getTrack(0).addOrUpdateNote({45, 100, {0,0,12}}, {0, 7}, false);
Sequencing::getTrack(0).addOrUpdateNote({36, 100, {0,0,12}}, {0, 8}, false);
Sequencing::getTrack(0).addOrUpdateNote({38, 100, {0,0,12}}, {0, 9}, false);
Sequencing::getTrack(0).addOrUpdateNote({41, 100, {0,0,12}}, {0, 10}, false);
Sequencing::getTrack(0).addOrUpdateNote({50, 100, {0,0,12}}, {0, 11}, false);
Sequencing::getTrack(0).addOrUpdateNote({36, 100, {0,0,12}}, {0, 12}, false);
Sequencing::getTrack(0).addOrUpdateNote({38, 100, {0,0,12}}, {0, 13}, false);
Sequencing::getTrack(0).addOrUpdateNote({41, 100, {0,0,12}}, {0, 14}, false);
Sequencing::getTrack(0).addOrUpdateNote({57, 100, {0,0,12}}, {0, 15}, false);

Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 4}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 8}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 12}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 13}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 14}, false);
Sequencing::getTrack(9).addOrUpdateNote({36, 100, {0,0,12}}, {0, 15}, false);

Sequencing::getTrack(1).addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::getTrack(1).addOrUpdateNote({45, 100, {0,0,12}}, {0, 1}, false);
Sequencing::getTrack(1).addOrUpdateNote({48, 100, {0,0,12}}, {0, 2}, false);
Sequencing::getTrack(1).addOrUpdateNote({50, 100, {0,0,12}}, {0, 3}, false);
Sequencing::getTrack(1).addOrUpdateNote({55, 100, {0,0,12}}, {0, 4}, false);
Sequencing::getTrack(1).addOrUpdateNote({60, 100, {0,0,12}}, {0, 5}, false);

//Assign 
Sequencing::getTrack(9).setSoundEngine(1);
Sequencing::getTrack(0).setSoundEngine(2);

Sequencing::getTrack(0).setMIDIPort(1);

MIDIports::getMIDIInputSettings(3).toggleReceiveClock();
MIDIports::getMIDIInputSettings(3).toggleReceiveStartStop();

setSequencePriority();
#ifdef USESERIAL
Serial.println("Initialising Sound Functions...");
#endif
scrn::print("Initialising Sound Functions...");

audio::begin();
monoSynth::begin();
drumSampler::begin();


#ifdef USESERIAL
Serial.println("Ready");
#endif
scrn::print("Ready");

draw::startup();//draw cool graphics

}//End setup

void loop(){
	//audio::sgtl5000_1.volume(audio::usb2.volume() * interface::volume);
	buttons::loop(); // Get the data from the buttons
	//encoders::check();
	scheduled::checkFinishedEvents();
	MIDIports::readMIDI();
	LEDfeedback::sendLEDs();
	if(Sequencing::isSequencerSlow()){
		lg("slowFunc");
	}
	scrn::update();
	if(hasPrinted){//DRaw a blank line after the end of each loop to make things more clear.
		hasPrinted = false;
		Serial.println();
	}
}//end loop
