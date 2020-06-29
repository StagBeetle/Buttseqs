#include "compatibility.h"	//Stuff to make libraries work
#include "forwarddec.h"	//Forward declarations and global parameters
#include "scheduled.h"	//Scheduling library
#include "Utility.h"	//Little helpful tools 
//#include "Midi.h"	//All that MIDI shit
#include "clockReceive.h"	//Midi clock receiving - faulty



#include "src/audio.h"	//audio stuff
#include "src/drumSampler.h"	//drumSampler
#include "src/monoSynth.h"	//Basic Monosynth
#include "src/audio2.h"	//audio stuff that applies to all the engines

#include "encoder.h"	//Rotary encoders
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
#include "interface.h"	//The functions triggered by the buttons
#include "functionDescriptions.h"	//Description of Functions
#include "functionlists.h"	//Lists of functions with names for certains modes
#include "LED.h"	//Controlling the LEDs
#include "draw.h"	//Drawing things on the screen
#include "modeDefinitions.h"	//Define the modes
#include "screenvars.h"	//Define the screenvars
#include "midi_handlers.h"	//MIDI handlers
#include "debug.h"	//Temporary debug routines

// jmp_buf env;


const bool enableStartup = false; //If program waits for Python input before starting

void setup(){
	if(enableStartup){
		Serial.begin(9600); //USB so disregard number
		while(true){//Wait for input before starting
			if (Serial.available()) {
				uint8_t incomingByte = Serial.read();
				if (incomingByte == 7){
					break;
				}
			}
		}
	}
	Serial.println("Booting up...");
	Serial.println("Intialising buttons...");
	buttons::setup(LEDfeedback::updateLEDs);
	// //pythonButtons::setup();
	
	// updateLEDs = LEDfeedback::updateLEDs;
	
	
	// draw::startup();..draw cool graphics
	Serial.println("Intialising card...");
	card::setup();
	Serial.println("Intialising MIDI and sequencing...");
	MIDIhandlers::setup();
	MIDIports::setup(); // sort out new MIDI ports
	Sequencing::setup();
	
	interface::modeSelect::setup();
	Serial.println("Intialising screen...");
	scrn::setup();
	Serial.println("Intialising LEDs...");
	LEDfeedback::setup();
	Serial.println("Intialising Rotary Encoders...");
	encoders::setup(modes::encoderAllocator);
	
//Add a load of test notes:
//This should load from an SD card
Sequencing::trackArray[0].addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::trackArray[0].addOrUpdateNote({38, 100, {0,0,12}}, {0, 1}, false);
Sequencing::trackArray[0].addOrUpdateNote({41, 100, {0,0,12}}, {0, 2}, false);
Sequencing::trackArray[0].addOrUpdateNote({45, 100, {0,0,12}}, {0, 3}, false);
Sequencing::trackArray[0].addOrUpdateNote({33, 100, {0,0,12}}, {0, 4}, false);
Sequencing::trackArray[0].addOrUpdateNote({38, 100, {0,0,12}}, {0, 5}, false);
Sequencing::trackArray[0].addOrUpdateNote({41, 100, {0,0,12}}, {0, 6}, false);
Sequencing::trackArray[0].addOrUpdateNote({45, 100, {0,0,12}}, {0, 7}, false);
Sequencing::trackArray[0].addOrUpdateNote({36, 100, {0,0,12}}, {0, 8}, false);
Sequencing::trackArray[0].addOrUpdateNote({38, 100, {0,0,12}}, {0, 9}, false);
Sequencing::trackArray[0].addOrUpdateNote({41, 100, {0,0,12}}, {0, 10}, false);
Sequencing::trackArray[0].addOrUpdateNote({50, 100, {0,0,12}}, {0, 11}, false);
Sequencing::trackArray[0].addOrUpdateNote({36, 100, {0,0,12}}, {0, 12}, false);
Sequencing::trackArray[0].addOrUpdateNote({38, 100, {0,0,12}}, {0, 13}, false);
Sequencing::trackArray[0].addOrUpdateNote({41, 100, {0,0,12}}, {0, 14}, false);
Sequencing::trackArray[0].addOrUpdateNote({57, 100, {0,0,12}}, {0, 15}, false);

Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 4}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 8}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 12}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 13}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 14}, false);
Sequencing::trackArray[9].addOrUpdateNote({36, 100, {0,0,12}}, {0, 15}, false);

Sequencing::trackArray[1].addOrUpdateNote({36, 100, {0,0,12}}, {0, 0}, false);
Sequencing::trackArray[1].addOrUpdateNote({45, 100, {0,0,12}}, {0, 1}, false);
Sequencing::trackArray[1].addOrUpdateNote({48, 100, {0,0,12}}, {0, 2}, false);
Sequencing::trackArray[1].addOrUpdateNote({50, 100, {0,0,12}}, {0, 3}, false);
Sequencing::trackArray[1].addOrUpdateNote({55, 100, {0,0,12}}, {0, 4}, false);
Sequencing::trackArray[1].addOrUpdateNote({60, 100, {0,0,12}}, {0, 5}, false);

//Assign 
Sequencing::trackArray[9].setSoundEngine(1);
Sequencing::trackArray[0].setSoundEngine(2);

Sequencing::trackArray[0].setMIDIPort(1);

MIDIports::inputs[3].toggleReceiveClock();
MIDIports::inputs[3].toggleReceiveStartStop();

sequence.priority(64);

Serial.println("Initialising Sound Functions...");

audio::setup();
monoSynth::setup();
drumSampler::setup();

Serial.println("Ready");

//scheduled::newEvent(scheduled::lOE::listOfEvents::misc, []{modes::switchToMode(modes::settings, true);}, 1000);

}//End setup

// uint8_t cycleNum = 0;
//long timeSinceLastSequence = 0;

void loop(){
	
	audio::sgtl5000_1.volume(audio::usb2.volume());
	
	//Time checking:
	static long int longestTime = 0;
	static long int longestSeqTime = 0;
	static long int longestTimeSinceLastSequencer = 0;
	long int startTime = micros();
	//Sequencing::checkPlaying();
	long int endSeqTime = micros();
	long int seqTime = endSeqTime - startTime;
	longestSeqTime = (longestSeqTime > seqTime) ? longestSeqTime : seqTime;
	longestTimeSinceLastSequencer = (longestTimeSinceLastSequencer > Sequencing::getTimeSinceLastSequence()) ? longestTimeSinceLastSequencer : Sequencing::getTimeSinceLastSequence();
	
	
	debug::serialButtonCheck();//Check serial for simulated buttno presses
	
	//pythonButtons::checkSerial(); //Simulating button Presses
	buttons::loop(); // Get the data from the buttons
	encoders::check();
	scheduled::checkFinishedEvents();
	MIDIports::readMIDI();
	//comms:read();
	long int endTime = micros();
	long int timePassed = endTime - startTime;
	longestTime = (longestTime > timePassed) ? longestTime : timePassed;
	static int timeAtLastSerialSend = 0;
	if(millis() - timeAtLastSerialSend > 1000){
		// long averageDuration_tmp;
		// typecopy(averageDuration_tmp, Sequencing::averageDuration);
		// Serial.println(averageDuration_tmp);
		
		// for(int i = 0; i<10; i++){
			// long time_tmp;
			// typecopy(time_tmp, Sequencing::times[i]);
			// lgc(time_tmp);
			// lgc(" ");
		// }
		
		timeAtLastSerialSend = millis();
	}
	if(Sequencing::slowFunc){
		Sequencing::slowFunc = false;
		// Serial.print("!");
		// Serial.println(Sequencing::slowTime);
	}
	
	//tft.updateScreenAsync(); 
	
	//DO BACKUP RUN SEQUENCING HERE
}//end loop
