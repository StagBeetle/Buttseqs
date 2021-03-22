//Most of the general functions that are triggered by the keys
#ifndef interface_h
#define interface_h

#include <bitset>
#include "scheduled.h"
#include "sequencing.h"
#include "setVarAfterDelay.h"
#include "Utility.h"
#include "list.h"


namespace interface{
	bool isShiftHeld = false;
	uint8_t keyboardOctave = 3;
	enum class editPriorityModes : uint8_t {noteFirst = 0, stepFirst = 1};
	uint8_t viewBar = 0;
	uint8_t editTrack = 0;

	std::bitset<128> editPitches;

	int8_t editStepForSubstep = -1;
	
	/*EXTMEM*/ std::bitset<gc::numberOfSubsteps> stepSelection;
	
	bool applyOnEntirePattern = false; //For process editing

	uint8_t editVelocity = 100;
	patMem::position editLength = {0,0,12};
	bool editAccent = false;
	bool editLegato = false;
	
	bool recording = false;
	
	bool isRecording(){
		return recording;
	}
	
	float volume = 1.0;
	
namespace settings{
	editPriorityModes editPriority = editPriorityModes::stepFirst;
	int advanceSteps = 16;
	bool notePreview = false;
	unsigned int doubleClickSpeed = 250; //milliseconds
	bool useSharpNotes = true; //Whether to use sharps or flats
	bool useFancyLEDChaser = false; //Rubbish feature
	bool pianoRollFollowPlay = false; //Should the piano roll on screen follow the active bar
	bool editSubstepsAlso = false; //Whether the normal editStep mode should edit every substep.
	bool jumpToActivePattern = true; //Automatically go to the active pattern in the pattern menu
	unsigned int recordPriorityCutoff = 1000; //Used but not well.
	bool silenceNotesWhenRecording = false; //Should playing the keyboard silence the playing pattern
	bool recordOnlyFromExternal = false;
	uint8_t screenBrightness = 255;
	bool showHelp = true;
	bool displayProcessedPatternsInPianoRoll = true;
	bool isInSaveMode = true; //False for load;
	bool clearNameBufferOnEnter = true;
	bool saveToLastLocation = true; //Else ask
	bool overwriteExisting = false; //Else overwrite
	bool clearNRPNafterEach = false;
	}//End settings
	
	int8_t gPFK(const uint8_t key){//GetPitchFromKey //REturn -1 for error
		int pitch = 12 * keyboardOctave + key;
		return (pitch < 128) ? pitch : pitch - 12;
	}
	
	bool inSubstep(){
		//return modes::checkActive({{modes::substep , modes::substepEdit}});
		return editStepForSubstep != -1;
	}
	
	// bool isEditingSubsteps(){
		// return settings::editSubstepsAlso && !inSubstep();
	// }
	
	inline void forEachPitch(std::function<void(uint8_t)> &&callback){
		//return;
		//unsigned int numberOfPitches = editPitches.count();
		for(uint8_t i = 0; i < 128; i++){
			if(editPitches[i]){
				// lg(i);
				callback(i);
			}
		}
	}
		
	inline void forEachStep(std::function<void(patMem::position)> &&callback, const bool applyToAll = false){
		for(uint16_t i = 0; i < gc::numberOfSubsteps; i++){
			if(stepSelection[i] || applyToAll){
				patMem::position pos = {i};
				callback(pos);
			}
		}
	}
		
	inline void forEachStepAndPitch(std::function<void(uint8_t, patMem::position)> &&callback){
		forEachStep([&callback](patMem::position s){
			forEachPitch([&callback, s](uint8_t p){
				callback(p, s);
			});
		});
	}
	
	
	//For LEDs and things that actually need the buttons mapped
	inline void forEachStepButton(std::function<void(patMem::position, int)> &&callback){
		int stepIncrementer = inSubstep() ? 1 : 16;
		int stepOffset = inSubstep() ? editStepForSubstep * 16 : 0;
		int button = 0;
		for(uint16_t i = viewBar * 16 + stepOffset; button < 16; i += stepIncrementer){
			if(stepSelection[i]){
				patMem::position pos = {i};
				callback(pos, button);
			}
			button++;
		}
	}
	
	//For LEDs and things that actually need the buttons mapped
	inline void forEachStepButtonWithSubstep(std::function<void(patMem::position, int, int)> &&callback){
		int stepIncrementer = inSubstep() ? 1 : 16;
		int stepOffset = inSubstep() ? editStepForSubstep * 16 : 0;
		int button = 0;
		int substepLimit = settings::editSubstepsAlso ? 1 : 16;
		for(uint16_t i = viewBar * 16 + stepOffset; button < 16; i += stepIncrementer){
			for(int j = 0; j<substepLimit; j++){
				if(stepSelection[i]){
					patMem::position pos = {i};
					callback(pos, button, j);
				}
			}
			button++;
		}
	}
	
namespace editNotes{
	void setEditPitch(int note){
		int newPitch = gPFK(note);
		if (newPitch < 128){//Ignores the top few notes of the keyboard
			if (!interface::isShiftHeld){
				editPitches.reset();
				editPitches.set(newPitch);
				}
			else{//If shift held
				editPitches.flip(newPitch);
				if(editPitches.count() == 0){
					editPitches.set();
					}
				}
			}
		draw::drawPianoRoll(false);
		draw::eventList();
		}
	
	void setViewBar(int bar){
		viewBar = bar;
		scrn::viewBar.update(viewBar);
		//SCRNREPLACER
		draw::eventList();
		draw::drawPianoRoll(false);
		draw::viewBar();
	}
	
	//If there are substeps to advance, we shift the step selection forward by however many. e.g. if the selection is as below, and advanceSTeps is set to 16
	//B---B---B---B--- 
	//-B---B---B---B--
	//It wraps round at the end
	void maybeAdvanceStep(){
		if (settings::advanceSteps){
			const int numberOfStepsToAdvance = settings::advanceSteps;
			const int patternLength = Sequencing::getActivePattern().getLength().getLength();
			std::bitset<gc::numberOfSubsteps> overflow; //This allocates 512(4096/8) bytes. Maybe risky?
			//From the last steps that fall within the pattern length, set the overflow to wrap round to the start
			for(int i=0; i<numberOfStepsToAdvance; i++){
				bool s = stepSelection.test(patternLength-1-i);
				overflow.set(numberOfStepsToAdvance-1-i, s);
			}
			// lg("e");
			//Actually shift the steps(within the paternLength) - from the end
			for(int i=0; i<patternLength-numberOfStepsToAdvance; i++){
				const int oldPos = patternLength-1-i-numberOfStepsToAdvance;
				const int newPos = patternLength-1-i;
				// lg(oldPos);
				// lg(newPos);
				const bool s = stepSelection.test(oldPos);
				stepSelection.set(newPos, s);
			}
			// lg("f");
			//Set the start from the overflow:
			for(int i=0; i<numberOfStepsToAdvance; i++){
				stepSelection.set(i, overflow.test(i));
			}
			// lg("g");
		}
	}
		
	int getSubstepThatButtonRepresents(const int button){
		int step	= (inSubstep() ? editStepForSubstep : button) * 16;
		int substep	= (inSubstep() ? button : 0);
		int total	= viewBar * 256 + substep + step;
		return total;
	}
		
	void setStep(uint8_t step, bool add){
		const int substep = getSubstepThatButtonRepresents(step);
		if (!add){
			stepSelection.reset();
			stepSelection.set(substep);
			}
		else{//Shift is held
			stepSelection.flip(substep);
			if(stepSelection.count() == 0){
				stepSelection.set();
				}
			}
		}
		
	void setSubstepStep(int step){
		editStepForSubstep = step;
		draw::drawPianoRoll(false);
		scrn::substep.update(editStepForSubstep);
	}
	
	void exitSubstepStep(){
		editStepForSubstep = -1;
		draw::drawPianoRoll(false);
		scrn::substep.update("-");
	}
	
	void setEditStep(int button){
		setStep(button, interface::isShiftHeld);
		draw::eventList();
		draw::drawPianoRoll(false);
	}
		
	void setSubstepOrEnterBar(int button){//Add notes
		if(isShiftHeld){
			exitSubstepStep();
			setViewBar(button);
			//modes::switchToMode(modes::substep, true);
		} else {
			setSubstepStep(button);
		}
	}
	
	void addNoteOnStep(int button){
		uint8_t s = static_cast<uint8_t>(button);
		// static long long timeAtLastPress = 0;
		// static uint8_t lastStep = 16;
		editNotes::setEditStep(button);
		if(settings::editPriority == editPriorityModes::noteFirst){ 
			forEachPitch([s](uint8_t i){
				patMem::position pos = inSubstep() ? patMem::position{viewBar, (uint8_t)editStepForSubstep, s} : patMem::position{viewBar, s};
				bool isMono = Sequencing::getActiveTrack().getTrackMode() == Sequencing::trackType::mono;
				Sequencing::getActiveTrack().addOrRemoveNote({i, editVelocity, editLength, editAccent, editLegato}, pos , isMono);
				});
			}

		draw::drawPianoRoll(false);

		}

	void triggerNote(int note){
		//silenceNotesWhenRecording
		if(settings::notePreview){
			uint8_t previewPitch = gPFK(note);
			Sequencing::getActiveTrack().playNoteOn({previewPitch, editVelocity, {false}});//Length does not matter
			if(settings::silenceNotesWhenRecording){
				Sequencing::getActiveTrack().setTemporaryMute();
			}
		}
	}
	
	void releaseNote(int note){
		if(settings::notePreview){
			uint8_t previewPitch = gPFK(note);
			Sequencing::getActiveTrack().playNoteOff(previewPitch);
			if(settings::silenceNotesWhenRecording){
				Sequencing::getActiveTrack().setTemporaryMute();
			}
		}
	}
	
	void addNoteOnPitch(int note){
		editNotes::setEditPitch(note);
		if(settings::editPriority == editPriorityModes::stepFirst){
			forEachStep([note](patMem::position pos){
				uint8_t noteToAdd = gPFK(note);
				if (noteToAdd <= 127){
					bool isMono = Sequencing::getActiveTrack().getTrackMode() == Sequencing::trackType::mono;
					Sequencing::getActiveTrack().addOrRemoveNote({noteToAdd, editVelocity, editLength, editAccent, editLegato}, pos, isMono);
					// lg(i);
				}
			});
			maybeAdvanceStep();
		}
		triggerNote(note);
		draw::drawPianoRoll(false);
	}
 
	void setEditTrack(int button){
		editTrack = button;
		scrn::track.update(interface::editTrack);
		// draw::patternNameAndNumber();
		// draw::drawPianoRoll(false);
		// draw::eventList();
		// draw::mute();
		draw::updateScreen();
		if(modes::checkActive(modes::editPattern)){
			list::editPatt.draw();
		}
	}
	
	void clearSteps(){
		//int numStepsCleared = 0;
		forEachStepAndPitch([=](uint8_t pitch, patMem::position pos){//Remove the p var
			//numStepsCleared++;
			//patMem::position pos = inSubstep() ? patMem::position{viewBar, editStepForSubstep, step} : patMem::position{viewBar, step};
			Sequencing::getActiveTrack().removeNote({pitch}, pos); //Just remove
		});
		// lg(numStepsCleared);
		draw::updateScreen();
	}
	
}// end editNotes namespace
	
namespace editPatt{
	//This sets the pattern to be 1 - 256 steps long
	void setStepLength(const int length){
		const int actualLength = (length == 256) ? 0 : length; //Use 0 steps to represent 256
		Sequencing::getActivePattern().setLength(actualLength);
		draw::updateScreen();
	}
	
	//This sets how many steps per bar, which is just for the display
	void setStepsPerBar(const int button){
		lg(button+1);
		Sequencing::getActivePattern().setStepsPerBar(button+1);
		draw::updateScreen();
	}
	
	//This sets how many bars the pattern should be and sets the pattern length to stepsPerBar * number of BArs
	void setNumberOfBars(const int bars){
		lg(bars+1);
		Sequencing::getActivePattern().setNumberOfBars(bars+1);
		draw::updateScreen();
	}
	
	void setVelocity(const int val){
		Sequencing::getPattern(editTrack).setVelocity(val);
		draw::updateScreen();
	}
	
	void setAccentVelocity(const int val){
		Sequencing::getPattern(editTrack).setAccentVelocity(val);
		draw::updateScreen();
	}
	
	void setShuffle(const int val){
		Sequencing::getPattern(editTrack).setShuffle(val);
	}
 
	void setFillToBar(){
		Sequencing::getPattern(editTrack).toggleStretchToBar();
	}
		
	void toggleFillToBar(){
		Sequencing::getPattern(editTrack).toggleStretchToBar();
	}

	void setTrackMode(const int val){
		Sequencing::getActiveTrack().setTrackMode(val);
	}

	const char* getFillToBar(){
		bool isStretchToBar = Sequencing::getPattern(editTrack).isStretchToBar();
		return isStretchToBar ? "yes" : "no";
	}
 
	void setPatternTranpose(const int note){
		int transpose = gPFK(note) - 48 ;
		Sequencing::getPattern(editTrack).setTranspose(transpose);
	}
	
	// void resetTransposeMode(){
		// modes::reset()
	// }
	// void setPatternTranpose(const int value){
		// Sequencing::getPattern(editTrack).setTranspose(value);
		// }
	
	void setTrackChannel(const int button){
		Sequencing::getActiveTrack().setMIDIChannel(button);
	}
	
	void setTrackPort(const int button){
		Sequencing::getActiveTrack().setMIDIPort(button);
	}
	
	void setSoundEngine(const int button){
		if(audio::isValidIndex(button)){
			Sequencing::getActiveTrack().setSoundEngine(button);
		}
	}
	
	// int getTrackMidi(const int chan){
		// return Sequencing::getActiveTrack().getMIDIchannel();
	// }
}
namespace patternFunctions{
	void clearNotesInPattern(){
		Sequencing::getPattern(editTrack).clearNotes();
	}
}//End pattern Functions namespace

namespace shiftNotes{//Just for changing notes in editSteps screen
	int snapBackToEditStepMode = false;
	void goToShiftNotesMode(){
		modes::switchToMode(modes::shiftNotes, true);
		snapBackToEditStepMode = true;
		editStep::setListPositionToLast(false);
	}
	
	void returnToStepMode(){
		if(snapBackToEditStepMode){
			modes::switchToMode(modes::editSteps, true);
		}
		snapBackToEditStepMode = false;
	}
	
	void jumpBackToStepMode(const int u){
		returnToStepMode();
	}
	
	
	void editPitch(int button){
		int newPitch = gPFK(button);
		forEachStepAndPitch([=](uint8_t pitch, patMem::position pos){
			patMem::note noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos, {pitch}).getNote();
			if(noteToEdit.isValid() && noteToEdit.getPitch() != newPitch){
				//noteToEdit.print();
				Sequencing::getActivePattern().addOrRemoveNote(noteToEdit, pos); 
				noteToEdit.setPitch(newPitch);
				//noteToEdit.print();
				Sequencing::getActivePattern().addOrRemoveNote(noteToEdit, pos);
			}
		});
		draw::eventList();
	}
	

}//End shiftNotes namesapce
namespace editStep{ //Shit namespace
	
	void setViewBarOrEditSubstep(int button){
		if(isShiftHeld){
			editNotes::setSubstepStep(button);
			//modes::switchToMode(modes::substepEdit, true); //RMV because substepEdit is now same mode
		} 
		else {
			editNotes::setViewBar(button);
		}
	}
	
	//Display funcs:
	void showEditLength(){
		char lengthString[10] = {0};
		editLength.makeString(lengthString);
		scrn::length.update(lengthString);
		}
		
	void setEditVelocity(byte vel){
		editVelocity = vel;
		scrn::velocity.update(editVelocity);
		}
		
	void toggleEditAccent(){
		editAccent = !editAccent;
		scrn::accent.update(utl::getYesOrNo(editAccent));
		}
		
	void toggleEditLegato(){
		editLegato = !editLegato;
		scrn::legato.update(utl::getYesOrNo(editLegato));
		}
		
setVarAfterDelay::setHex editStepLength {[](uint16_t x){
	forEachStepAndPitch([=](uint8_t p, patMem::position pos){
		auto noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
		noteToEdit.setLength({x});
	});
	editLength = {x};
	showEditLength();
	draw::eventList();
	draw::drawPianoRoll(false);
},3};
	
setVarAfterDelay::setDecimal editStepVelocity {[](uint8_t x){
	forEachStepAndPitch([=](uint8_t p, patMem::position pos){
		auto noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
		noteToEdit.setVelocity(x);
	});
	setEditVelocity(x);
	draw::eventList();
	draw::drawPianoRoll(false);
},0,127};

	enum class editStepMode{
		lengthBars = 0	,
		lengthSteps	,
		lengthSubsteps	,
		length	,
		positionBar	,
		positionStep	,
		positionSubstep	,
		position	,
		pitch	,
		velocity	,
		accent	,
		legato	,
		max	,
	};
editStepMode activeEditStepMode;

	const char* const paramNames[] = {
		"Length(bars)"	,
		"Length(steps)" 	,
		"Length(substeps)"	,
		"Length"	,
		"Position(bar)"	,
		"Position(step)"	,
		"Position(substep)"	,
		"Position"	,
		"Pitch"	,
		"Velocity"	,
		"Accent"	,
		"Legato"	,
	};
	
	
	//Set the parameters of selected notes
	void editStepParameter(const int button){
		uint8_t b = static_cast<uint8_t>(button);
		switch (activeEditStepMode){
			case editStepMode::lengthBars      :
			case editStepMode::lengthSteps	:
			case editStepMode::lengthSubsteps	:
			case editStepMode::positionBar	:
			case editStepMode::positionStep	:
			case editStepMode::positionSubstep	:
			
					forEachStepAndPitch([=](uint8_t p, patMem::position pos){
						auto noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
						switch(activeEditStepMode){
							case editStepMode::lengthBars:	noteToEdit.setLengthBars	({b,0,0})	; break;
							case editStepMode::lengthSteps:	noteToEdit.setLengthSteps	({0,b,0})	; break;
							case editStepMode::lengthSubsteps:	noteToEdit.setLengthSubsteps	({0,0,b})	; break;
							case editStepMode::positionBar:	/**/	/**/	; break;
							case editStepMode::positionStep:	/**/	/**/	; break;
							case editStepMode::positionSubstep:	/**/	/**/	; break;
							default: break;
						}
					});
				
				switch(activeEditStepMode){
					case editStepMode::lengthBars:	editLength.setBar(button);	showEditLength();	break;
					case editStepMode::lengthSteps:	editLength.setStep(button);	showEditLength();	break;
					case editStepMode::lengthSubsteps:	editLength.setSubstep(button);	showEditLength();	break;
					case editStepMode::positionBar:	notifications::notImplemented.display();	/**/	break;
					case editStepMode::positionStep:	notifications::notImplemented.display();	/**/	break;
					case editStepMode::positionSubstep:	notifications::notImplemented.display();	/**/	break;
					default: break;
				}
				draw::eventList();
				draw::drawPianoRoll(false);
				break;
			case editStepMode::length:
				editStepLength.buttonPress(button);
				break;
			case editStepMode::position:
				notifications::notImplemented.display();
				break;
			case editStepMode::pitch:
				//shiftNotes::goToShiftNotesMode();
				lg("NOT HERE");
				break;
			case editStepMode::velocity:
				editStepVelocity.buttonPress(button);
				break;
			default:
				break;
		}
	}
	
	int lastTypeSet = 0;
	void setListPositionToLast(const bool redraw){
		list::editSteps.setPosition(lastTypeSet, redraw);
	}
	
	void setStepParameterType(const int toSet){
		if(toSet == static_cast<uint8_t>(editStepMode::accent)){
			forEachStepAndPitch([=](uint8_t p, patMem::position pos){
				auto noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
				noteToEdit.toggleAccent();
			});
			toggleEditAccent();
			interface::editStep::setListPositionToLast();
			draw::eventList();
			draw::drawPianoRoll(false);
		}
		else if (toSet == static_cast<uint8_t>(editStepMode::legato)){
			forEachStepAndPitch([=](uint8_t p, patMem::position pos){
				auto noteToEdit = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
				noteToEdit.toggleLegato();
			});
			toggleEditLegato();
			interface::editStep::setListPositionToLast();
			draw::eventList();
			draw::drawPianoRoll(false);
		}
		else if(toSet < static_cast<uint8_t>(editStepMode::max)){
			activeEditStepMode = static_cast<editStepMode>(toSet);
			scrn::editSteps.update(interface::editStep::paramNames[static_cast<int>(activeEditStepMode)]);
			lastTypeSet = toSet;
		} else {
			notifications::dataOutOfRange.display();
		}
	}

}//End editstep namesapce
namespace mute{
	//Holding shift toggles every other note
	void toggleMuteTrack(int button){
		if(interface::isShiftHeld){
			for(int i=0; i<gc::numberOfTracks; i++){
				if(i != button){
					Sequencing::getTrack(i).toggleMute();
				}
			}
		} else { //Shift not held:
			Sequencing::getTrack(button).toggleMute();
		}
		draw::muteTracks();
	}
	
	void muteTrack(int button){
		Sequencing::getTrack(button).mute();
		Sequencing::getTrack(button).endLegatoNote();
		draw::muteTracks();
	}
	
	void unmuteTrack(int button){
		Sequencing::getTrack(button).unmute();
		draw::muteTracks();
	}
		 
	void toggleMuteNote(int note){
		int noteToMute = gPFK(note);
		
		if(noteToMute < 128){
			if(interface::isShiftHeld){
				for(int i=0; i < 128; i++){
					if(i != noteToMute){
						Sequencing::getActiveTrack().toggleMutedNote(i);
					}
				}
			} else { //Shift not held:
				Sequencing::getActiveTrack().toggleMutedNote(noteToMute);
			}
			draw::muteNotes();
		}
	}
}
namespace settings{
	const char* getPriorityModeString(){
		return (editPriority == editPriorityModes::stepFirst) ? "step" : "note";
		}
		
	void resetAfterEditToggle(){
		// if(settings::editPriority == editPriorityModes::stepFirst) {
			// editPitches.reset();
		// } else{
			// editSteps.reset();
		// }
	}
	void setScreenBrightness(int value){
		screenBrightness = value;
		scrn::brightness(screenBrightness);
	}
	int getScreenBrightness(){
		return	screenBrightness; 
	}
	int getAdvanceSteps(){
		return	advanceSteps; 
	}
	void setAdvanceSteps(const int value){
		advanceSteps = value % 4096;
	}
}//End Settings namespace
namespace performance{
	void syncOnBarChange(){//Replace this to take a pattern and it resets when that pattern loops round
		Sequencing::setSyncOnBarChange();
	}
		
	void tempoChangeLarge(int b){
		int multiplier = (b * 2) - 1;
		double tempo = Sequencing::getTempo();
		interface::all::setTempo(tempo + multiplier);
	}
	
	void tempoChangeSmall(int b){
		int multiplier = (b * 2) - 1;
		double tempo = Sequencing::getTempo();
		interface::all::setTempo(tempo + multiplier * 0.01);
	}
	
	void tempoChangeLargeEnc(int b){
		double tempo = Sequencing::getTempo();
		interface::all::setTempo(tempo + (float)b);
	}
	
	void tempoChangeSmallEnc(int b){
		double tempo = Sequencing::getTempo();
		interface::all::setTempo(tempo + (float)b * 0.01);
	}
		
	//Fills etc.
}//End performance namespace
namespace pattUtils{
	char currentLoadPatternName[card::maxDirectoryLength+1] = {0};
	
	void renamePattern(){
		std::function<void(const char*)> setter = [](const char* newName) {Sequencing::getActivePattern().setName(newName);};
		all::setKeyboardContext(setter, []{return Sequencing::getActivePattern().getName();}, "Pattern name", 16);
	}
		
	void renameTrack(){
		std::function<void(const char*)> setter = [](const char* newName) {Sequencing::getActiveTrack().setName(newName);};
		all::setKeyboardContext(setter, []{return Sequencing::getActiveTrack().getName();}, "Track name", Sequencing::nameLength);
	}
		
	//Removed due to rewriting the on-screen keyboard 
	// void addCharToPatternName(const char character){
		// Sequencing::getActivePattern().addCharToName(character);
		// }
		
	// void removeCharFromPatternName(){
		// Sequencing::getActivePattern().removeCharFromName();	
		// }
		
	// void savePatternAsToCard(bool checkFileExists){
		// char name[9];
		// Sequencing::getActivePattern().getName(name);
		// if(name[0] == 0){ //If string has no name
			// notifications::noFileName.display();
			// modes::switchToMode(modes::rename, true);
			// return;
			// }
		// if(card::fileExists(name) && checkFileExists){
			// modal::initiateModal(
				// "Overwrite file?", 
				// {
					// {"No",  nullFunc},
					// {"Yes", []{savePatternAsToCard(false);}}
				// }
			// );
			// return; //Do not save if file exists
		// }
		// Sequencing::getActivePattern().saveToCard();
	// }
		
	// void savePatternAsToCard(){
		// savePatternAsToCard(true);
	// }
	
	void enterSaveMode(){
		settings::isInSaveMode = true;
		modes::switchToMode(modes::cardView, true);
	}
	
	void enterLoadMode(){
		settings::isInSaveMode = false;
		modes::switchToMode(modes::cardView, true);
	}
	
	void goToParentFolder(){
		card::goToParentFolder();
		list::cardFiles.draw();
	}
	
	void lookInside(){
		card::lookInside();
		list::cardFiles.draw();	
	}
	
	void newFolder(){
		//void setKeyboardContext(std::function<void(const char*)> function, const char* name, const int maxLength){
		all::setKeyboardContext(card::newFolder, []{return "";}, "Folder Name", 64);
	}
	
	void saveAll(){
		all::setKeyboardContext(card::saveAll, []{return "";}, "File Name", 64);
		list::cardFiles.draw();	
	}
	
	void loadAllFromActive(){
		card::loadAllFromActive();
		list::cardFiles.draw();	
	}
	
	void renameActive(){
		all::setKeyboardContext(card::renameActive, []{return card::getActiveName();}, "Rename Entry", 64);
	}	
	
	void deleteActive(){
		modal::initiateModal(
			"Delete file?", 
			{
				{"No",  nullFunc},
				{"Yes", []{card::deleteActive();list::cardFiles.draw();	}}
			}
		);
	}
		
	
	void copy(){
		card::copy();
		list::cardFiles.draw();	
	}	
	
	void cut(){
		card::cut();
		list::cardFiles.draw();	
	}	

	void paste(){
		card::paste();
		list::cardFiles.draw();	
	}		
	
	// void toggleSaveMode(){
		// settings::isInSaveMode = !settings::isInSaveMode;
	// }
	
	// void toggleDefaultSaveLocation(){
		// settings::saveToLastLocation = !settings::saveToLastLocation;
	// }
	
	// void toggleOverwrite(){
		// settings::overwriteExisting = !settings::overwriteExisting;
	// }
		
	void doCardFunction(const int button){
		list::cardView.doFunc(button);
	}
		
	int32_t loadPatternToMemory(){
		int32_t newPattern = Sequencing::loadPatternToMemory(currentLoadPatternName);
		return newPattern;
		}
		
	void loadPatternToActiveTrack(){
		const int32_t newPattern = loadPatternToMemory();
		if(newPattern != -1){
			Sequencing::getActiveTrack().switchPattern(static_cast<uint32_t>(newPattern));
			draw::patternNameAndNumber();
			}
		}
		
	void loadPatternFromCardToTrackBank(){
		// const int16_t newPattern = loadPatternToMemory();
		// if(newPattern != -1){
			// Sequencing::getActiveTrack().addPatternToBank(newPattern);
			// }
		}
		
	void renamePatternOnCard(){
		
		}	
	void connectToCard(){
		card::begin();
		}
	}//End Pattutils namespace
namespace pattSwitch{
	
	// void selectPatternUsingButton(const int button){
		// static bool firstPress = true;
		// static int lastKey;
		// static scheduled::eventID switchEvent;
		// if(firstPress){
			// switchEvent = scheduled::newEvent([button](){
				// if(Sequencing::getActiveTrack().doesTrackBankHavePattern(button)){
					// patMem::pattern_t patt = Sequencing::getActiveTrack().getTrackBankPattern(button);
					// Sequencing::getActiveTrack().switchPattern(patt);
					// interface::pattSwitch::selectedPattern = patt.getAddress();
					// draw::updateScreen();
				// } else {
					// notifications::patternDoesNotExist.display();
				// }
				// firstPress = true;
			// }, 750);
			// firstPress = false;
			// lastKey = button;
		// } 
		// else{
			// scheduled::clearEvent(switchEvent);
			// const int newPattern = (lastKey * 16) + button;
			// Sequencing::getActiveTrack().switchPattern(newPattern);
			// interface::pattSwitch::selectedPattern = newPattern;
			// firstPress = true;
			// list::memPatts.draw();
			// draw::patternNameAndNumber();
		// }
	// }
	
	void switchToPattern(const int button){
		static bool firstPress = true;
		static int lastKey;
		if(firstPress){
			scheduled::newEvent(scheduled::lOE::patternSwitch, [button](){
				if(Sequencing::getActiveTrack().doesTrackBankHavePattern(button)){
					patMem::pattern_t patt = Sequencing::getActiveTrack().getTrackBankPattern(button);
					Sequencing::getActiveTrack().switchPattern(patt);
					interface::pattSwitch::selectedPattern = patt.getAddress();
					draw::updateScreen();
				} else {
					notifications::patternDoesNotExist.display();
				}
				firstPress = true;
			}, 750);
			firstPress = false;
			lastKey = button;
		} 
		else{
			scheduled::clearEvent(scheduled::lOE::patternSwitch);
			const uint32_t newPattern = (lastKey * 16) + button;
			Sequencing::getActiveTrack().switchPattern(newPattern);
			interface::pattSwitch::selectedPattern = newPattern;
			firstPress = true;
			list::memPatts.draw();
			draw::patternNameAndNumber();
		}
	}
		
	void addPatternToActiveTrack(){
		Sequencing::getActiveTrack().switchPattern(selectedPattern);
		draw::patternNameAndNumber();
	}
	
	void removePatternFromTrackBank(const int bankPos){
		Sequencing::getActiveTrack().removePatternFromBank(bankPos);
	}
	
	void removePatternFromTrackBankInitiate(){
		modal::initiateNumberModal(
 			"Remove Which Pattern From Bank?",
			removePatternFromTrackBank
			);
		}
		
	void clearPatternWhereItIsUsed(const uint32_t patNum){//Doesn't deal with arrangement
		//TODO: sort out arrangement as the code Doesn't deal with arrangement");
		for(int i = 0; i<16; i++){
			Sequencing::getTrack(i).removePatternFromBankIfExists(patNum);
			Sequencing::getTrack(i).removePatternFromTrackIfActive(patNum);
		}
		for(int i = 0; i< arrangement::numberOfArrangements; i++){
			for(int j = 0; j< gc::numberOfTracks; j++){
				arrangement::getArrangeChannel(i,j).deletePatternIfFound({patNum});
			}
		}
	}
	
	void deletePatternFromMemory(){
		patMem::pattern_t(selectedPattern).destroy();
		clearPatternWhereItIsUsed(selectedPattern);
		list::memPatts.draw();
		draw::patternNameAndNumber();
	}
	
	void addPatternToTrackBank(){
		Sequencing::getActiveTrack().addPatternToBank({selectedPattern});
	}
	
	void newBlankPattern(){
		patMem::pattern_t newPattern = Sequencing::getActiveTrack().newPattern();
		if(newPattern.isValid()){
			if(Sequencing::getActiveTrack().areTrackBankSlotsAvailable()){
				Sequencing::getActiveTrack().addPatternToBank(static_cast<uint32_t>(newPattern.getAddress()));
				}
			list::memPatts.draw();
			draw::patternNameAndNumber();
		}	
	}
	
	void newDuplicatePattern(){
		patMem::pattern_t oldPattern = Sequencing::getActiveTrack().getActivePattern();
		patMem::pattern_t newPattern = {true};
		if (newPattern.isValid()){
			newPattern.copyFromPattern(oldPattern);
			Sequencing::getActiveTrack().switchPattern(newPattern);
			list::memPatts.draw();
			draw::patternNameAndNumber();
			}
		if(Sequencing::getActiveTrack().areTrackBankSlotsAvailable()){
			Sequencing::getActiveTrack().addPatternToBank(newPattern);
			}
		}
		
	void clearPatternFromTrack(){
		Sequencing::getActiveTrack().removePatternFromTrack();
		draw::patternNameAndNumber();
		}
		
	void doPatternListFunction(const int button){
		list::pattSwit.doFunc(button);
		}
	
	void switchToActive(){
		if(Sequencing::getActiveTrack().getActivePattern().isValid()){
			interface::pattSwitch::selectedPattern  = Sequencing::getActiveTrack().getActivePattern().getAddress();
		}
	}
		 
	}//End pattswitch namespace
namespace record{
	
	void clearNoteBuffer(){
		for(int i = 0; i< Sequencing::maxRecordingNotes; i++){
			Sequencing::getRecordingNote(i).n.clear();
		}
	}
	
	void startNote(const int key){
		if(Sequencing::isSeqPlaying()){
			//Add pattern if not one there already:
			if(!Sequencing::getActivePattern().isValid()){//Create new track or clear buffer
				patMem::pattern_t newPatt = Sequencing::getActiveTrack().newPattern();
				if(!newPatt.isValid()){
					clearNoteBuffer();
					interface::editNotes::triggerNote(key);
					return;
				}
				Sequencing::getActiveTrack().addPatternToBank(newPatt);
			}
			//Which step to go on:
			int pitch = gPFK(key);
			//Look for note in array:
			bool addedNote = false;
			for(int i = 0; i<Sequencing::maxRecordingNotes; i++){//See if the note is there already:
				if(Sequencing::getRecordingNote(i).n.isValid() && Sequencing::getRecordingNote(i).n.getPitch() == pitch){
					Sequencing::getRecordingNote(i).p = Sequencing::getActiveTrack().getPlayPosition();
					Sequencing::getRecordingNote(i).n.setLength({0, 0, isShiftHeld});//If shift is held, length is set to one and will not be added
					// lg(Sequencing::getRecordingNote(i).n.getLength().getValue());
					addedNote = true;
					break;
				}
			}
			if(!addedNote){//If the note is not there already:
				for(int i = 0; i<Sequencing::maxRecordingNotes; i++){
					if(!Sequencing::getRecordingNote(i).isValid()){
						Sequencing::getRecordingNote(i).n.setNote(pitch, editVelocity, {0, 0, isShiftHeld}); //Length represents is shift held. Real length is not known at this stage
						Sequencing::getRecordingNote(i).p = Sequencing::getActiveTrack().getPlayPosition();
						addedNote = true;
						break;
					}
				}
			}
			if(!addedNote){
				lg("error"); //Record buffer full
			}
		}
		//Perhaps play the note, if shift is not held:
		if(!isShiftHeld){
			interface::editNotes::triggerNote(key);
		}
	}
	
	void endNote(const int key){
		if(Sequencing::isSeqPlaying()){
			uint8_t pitch = gPFK(key);
			for(int i = 0; i<Sequencing::maxRecordingNotes; i++){
				if(Sequencing::getRecordingNote(i).n.getPitch() == pitch){
					if(!isShiftHeld && Sequencing::getRecordingNote(i).n.getLength().getValue() != 1){
						// lg("test: ");
						// lg(Sequencing::getRecordingNote(i).n.getLength().getValue());
						Sequencing::notePos np = Sequencing::getRecordingNote(i);
						//check note length and do not add if length equals one
						bool isMono = Sequencing::getActiveTrack().getTrackMode() == Sequencing::trackType::mono;
						patMem::position length = patMem::subtract(Sequencing::getActiveTrack().getPlayPosition(), np.p, Sequencing::getActiveTrack().getLength());
						Sequencing::getActiveTrack().addOrUpdateNote({pitch, np.n.getVelocity(), length, editAccent, editLegato}, np.p, isMono);
					}
					Sequencing::getRecordingNote(i).n.clear();
					scheduled::newEvent(scheduled::lOE::drawPianoRoll, []{
						draw::drawPianoRoll(false);
					},0);
					interface::editNotes::releaseNote(key);
					return;
				}
			}
			notifications::recordNoteOffWithoutOn.display();
		}
		interface::editNotes::releaseNote(key);
	}
	
	void toggleRecMode(){
		recording = !recording;
		LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
		draw::recordIcon();
		//If we are now recording, and not only from external:
		modes::focusedContext newContext = recording && !settings::recordOnlyFromExternal ? modes::focusedContext::keyboardPiano : modes::focusedContext::none;
		modes::setDialog(newContext);
	}
	
}//End record namespace
namespace modals{
	void doModalFunction(const int num){
		modal::doFunction(num);
		modes::clearDialog();
		}
	void doModalNumFunction(const int num){
		modal::doNumFunction(num);
		modes::clearDialog();
		}
}//End modals namespace

namespace all{
	void setTempo(const double tempo){
		Sequencing::setTempo(tempo);
		draw::tempo();
	}
	
	void changeVolume(const int change){
		volume += (static_cast<float>(change)*0.01);
		lg(volume);
	}
	
	void keyboardOctShift(const int udKeyNum){//0 is down. 1 is up
		int dir = (udKeyNum * 2) - 1;
		keyboardOctave = constrain(keyboardOctave+dir, 0, 9);
		scrn::octave.update(keyboardOctave);
		draw::drawPianoRoll(false);
		draw::muteNotes();
	}
	void listMove(const int udKeyNum){
		int dir = udKeyNum * 2 - 1;
		list::moveList(dir);
	}
		
	void jumpToListItem(const int button){
		list::jumpTo(button);		
		}
		
	void performActionWithListItem(const int button){
		list::performActionOnActiveListItem(button);
	}
 
	void setShiftOn(){
		isShiftHeld = true;
		//bitSetTest();
	}
	bool helpHasTriggered = false;
	
	void setShiftOff(){
		isShiftHeld = false;
		if(helpHasTriggered){
			scrn::writeFillRect(0, scrn::topOffset, scrn::width, scrn::height, scrn::getThemeColour(scrn::td::bg));
			draw::updateScreen();
			helpHasTriggered = false;
		}
	}
	
	void displayHelp(){
		isShiftHeld = true;
		if(settings::showHelp){
			functionDescriptions::displayDescriptions();
			draw::modeGrid();
			helpHasTriggered = true;
		}
	}

	void startButton(){
		// lgc("startButton:");
		// lg(Sequencing::seqSta[static_cast<int>(Sequencing::seqStatus)]);
		using namespace Sequencing;
		switch(Sequencing::getSequencerStatus()){
			case sequencerStatus::paused:
				Sequencing::continueSequencer();
				return;
			case sequencerStatus::stopped:
				Sequencing::startSequencer();
				return;
			case sequencerStatus::playing:
				Sequencing::pauseSequencer();
				return;
		}
	}
	
	void stopButton(){
		// lgc("stopButton:");
		// lg(Sequencing::seqSta[static_cast<int>(Sequencing::seqStatus)]);
		using namespace Sequencing;
		switch(Sequencing::getSequencerStatus()){
			case sequencerStatus::paused:
			case sequencerStatus::playing:
				Sequencing::stopSequencer();
				return;
			case sequencerStatus::stopped:
				Sequencing::pauseSequencer();
				//Sequencing::resetSequencer();
				return;
		}
	}
	
	// void exitError(){
		// notifications::notification::returnToMode();
		// }
	void exitError(const int button){
		//lg("exitError");
		modes::clearDialog();
	}
		
	void sendAllNotesOff(){
		MIDIports::sendAllNotesOff();
	}
	
	const int keyboardBufferLength = 64;
	size_t activeSetterLength = 999;
	char textBuffer[keyboardBufferLength] = {0};
	std::function<void(const char*)> keyboardSetterFunction = nullptr;
	
	void drawTextBuffer(){
		const bool screenLocked = scrn::isScreenLocked();
		scrn::setScreenLock(false);
		scrn::writeFillRect(30, 50, 300, 20, scrn::getThemeColour(scrn::td::bg));
		scrn::write(30, 50, textBuffer);
		lg(textBuffer);
		scrn::setScreenLock(screenLocked);
	}
	
	void setKeyboardContext(std::function<void(const char*)> function, std::function<const char*()> currentName, const char* fieldName, const int maxLength){
		keyboardSetterFunction = function;
		activeSetterLength = maxLength;
		strcpy(textBuffer, currentName());
		scrn::writeFillRect(25, 25, 430, 50, scrn::getThemeColour(scrn::td::bg));
		scrn::writeEdgeRect(25, 25, 430, 50, scrn::getThemeColour(scrn::td::acc1));
		scrn::write(30, 30, fieldName);
		drawTextBuffer();
		draw::keyboardButtons();
		modes::setDialog(modes::focusedContext::keyboardText);
		scrn::setScreenLock(true);
	}
		
	void keyboardTypePress(const int shiftedNum){
		//If a character, add it
		if(shiftedNum < gc::numberOfCharacters-2){
			if(strlen(textBuffer) < activeSetterLength - 1){
				strcatc(textBuffer, gc::chars[shiftedNum]);
				drawTextBuffer();
			} else {
				notifications::nameFull.display();
			}
		}
		//If a backspace, backspace
		else if (shiftedNum < gc::numberOfCharacters-1){
			const int textLength = strlen(textBuffer);
			if(textLength > 0){
				textBuffer[textLength-1] = 0;
				drawTextBuffer();
			} else {
				notifications::nameEmpty.display();
			}
		}
		//If "save", save it
		else if (shiftedNum < gc::numberOfCharacters){
			if(keyboardSetterFunction){
				keyboardSetterFunction(textBuffer);
				if(settings::clearNameBufferOnEnter){
					textBuffer[0] = 0;
				}
			} else {
				lg("error");
			}
			modes::clearDialog();
			scrn::setScreenLock(false);
		}
	}
		
		
}//End all namespace

namespace colour{
	int editColourDescriptor = 0;
	int editColourChannel = 0;
	uint8_t value = 0;
	
	void updateValueFromTheme(){
		value = scrn::getColourChannelValue(editColourDescriptor, editColourChannel);
	}
	
	void setEditColourDescriptor(const int i){
		editColourDescriptor = i;
		updateValueFromTheme();
		draw::editColoursVars();
	}
	
	void setEditColourChannel(const int i){
		if(i < 3){
			editColourChannel = i;
			updateValueFromTheme();
			draw::editColoursVars();
		}
	}
	
	void editColour(const int val){
		scrn::editColour(editColourDescriptor, editColourChannel, val);
		value = val;
		// draw::editColoursVars();
		// draw::editColours();
		draw::updateScreen();
	}
	
	setVarAfterDelay::setDecimal setter{editColour, 0, 255};
	
	void editColourValue(const int button){
		value = setter.buttonPress(button);
		draw::editColoursVars();
	}
}//end colour namespace
namespace arrange{
	const int numberOfBars = 32;
	int editTimelinePosition = 0;
	int firstTimelinePosition = 0; //Where the grid starts on the screen
	int playingTimelinePosition = 0;
	long long timelineTick = 0; //In micros//
	uint8_t activeArrangement = 0;
	patMem::pattern_t editPattern = {};
	patMem::pattern_t highlightedPattern = {};
	int appendNewPatternsForXBars = 0;
	int arrangementEnd = 0; //For adding new patterns to end of arrangement
	int addLength = 8;
	bool useArrangeMode = false; //Whether each tracks takes patterns from the arrangeMode
	struct cursorPos { enum cp{ 
		centre	,
		first	,
		free	,
		max	,
	};};
	cursorPos::cp cursorPosition = cursorPos::centre;
	const char* const cursorPositionStrings[] = {
		"centre"	,
		"first"	,
		"free"	,
	};
	
	void setEditPatternToEmpty(){
		editPattern = {};
		draw::arrangeDetails();
	}
	
	void setEditPatternToHighlighted(){
		editPattern = highlightedPattern;
		draw::arrangeDetails();
	}
	
	void setEditPatternToCurrentTrackPattern(){
		editPattern = Sequencing::getActivePattern();
		draw::arrangeDetails();
	}
	
	bool shouldAppendNewPatterns(){
		return appendNewPatternsForXBars;
	}
	
	void setAppendNewPatternsForBars(const int numberOfBars){//Zero for nothing
		appendNewPatternsForXBars = numberOfBars;
		list::drawList();
	}
	
	int getAppendNewPatternsForBars(){
		return appendNewPatternsForXBars;
	}
	
	void selectTrackFromBank(const int button){
		if(Sequencing::getActiveTrack().doesTrackBankHavePattern(button)){
			editPattern = Sequencing::getActiveTrack().getTrackBankPattern(button);
		}
	}
	
	void changeCursorPosition(const int button){
		if(button < cursorPos::max){
			cursorPosition = static_cast<cursorPos::cp>(button);
			list::drawList();
			return;
		}
		notifications::dataOutOfRange.display();
	}
	
	const char* getCursorPositionString(){
		return cursorPositionStrings[cursorPosition];
	}
	
	
	void moveTimelinePosition(const int key){
		int dir = key * 2 - 1;
		if(isShiftHeld){
			playingTimelinePosition = constrain(editTimelinePosition+dir, 0 , arrangement::maxPosition);
			draw::arrangeChaser();
		} else {
			editTimelinePosition = constrain(editTimelinePosition+dir, 0 , arrangement::maxPosition);
			highlightedPattern = arrangement::getArrangeChannel(activeArrangement,editTrack).getCurrentPattern(editTimelinePosition);
			
			switch(cursorPosition){
				case cursorPos::centre:
					firstTimelinePosition = max(0, editTimelinePosition - numberOfBars/2);
					break;
				case cursorPos::first:
					firstTimelinePosition = editTimelinePosition;
					break;
				case cursorPos::free:
				default:
					break;
			}
			// draw::arrangeTracks();
			// draw::arrangeDetails();
			draw::updateScreen();
		}
	}	
	
	void jumpTo(const int pos){
		if(pos < arrangement::maxPosition){
			editTimelinePosition = pos;
		}
		draw::updateScreen();
	}
	
	int getTimelinePosition(){
		return editTimelinePosition;
	}
	
	void nextTrack(const int button){
		editNotes::setEditTrack(constrain(editTrack + (button * -1) + 2, 0, gc::numberOfTracks-1));
	}
	
	void addPatternToTrack(){
		// lg("aPTT");
		// lg(blocks::patternBlockSize );
		// lg(blocks::contentsBlockSize);
		// lg(blocks::arrangeBlockSize );
		// lg("aPTT");
		// lg((long int)blocks::block_t(35).getPointer());
		// lg(*(blocks::block_t(35).getPointer()));
		// lg("aPTT");
		// lg((long int)arrangement::node(35, true).getPointer());
		// lg(*(arrangement::node(35, true).getPointer()));
		// lg("aPTT");
		for(int i=0; i<addLength; i++){
			arrangement::getArrangeChannel(activeArrangement,editTrack).addPattern(editTimelinePosition + i, editPattern);
		}
		//int setIntToEmptyPosition = hasAddedEmptyPattern ? 1 : 0; //Subtract one from if the last pattern added was empty
		arrangementEnd = max(editTimelinePosition + 1 + addLength, arrangementEnd);
		// draw::arrangeDetails();
		// draw::arrangeTracks();
		draw::updateScreen();
	}
	
	void changeArrangement(const int button){
		static uint8_t first = 0;
		static uint8_t last = 0;
		if(button >= 8)	{first = button;}
		else	{last = button-8;}
		activeArrangement = first*8 + last;
		//arrangementEnd - work out this somehow just look for 4095 and thn back/?
	}
	
	void setAddLength(const int num){
		addLength = num;
		list::drawList();
	}
	
	int getAddLength(){
		return addLength;
	}
	
	void setEditTrackOrChangeArrangement(const int button){
		if(isShiftHeld){
			changeArrangement(button);
		} else {
			editNotes::setEditTrack(button);
		}
	}
	
	void setLoopStart(){
		Sequencing::setLoopStart(editTimelinePosition);
		draw::updateScreen();
	}
	
	void setLoopEnd(){
		Sequencing::setLoopEnd(editTimelinePosition+1);
		draw::updateScreen();
	}
	
	/* In new or duplicate patterns:
	if(shouldAppendNewPatterns > 0){
		arrangement::arrangeChannels[activeArrangement][editTrack].addPattern(arrangementEnd, newPatt);
	}
	*/
	
	// void removePatternFromTrack(const patMem::pattern_t patt){
		// arrangement::arrangeChannels[activeArrangement][editTrack].addPattern(editTimelinePosition, patt);
		//arrangementEnd - maybe reduce
	// }
	
} //end arrange namespace
namespace MIDIPortScreen{
	
	std::bitset<gc::numberOfMIDIIns> selectedInputs;
	std::bitset<gc::numberOfMIDIOuts> selectedOutputs;
	
	bool isInputSelected = true; //Else output is selected
	
	void setCorrectListActive(){
		list::setActiveList(isInputSelected ? &list::MIDIInput : &list::MIDIOutput);
	}
	
	void setActivePort(const int button){
		if(button >= gc::numberOfMIDIIns + gc::numberOfMIDIOuts){
			notifications::dataOutOfRange.display(); 
			return;
			}
		if(button<gc::numberOfMIDIIns){
			if(!isShiftHeld){
				selectedInputs.reset();
			}
			selectedInputs.flip(button);
			isInputSelected = true;
		}
		else{
			if(!isShiftHeld){
				selectedOutputs.reset();
			}
			selectedOutputs.flip(button-gc::numberOfMIDIIns);
			isInputSelected = false;
		}
		setCorrectListActive();
		draw::routingMatrix();
	}
	
	//Outputs:
	void toggleSendClock(){
		forEachIf(selectedOutputs, 
			[](unsigned int n){MIDIports::getMIDIOutputSettings(n).toggleSendClock();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleSendStartStop(){
		forEachIf(selectedOutputs, 
			[](unsigned int n){MIDIports::getMIDIOutputSettings(n).toggleSendStartStop();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleSendSeqControl(){
		forEachIf(selectedOutputs, 
			[](unsigned int n){MIDIports::getMIDIOutputSettings(n).toggleSendSeqControl();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleSendSSP(){
		forEachIf(selectedOutputs, 
			[](unsigned int n){MIDIports::getMIDIOutputSettings(n).toggleSendSSP();}
			//waste
		);
		draw::routingMatrix();
	}
	
	
	//Inputs:
	void toggleReceiveClock(){
		forEachIf(selectedInputs, 
			[](unsigned int n){MIDIports::getMIDIInputSettings(n).toggleReceiveClock();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleReceiveStartStop(){
		forEachIf(selectedInputs, 
			[](unsigned int n){MIDIports::getMIDIInputSettings(n).toggleReceiveStartStop();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleReceiveSeqControl(){
		forEachIf(selectedInputs, 
			[](unsigned int n){MIDIports::getMIDIInputSettings(n).toggleReceiveSeqControl();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void toggleReceiveSSP(){
		forEachIf(selectedInputs, 
			[](unsigned int n){MIDIports::getMIDIInputSettings(n).toggleReceiveSSP();}
			//waste
		);
		draw::routingMatrix();
	}
	
	void setRecordType(const int button){
		forEachIf(selectedInputs, 
			[button](unsigned int n){MIDIports::getMIDIInputSettings(n).setRecordType(button);}
			//waste
		);
		draw::routingMatrix();
	}
	void setPreviewType        (const int button){
		forEachIf(selectedInputs, 
			[button](unsigned int n){MIDIports::getMIDIInputSettings(n).setPreviewType(button);}
			//waste
		);
		draw::routingMatrix();
	}
	void setRoutingType        (const int button){
		forEachIf(selectedInputs, 
			[button](unsigned int n){MIDIports::getMIDIInputSettings(n).setRoutingType(button);}
			//waste
		);
		draw::routingMatrix();
	}
	void setRoutingDestination (const int button){
		forEachIf(selectedInputs, 
			[button](unsigned int n){MIDIports::getMIDIInputSettings(n).setRoutingDestination(button);}
			//waste
		);
		draw::routingMatrix();
	}
	
	void traverseList(const int b){
		interface::all::listMove(b);
		draw::routingMatrix();
	}
	
	void setActiveParameter(const int b){
		interface::all::jumpToListItem(b); 
		draw::routingMatrix();
	}
	
}//end routing namespace
namespace transpose{
	int snapBackToEditPatternMode = false;
	void goToTransposeMode(int u){//int u so it only triggers on a data button press
		modes::switchToMode(modes::transpose, true);
		snapBackToEditPatternMode = true;
	}
	
	void returnToPatternMode(){
		if(snapBackToEditPatternMode){
			modes::switchToMode(modes::editPattern, true);
		}
		snapBackToEditPatternMode = false;
	}
	
	void setTranspose(const int button){
		int note = gPFK(button);
		int transpose = note - 48;
		Sequencing::getActivePattern().setTranspose(transpose);
		draw::transpose();
	}
	
	void jumpBackToPatternMode(const int u){
		returnToPatternMode();
	}
}//end transpose namespace
namespace debug{
	int memoryPage = 0;
	bool zoomed = false;
	uint16_t selectedBlock = 0;
	
	void setMemoryPage(const int button){
		memoryPage = button;
		draw::updateScreen();
	}
	
	void changeSelectedColumn(const int dir){
		if(dir){
			if(selectedBlock < blocks::getMaxBlockNum()){
				selectedBlock++;
			}
		}
		else if(selectedBlock > 0){
			selectedBlock--;
		}
	}
	
	void changeSelectedRow(const int dir){
		int rowWidth = zoomed ? draw::memoryUsageConstants::zoomedBlockWidth : draw::memoryUsageConstants::blockWidth;
		if(dir){
			if(selectedBlock < blocks::getMaxBlockNum() - rowWidth){
				selectedBlock += rowWidth;
			} else {
				selectedBlock = blocks::getMaxBlockNum();
			}
		} else {
			if(selectedBlock > rowWidth){
				selectedBlock -= rowWidth;
			} else {
				selectedBlock = 0;
			}
		}
		draw::updateScreen();
	}
	
}//ENd debug namespace
namespace selection{
	//bool selectSubstepsWithSteps = false;
	//int8_t inSubstepMode = -1; //-1 = not in mode
	bool firstStepOfRangeSet = false;
	int8_t firstStepOfRange[3] = {-1,-1,-1};
	//int8_t lastStepOfRange[3] = {-1,-1,-1};
	
	// enum class viewMode{
		// whole
		// bar
		// step
	// }

	enum class selType {
		set	,
		add	,
		remove	,
		toggle	,
	};
	
	const char* const selTypeNames[] = {
		"set"	,
		"add"	,
		"remove"	,
		"toggle"	,
	};
	
	selType selectionType = selType::set;
	
	const char* getSelTypeName(){
		return selTypeNames[static_cast<int>(interface::selection::selectionType)];
	}
	
	void setSelectionType(const selType type){
		selectionType = type;
	}
	void setSelectionTypeSet	(){setSelectionType(selType::set	);}
	void setSelectionTypeAdd	(){setSelectionType(selType::add	);}
	void setSelectionTypeRemove	(){setSelectionType(selType::remove	);}
	void setSelectionTypeToggle	(){setSelectionType(selType::toggle	);}	
	
	
	void affectSelection(const int start, int end = -1, const bool selectSubstepsToo = settings::editSubstepsAlso, const selType type = selectionType){
		if(end == -1){//So the function can be called to modify a single step
			end = start;
		}
		if(type == selType::set){
			stepSelection.reset();
		}
		for(int i = start; i <= end; i++){
			//Set will not work properly because it will clear. Look into this
			if((!selectSubstepsToo && (i%16 == 0)) || selectSubstepsToo){//So only steps get selected, if applicable
				switch (type){
					case selType::set:
					case selType::add:
						stepSelection.set(i);
						break;
					case selType::remove:
						stepSelection.reset(i);
						break;
					case selType::toggle:
						stepSelection.flip(i);
						break;
				}
			}
		}
		draw::updateScreen();
	}
	
	int getStart(){
		return 
			firstStepOfRange[0] * 256	+
			firstStepOfRange[1] * 16	+
			firstStepOfRange[2] * 1	;
	}

	void exitSubstepMode(){
		editStepForSubstep = -1;
		draw::selectionVars();
	}
	bool isInSubstepMode(){
		return editStepForSubstep != -1;
	}
	
	int getStepValue(const int button){
		return isInSubstepMode() ? editStepForSubstep : button;
	}
	int getSubstepValue(const int button){
		return isInSubstepMode() ? button : 0;
	}
	
	void enterSubstepSelect(const int button){
		editStepForSubstep = button;
		draw::selectionVars();
	}
	
	void setViewBarOrRange(const int button){
		if(isShiftHeld){
			if(!firstStepOfRangeSet){
				firstStepOfRange[0] = button;
				firstStepOfRange[1] = 0;
				firstStepOfRange[2] = 0;
				firstStepOfRangeSet = true;
			} 
			else {
				int start = getStart();
				int end = button * 256 + 255;
				affectSelection(start, end);
				firstStepOfRangeSet = false;
			}
		}
		else {
			viewBar = button;
			draw::selectionVars();
		}
	}
	
	void selectAllInBar(const int button){
		affectSelection(button*256, button*256 + 255);
	}
	
	void selectStepOrSubstep(const int button){
		if(isShiftHeld){
			if(!firstStepOfRangeSet){
				
				firstStepOfRange[0] = viewBar;
				firstStepOfRange[1] = getStepValue(button);
				firstStepOfRange[2] = getSubstepValue(button);
				
				firstStepOfRangeSet = true;
			}
			else {
				int start = getStart();
				int end = (viewBar * 256) + (getStepValue(button)*16) + getSubstepValue(button) + (isInSubstepMode() ? 0 : 15);
				affectSelection(start, end);
				firstStepOfRangeSet = false;
			}
		} 
		else{
			int step = getStepValue(button);
			int substep = getSubstepValue(button);
			int start = (viewBar * 256) + (step*16) + substep;
			int end = start + (isInSubstepMode() ? 0 : 15);
			bool selectSubstep = isInSubstepMode() ? true : settings::editSubstepsAlso;
			affectSelection(start, end, selectSubstep);
		}
	}
}//end select namespace
namespace modeSelect{
	
	modes::modeType typeToDisplay = modes::modeType::creation;
	int8_t modesDisplaying[16] = {0}; // List of IDs
	
	void getRelevantModes(){
		for(int i=0; i<16; i++){
			modesDisplaying[i] = -1;
		}
		
		int counter = 0;
		modes::forEachMode([&counter](modes::mode m){
			if (m.getType() == typeToDisplay){
				modesDisplaying[counter] = m.getID();
				counter++;
			}
		});
	}
	
	void setDisplayType(const int button){
		if(button<4){
			typeToDisplay = static_cast<modes::modeType>(button);
			getRelevantModes();
			draw::modeList();
		}
	}
	
	void goToMode(const int button){
		int id =	modesDisplaying[button];
		modes::mode& switchTo = modes::getModeByID(id);
		if(modes::isValid(switchTo)){
			modes::switchToMode(switchTo, true);
		}
	}
	
	void begin(){
		getRelevantModes();
	}
}//end modeSelect namespace

namespace process{
	patMem::pattern_t sourcePattern;
	patMem::pattern_t activePattern;
	
	enum class rangeState{
		rangeStart	= 0,
		rangeEnd	= 1,
	};
	
	const char* const rangeStrings[] = {
		"Range start",
		"Range end",
	};
	
	static rangeState rangeSelectState = rangeState::rangeStart;
	
	void drawActivePatternNameAndNum(){
		draw::patternNameAndNumber(activePattern);
	}
	
	void updateDisplay(){
		//scrn::rangeMode.update(rangeStrings[static_cast<int>(rangeSelectState)]);
		// char startPos[6] = {0}; 
		// getRangeStart().makeString(startPos, ':', 2);
		// scrn::rangeStart.update(startPos);
		
		// char endPos[6] = {0}; 
		// getRangeEnd().makeString(endPos, ':', 2);
		// scrn::rangeEnd.update(endPos);
		
		// if(rangeSelectState == rangeState::rangeStart){
			// scrn::rangeStart.showActive();
			// scrn::rangeEnd.showInactive();
		// } else {
			// scrn::rangeEnd.showActive();
			// scrn::rangeStart.showInactive();
		// }
		drawActivePatternNameAndNum();
	}
	
	void setRangeSelectState(rangeState newState){
		rangeSelectState = newState;
		updateDisplay();
	}
	
	void rangeSet(const int button){
		static int rangeStart = 0;
		switch(rangeSelectState){
			case rangeState::rangeStart:
				rangeStart = (viewBar * 256) + (button * 16);
				setRangeSelectState(rangeState::rangeEnd);
				break;
			case rangeState::rangeEnd:
				const int rangeEnd = (viewBar * 256) + (button * 16) + 15;
				selection::affectSelection(rangeStart, rangeEnd, true, selection::selType::set);
				setRangeSelectState(rangeState::rangeStart);
				updateDisplay();
				break;
		}
	}
	
	void gotoStart()	{setRangeSelectState(rangeState::rangeStart);	}
	void gotoEnd()	{setRangeSelectState(rangeState::rangeEnd);	}
	
	void setActivePatternForProcess(patMem::pattern_t patt = Sequencing::getActivePattern()){
		activePattern = patt;
		drawActivePatternNameAndNum();
	}
	
	void setActivePatternIfNone(){
		if(!activePattern.isValid()){
			setActivePatternForProcess(Sequencing::getActivePattern());
		}
	}
	
	void setActivePatternForProcessFromButtons(int button){
		static bool firstPress = true;
		static int lastKey;
		if(firstPress){
			scheduled::newEvent(scheduled::lOE::patternSwitch, [button](){
				if(Sequencing::getActiveTrack().doesTrackBankHavePattern(button)){
					patMem::pattern_t patt = Sequencing::getActiveTrack().getTrackBankPattern(button);
					setActivePatternForProcess(patt);
				} else {
					notifications::patternDoesNotExist.display();
				}
				firstPress = true;
			}, 750);
			firstPress = false;
			lastKey = button;
		} 
		else{
			scheduled::clearEvent(scheduled::lOE::patternSwitch);
			const uint32_t newPattern = (lastKey * 16) + button;
			setActivePatternForProcess(newPattern);
			firstPress = true;
		}
	}
	
	void clearRange(){
		forEachStep([](patMem::position p){
			p.print();
			process::activePattern.clearStepOfNotes(p);
		}, applyOnEntirePattern);
	}
	
	//std::function<void(const int)> processSetter;
	
	
	void addProcessToTrack(const int data){
		static int pos = -1;
		// processSetter = std::bind(Sequencing::getActiveTrack().addProcess, data);
		// modes::switchToMode(modes::processSelection, false);
		if(pos == -1){//Firstly, select position
			lg("stage1");
			pos = data;
		} else { //Then select effect
			lg("stage2");
			Sequencing::getActiveTrack().addProcess(pos, data);
			pos=-1;
		}
		lg("endAdd");
	}
	
	void removeProcessFromTrack(const int position){
		Sequencing::getActiveTrack().removeProcess(position);
	}
	
	void reorderProcesses(const int position){
		ASSERT(position < Sequencing::processesPerTrack);
		static int from = -1;
		if(from == -1){
			from = position;
			return;
		} else {
			Sequencing::getActiveTrack().reorderProcesses(from, position);
			from = -1;
		}
	}
	
	void goToProcess(const int position){
		//Serial.print("implement this");
		//TODO: THIS
	}
	
	void changeViewMode();
	
}//End process namespace
namespace copy{
	
	bool clearBeforeCopy = false;
	bool wrapPaste = false;
	
	void drawSourcePattern(){
		if(process::sourcePattern.isValid()){
			scrn::sourcePattNum.update(process::sourcePattern.getAddress());
			char name[9] = {0};
			process::sourcePattern.getName(name);
			scrn::sourcePattName.update(name);
		} else {
			scrn::sourcePattName.update("nosource");
		}
	}
	
	void updateDisplay(){
		drawSourcePattern();
		process::updateDisplay();
	}
	
	void pasteTo(const int button){
		if(!process::activePattern.isValid()){
			notifications::noDestinationPattern.display();
			return;
		}
		if(!process::sourcePattern.isValid()){
			notifications::noSourcePattern.display();
			return;
		}
		
		int firstSetStep = utl::getFirstSetBit(stepSelection);
		if(firstSetStep == -1){
			notifications::noStepSelection.display();
			return;
		}
		
		const patMem::position firstStepPosition = {static_cast<uint16_t>(firstSetStep)};
		const patMem::position firstDestinationPos = {viewBar, static_cast<uint8_t>(button)};
		
		patMem::length_t wrapLength = wrapPaste ? process::activePattern.getLength() : patMem::length_t{true};
		
		patMem::pattern_t tempBuffer = patMem::pattern_t{true};
		if(!tempBuffer.isValid()){return;}
		tempBuffer.clearAll();
		
		forEachStep([&tempBuffer](patMem::position p) mutable{
			//patMem::position offsetDestinationPos = patMem::subtract(p, p, {true});
			tempBuffer.copyStep(process::sourcePattern, p, p, false);
		});
		
		forEachStep([firstDestinationPos, firstStepPosition, wrapLength](patMem::position p){
			//patMem::position offsetDestinationPos = patMem::add(firstDestinationPos, patMem::subtract(p, firstStepPosition, wrapLength), wrapLength);
			patMem::position offsetDestinationPos = patMem::add(firstDestinationPos, p, wrapLength);
			//patMem::position offsetSourcePos = patMem::add(firstDestinationPos, p, wrapLength);
			process::activePattern.copyStep(process::sourcePattern, p, offsetDestinationPos, clearBeforeCopy);
		});
		
		tempBuffer.clearAll();
		tempBuffer.destroy();
	}
	
	void selectAsSource(){
		process::sourcePattern = process::activePattern;
		drawSourcePattern();
	}

}//end copy namespace
namespace quantise{
	bool wrapQuantise = true;
	
	const uint8_t quantiseLevels[]{
		1,2,4,8,16,32,64,
	};
	
	const char* const quantiseLevelStrings[] {
		"1/1"	,
		"1/2"	,
		"1/4"	,
		"1/8"	,
		"1/16"	,
		"1/32"	,
		"1/64"	,
	};
	
	//Originally, one woudl set the quantise level and then apply it with a separate click. This selects and applies it
	void apply(const uint8_t level){
		const uint8_t leveloftwo = quantiseLevels[level];
		forEachStep([leveloftwo](patMem::position p){
			process::activePattern.quantiseStep(p, leveloftwo, wrapQuantise);
		}, applyOnEntirePattern);
	}
	
	void applyQuantise(int p_quantiseLevel){
		apply(p_quantiseLevel);
	}
	
}//end quantise namespace

namespace dataEdit{
	
	dataEvents::dataEventType activeEventType;
	
	void addEventOnStep(const patMem::position p){
		
	}
	
	void editParameterOrData(const int button){


	}
	
	
}//end dataEvents namespace

}//End interface namespace
#endif