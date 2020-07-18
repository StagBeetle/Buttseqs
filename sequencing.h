//Storing the note data and exporting it
#ifndef Sequencing_h
#define Sequencing_h
#include <functional>
//#include <algorithm> 
#include <vector> 
#include <array>
#include <bitset>	
#include "notifications.h"	
#include "Utility.h"
#include <string> // Just for testing
#include <string.h>
#include <float.h>
#include "patternStorage.h"
#include "MIDInputOutput.h"
#include "bitsetlite.h" //Volatile
#include "processes.h" 

namespace Sequencing{
	using patMem::note;
	using patMem::position;
	using patMem::notePos;
	using patMem::noteOff;
	using patMem::noteList;
	
	//Constants:
	
	const int noteOffPolyphony = 16; //How many notes can be playing at once per channel
	const uint8_t PPS = 48; // 6 Pulses per Step is Equal to 24 PPQN - what midi clock uses - 48 is LCM of 16 and 6
	const uint8_t PPSS = PPS / 16; //Pulses per substep
	const uint8_t MIDIPPS = 6;
	//const int maxNotesPerStep = 8; //How many notes can be programmed on each step
	//const int maxTracks = 16; //Define in forwarddec.h
	const int shuffleLevels = 16;
	const int numberOfTimings = 16;
	const int maxBarsPerPattern = 16; //How many notes can be programmed on each step
	const int maxCCPerStep = 4; //How many CC can be programmed on each step
	const int maxAuxPerStep = 4;
	//const int processesPerTrack = 16;
	volatile uint8_t legatoOffset = 1; //The number of ticks after a note has played to schedule a noteoff for the legato note
	
	volatile uint8_t midiSyncExternal = 0;
	
	bool adjustNoteLengthByShuffle = true;
	
	const int maxRecordingNotes = 16;
	patMem::notePos recordNoteBuffer[maxRecordingNotes];
	
	int8_t globalTranspose = 0;
	//bool isSeqPlaying = false;

	int16_t loadPatternToMemory(const char* patternName){
		return 0;
		}

	enum class trackType {
		poly = 0,
		mono,
		drum,
		numberOf
	};
	
	const char* const trackTypeString [] = {
		"poly",
		"mono",
		"drum",
	};
		
	class track{
		private:
			static const int bankSlots = 16;
			static const unsigned int maxTick = 16 * maxBarsPerPattern * PPS; //12288;
			
			volatile int8_t transpose = 0;
			volatile const uint8_t trackNum;
			volatile uint8_t channel = 1; //1-16
			volatile uint8_t midiPort = gc::numberOfMIDIOuts; //off=0, 1-4, USB=5
			volatile bool isMuted = false;
			volatile bitset_lite<bankSlots> usedBankSlots;
			//volatile uint8_t 
			volatile patMem::pattern_t activePattern;
			volatile patMem::pattern_t trackBank[bankSlots];
			volatile int8_t currentSubstep = -1;
			volatile int16_t currentStep = -1;
			volatile noteOff scheduledNoteOffs[noteOffPolyphony]; //Notes that end on each step
			volatile trackType m_trackType = trackType::poly;
			volatile uint8_t realShuffle = 0;
			volatile uint8_t realPatternLength = 0; //The actual length of the pattern
			volatile uint8_t realStepsPerBar = 0; //The actual length of the pattern
			//uint8_t realBarsPerPattern = 0; //The actual length in bars of the pattern
			volatile bool realStretchToBar = false;
			volatile note legatoNote;
			volatile uint8_t legatoNoteChannel; //In the possible situation channel is changed midway through legato note
			volatile uint8_t legatoNotePort = 0;
			volatile bitset_lite<128> mutedNotes;
			volatile process::processStack pStack;
			volatile uint8_t soundSourceIndex = 0;
			volatile bool temporaryMute = false; //Mute when new notes are being played when recording.
		public:
			static uint8_t channelInc;
			
			track() :
				trackNum(channelInc),
				channel(channelInc+1){
					channelInc++;
				}
			
			void setSoundEngine(const int index) volatile {
stopTimer();
				soundSourceIndex = index;
startTimer();
			}
			
			const char* getSoundEngineName() const volatile {
				return audio::getSoundEngineName(soundSourceIndex);
			}
			
			void playNoteOn(const note theNote) volatile const {
				// lgc("seq::playNoteOn:");lg(midiPort);
				MIDIports::sendNoteOn(theNote.getPitch(), theNote.getVelocity(), channel, midiPort);
				if(audio::isValidEngine(soundSourceIndex)){audio::getSoundEngine(soundSourceIndex)->noteOn(theNote.getPitch(), theNote.getVelocity());}
			}
			
			void playNoteOff(const noteOff theNote) volatile const{
				int port = theNote.getPort();
				// lgc("seq::playNoteOff:");lg(port);
				MIDIports::sendNoteOff(theNote.getPitch(), 0, theNote.getChannel(), port);
				if(audio::isValidEngine(soundSourceIndex)){audio::getSoundEngine(soundSourceIndex)->noteOff(theNote.getPitch());}
			}
			
			void playNoteOff(const uint8_t theNote) volatile const{
				// lgc("seq::playNoteOffuint8_t:");lg(midiPort);
				MIDIports::sendNoteOff(theNote, 0, channel, midiPort);
				if(audio::isValidEngine(soundSourceIndex)){audio::getSoundEngine(soundSourceIndex)->noteOff(theNote);}
			}
				
			void toggleMute() volatile {
stopTimer();
				isMuted = !isMuted;
				if(isMuted){
					endLegatoNote();
				}
startTimer();
			}
			
			void mute() volatile {
stopTimer();
				isMuted = true;
				endLegatoNote();
startTimer();
			}
			
			void unmute() volatile {
stopTimer();
				isMuted = false;
startTimer();
			}
			
			void setTemporaryMute() volatile {
				temporaryMute = true;
			}
				
			bool getMuted() volatile const{
				return isMuted;
			}
			
			//Pattern stuff				
			patMem::pattern_t getActivePattern() const volatile {
				return activePattern;
			}
			
			void removePattern() volatile {
stopTimer();
				activePattern.invalidate();
startTimer();
			}
			
			void switchPattern(const patMem::pattern_t newPattern) volatile {
				//This should probably clear the pattern bar and step positions
				if(patMem::doesPatternExist(newPattern)){
stopTimer();
					activePattern = newPattern;
startTimer();
					}
				else {
					notifications::patternDoesNotExist.display();
					}
			}
			
			patMem::pattern_t newPattern() volatile {//Return a pattern that can be chcked for validity
stopTimer();
				patMem::pattern_t newPatt = {true};
				if (newPatt.isValid()){
					activePattern = newPatt;
					return newPatt;
				}
				return {};
startTimer();
			}
			
			void addOrRemoveNote(const note noteToAdd, const position pos, const bool isMonoPatt) volatile {
stopTimer();
				if(activePattern.isValid()) {
					if(isMonoPatt){
						bool isNoteOnStep = activePattern.getNoteOnStep(pos, noteToAdd).isValid();
						activePattern.clearStepOfNotes(pos);
						if(!isNoteOnStep){//Add the step if it wasn't there, else just erase the step
							activePattern.addOrUpdateNote(noteToAdd, pos);
						}
					} else {//Not a mono pattern:
						activePattern.addOrRemoveNote(noteToAdd, pos);
					}
				} else {//if invalid, try to add a new pattern
					patMem::pattern_t newPatt = {true};
					if (newPatt.isValid()){
						activePattern = newPatt;
						addPatternToBank(activePattern);
						activePattern.addOrUpdateNote(noteToAdd, pos);
					}
				}
startTimer();
			}
			
			void addOrUpdateNote(const note noteToAdd, const position pos, const bool isMonoPatt) volatile {
stopTimer();
				if(activePattern.isValid()){
					if(isMonoPatt){
						activePattern.clearStepOfNotes(pos);
						activePattern.addOrUpdateNote(noteToAdd, pos);
					} else {//Not a mono pattern:
						activePattern.addOrUpdateNote(noteToAdd, pos);
					}
				} 
				else {//if invalid, try to add a new pattern
					patMem::pattern_t newPatt = {true};
					if (newPatt.isValid()){
						activePattern = newPatt;
						addPatternToBank(activePattern);
						activePattern.addOrUpdateNote(noteToAdd, pos);
					}
				}
startTimer();
			}
			
			void removeNote(const uint8_t pitch, const position pos) volatile {
stopTimer();
				activePattern.removeNote({pitch}, pos);
startTimer();
			}
			//Other stuff
				
			void setTranspose(int8_t transposeAmount) volatile {
stopTimer();
				transpose = transposeAmount;
startTimer();
				}
				
			bool doesTrackBankHavePattern(const int bankSlotNum) volatile const{
				return usedBankSlots.test(bankSlotNum);
				}
				
			patMem::pattern_t getTrackBankPattern(const int bankSlotNum) volatile const{
				return trackBank[bankSlotNum];
				}
				
			void addPatternToBank(const patMem::pattern_t newPattern) volatile {
stopTimer();
				for(int i = 0; i<bankSlots; i++){
					if(!usedBankSlots.test(i)){
						trackBank[i] = newPattern;
						usedBankSlots.set(i);
						return;
						}
					}
				notifications::trackBankFull.display();
startTimer();
				}
				
			void removePatternFromBank(const int bankSlotNum) volatile {
stopTimer();
				usedBankSlots.reset(bankSlotNum);
startTimer();
				}
				
			void removePatternFromBankIfExists(patMem::pattern_t patt) volatile {
stopTimer();
				for(int i = 0; i<bankSlots; i++){
					const patMem::pattern_t p = trackBank[i];
					if(usedBankSlots.test(i) && p == patt){
						removePatternFromBank(i);
						break;
						}
					}
startTimer();
				}
				
			bool areTrackBankSlotsAvailable() volatile const{
				for(int i = 0; i<bankSlots; i++){
					if(usedBankSlots.test(i) == 0){
						return true;
						}
					}
					return false;
				}
				
			void removePatternFromTrack() volatile {
stopTimer();
				removePattern();
				resetPattern();
startTimer();
				}
				
			void removePatternFromTrackIfActive(const uint16_t patternNum) volatile {
stopTimer();
				if(activePattern.getAddress() == patternNum){
					removePatternFromTrack();
					}
startTimer();
				}
				
			void assignPatternToTrack(uint16_t newPattern) volatile {
stopTimer();
				if(patMem::doesPatternExist(newPattern)){
					switchPattern(newPattern);
					}
				else {
					notifications::patternDoesNotExist.display();
					}
startTimer();
				}
			
			void advanceStep() volatile {
stopTimer();
				currentSubstep++;
				if(currentSubstep % 16 == 0){
					currentSubstep = 0;
					currentStep++;
					if(trackNum == interface::editTrack){
						scheduled::newEvent(scheduled::lOE::drawPianoRoll, []{
							draw::drawPianoRollChaser();
							LEDfeedback::stepChaser(LEDfeedback::getLEDSet(buttons::keySet::step));
						}, 0); //Delay this so it doesn't operate from within doSequencing
					}
					if(currentStep >= realPatternLength){
						currentStep = 0;
						temporaryMute = false; // Reset mute on end of bar
					}
					
					if(currentStep % realStepsPerBar == 0){
						setTrackVarsFromPattern();
						// if(realPatternLength > realStepsPerBar && interface::settings::pianoRollFollowPlay){//If the pattern has more than one bar for duisplay
							// scheduled::newEvent(
								// scheduled::lOE::drawPianoRoll, 
								// []{draw::drawPianoRoll(false);}, 
								// 0
							// ); //Delay this so it doesn't operate from within doSequencing
						// }
					}
				}
startTimer();
			}
			
			void resetPattern() volatile {//Set the pattern play point to -1 so next tick it is zero
stopTimer();
				currentSubstep = -1;
				currentStep = -1;
				setTrackVarsFromPattern(); //Maybe doesn't work;
				setStretchToBar();
startTimer();
			}
				
			bool setShuffleFromPattern() volatile {
stopTimer();
				int patternShuffle = activePattern.getShuffle();
				bool hasChanged = false;
				hasChanged = realShuffle != patternShuffle;
				realShuffle = activePattern.getShuffle();
startTimer();
				return hasChanged;

			}
				
			void setTrackVarsFromPattern() volatile {//Runs every tick
stopTimer();
				uint8_t newPatternLength = activePattern.isValid() ? activePattern.getLengthInSteps() : 1;
				uint8_t newStepsPerBar = activePattern.isValid() ? activePattern.getStepsPerBar() : 1;
				
				if(currentStep < newPatternLength){
					realPatternLength = newPatternLength;
				}
				
				//const int bar	= currentStep / realStepsPerBar;
				const int step	= currentStep % realStepsPerBar;
				
				if(!realStretchToBar && newStepsPerBar > step){
					realStepsPerBar = newStepsPerBar;
				}

				// I don't know why this was here:
				// if(trackNum == interface::editTrack && ){
					// scheduled::newEvent(scheduled::lOE::drawPianoRoll, []{
						// draw::drawPianoRollSteps();
					// }, 0); //Delay this so it doesn't operate from within doSequencing
				// }
startTimer();
			}
				
			bool setStretchToBar() volatile {
stopTimer();
				bool newStretchToBar = activePattern.isStretchToBar();
				bool hasChanged = newStretchToBar != realStretchToBar;
				realStretchToBar = newStretchToBar;
				realStepsPerBar = activePattern.getStepsPerBar();
startTimer();
				return hasChanged;

			}
				
			int getShuffleOffset() volatile {//Looks at the current step and the shuffle amount
stopTimer();
				int positionInTwoSteps = (currentStep % 2) * 16 + currentSubstep;
				int positionalOffset = (positionInTwoSteps < 16) ? positionInTwoSteps : 32 - positionInTwoSteps;
				int shuffleOffset = ((realShuffle * positionalOffset) + 15) / 16; //Always round up so it removes more to ensure there are no legatoBois
startTimer();
				return shuffleOffset;
				//240 max substep delay?

			}			
				
			void doTick() volatile {//TIMER FUNCTION
				advanceStep();
				// lg(currentStep);
				// lg(currentSubstep);
				noteList notesToPlay = getNotesOnCurrentStep();
				
				bool isPatternMuted = getMuted();
				for(auto & theNote: notesToPlay){ //For each note at the step
				
					if (theNote.isValid()){ //If the note is valid - I think it should always be valid,?
						bool noteRemoved = false;
						if(interface::isRecording() && trackNum == interface::editTrack && interface::isShiftHeld){//Recording notes - will only remove 
							for(int i = 0; i<maxRecordingNotes; i++){
								notePos np = recordNoteBuffer[i];
								if(np.isValid() && np.n.getPitch() == theNote.getPitch()){
									// lg("rem");
									addOrRemoveNote(theNote, getPlayPosition(), false);
									noteRemoved = true;
									break;
								}
							}
						}
						
						if(isNoteMuted(theNote.getPitch()) || isPatternMuted || noteRemoved || temporaryMute){continue;}
						
						theNote.setPitch(theNote.getPitch() + activePattern.getTranspose());
						int velocityMod = theNote.getAccent() ? activePattern.getAccentVelocity() : activePattern.getVelocity();
						theNote.setVelocity(theNote.getVelocity() + velocityMod);
						
						note legatoNote_tmp;
						typecopy(legatoNote_tmp, legatoNote);
						
						//Only play the note if it is not the legato note:
						const bool noteSameAsLegato = legatoNote_tmp.isValid() && (theNote.getPitch() == legatoNote_tmp.getPitch());//If the new note is the same pitch as the legato note 
						const bool doubleLegato = noteSameAsLegato && theNote.getLegato(); //If the new note is ALSO a legato note and the same pitch
						
						if (doubleLegato){break;}//No action required
						
						//Only play the new note and do a note off for the legato if they are different
						if(!noteSameAsLegato){
							//Play the new note:
							playNoteOn(theNote);
						
							//Legato Noteoff and clear the legato note:
							if (legatoNote_tmp.isValid()){
								signed int position = addNoteOffToScheduled(legatoNote_tmp, legatoNoteChannel, legatoNotePort);
								if(position >= 0){//If has added the note...
									patMem::noteOff noteOff_tmp;
									typecopy(noteOff_tmp, scheduledNoteOffs[position]);
									noteOff_tmp.setLength(legatoOffset);
									typecopy(scheduledNoteOffs[position], noteOff_tmp);
								}
							}
						}
						
						//Always clear the noteoff if a new note has come in. If there is no legato note, this is superfluous
						legatoNote_tmp.clear();
						typecopy(legatoNote, legatoNote_tmp);
						
						//Add the current note off to scheduled or legato
						if (theNote.getLegato()){//Add it to legato...
							typecopy(legatoNote, theNote);
							legatoNoteChannel = channel;
							legatoNotePort = midiPort;
						} else {//...Or schedule a noteoff
							// lgc("seq::beforeAddNoteToScheduled:");lg(midiPort);
							addNoteOffToScheduled(theNote, channel, midiPort);
						}
					}
				}
				checkForNoteOffs();
				
			}//End do tick
				
			uint8_t getCurrentStep() const volatile{
				return currentStep % realStepsPerBar;
			}
				
			uint8_t getCurrentBar() const volatile{
				return currentStep / realStepsPerBar;
			}
				
			uint8_t getStepsPerBar() const volatile{
				return realStepsPerBar;
			}
			
			//IF it's less than (16 + shuffle), getStep (positionInTwoStep / (16 + shuffle)) * (16) //Rearrange so it does not go wrong
			//IF it's more than (16 + shuffle), getStep (positionInTwoStep - (16 + shuffle)) / ((16*2) - (16 + shuffle)) * 16
			
			// noteList applyProcessChain(noteList notes) volatile const{
				// // for(auto p: processes){
					// // if(!p.isValid()){break;}
					// // notes = p.apply(trackNum, notes);
				// // }
				// return notes;
			// }
	
			noteList getNotesOnStep(const patMem::position pos, const bool showProcess = true) volatile {//SHow process determines whether to use processed or original
				// lg("track.getNotesOnStep");
				if(!activePattern.isValid()){return {};}
				noteList notesOnStep = showProcess ? pStack.getNotesOnStep(pos) : getActivePattern().getNotesOnStep(pos);
				return notesOnStep;
			}
	
			noteList getNotesOnCurrentStep() volatile {
				if(!activePattern.isValid()){return {};}
				return getNotesOnStep(getPlayPosition(), true);
			}
				
			signed int addNoteOffToScheduled (const note theNote, const uint8_t theChannel, const uint8_t thePort) volatile {
stopTimer();
				// lgc("seq::addNoteOffToScheduled:");lg(thePort);
				for(byte j=0;j<noteOffPolyphony;j++){ // Check through the list of scheduled note offs to find one available
					patMem::noteOff noteOff_tmp;//For volatile stuff
					typecopy(noteOff_tmp, scheduledNoteOffs[j]);
					
					if (!noteOff_tmp.isValid()){//If there is not a note there
						int theLength = theNote.getLengthInSubsteps();
						noteOff_tmp.setNoteOff(theNote.getPitch(), theLength, theChannel, thePort); //Add the note to the array of noteoffs
						typecopy(scheduledNoteOffs[j], noteOff_tmp);
startTimer();
						return j; //Finish the for loop as the noteoff has been scheduled
					}
					
				}
				//If there is no space left in the array of noteoffs, terminate the note immediately
		// lg("Immediate");
				playNoteOff({theNote.getPitch(), theChannel, thePort});
startTimer();
				return -1;
				//notifications::noteOffMemoryFull();

			}
				
			void checkForNoteOffs() volatile {
stopTimer();
				for(int j=0; j<noteOffPolyphony; j++){ // Check through the array of scheduled note offs
					patMem::noteOff noteOff_tmp;//For volatile stuff
					typecopy(noteOff_tmp, scheduledNoteOffs[j]);
				
					if (noteOff_tmp.isValid()){ //If there is a note there
						noteOff_tmp.decrementLength(); //Remove one from the duration
						if (noteOff_tmp.getLength() < 0){ //If the note now has negative duration
							// lgc("seq::checkForNoteOffsInFor:");lg(scheduledNoteOffs[j].getPort());
							playNoteOff(noteOff_tmp);
							noteOff_tmp.clearNoteOff(); //Remove the note from array (sort of)
						}
						typecopy(scheduledNoteOffs[j], noteOff_tmp);
					}
				}
startTimer();
			}	
			
			void endLegatoNote() volatile {
stopTimer();
				patMem::note note_tmp;//For volatile stuff
				typecopy(note_tmp, legatoNote);
					
				if(note_tmp.isValid()){
					playNoteOff({note_tmp.getPitch(), legatoNoteChannel, legatoNotePort});
					note_tmp.clear();
					typecopy(legatoNote, note_tmp);
				}
startTimer();
			}
			
			void playAllNoteOffs() volatile {//When sequencer is stopped
stopTimer();
				for(int j=0;j<noteOffPolyphony;j++){
					patMem::noteOff noteOff_tmp;//For volatile stuff
					typecopy(noteOff_tmp, scheduledNoteOffs[j]);
					
					if (noteOff_tmp.isValid()){
						// lgc("seq::playAllNoteOffs:");lg(scheduledNoteOffs[j].getPort());
						playNoteOff(noteOff_tmp);
						noteOff_tmp.clearNoteOff();
						typecopy(scheduledNoteOffs[j], noteOff_tmp); //Only copy back if it is valid and therefore has changed
					}
					
				}
				endLegatoNote();
startTimer();
			}
			
			void setMIDIChannel(const int button) volatile {
stopTimer();
				channel = button+1;
startTimer();
			}
			
			void setMIDIPort(const int button) volatile {
stopTimer();
				// lgc("seq::setMIDIport:");lg(button);
				if(button > gc::numberOfMIDIOuts){
					notifications::dataOutOfRange.display();
					return;
				}
				midiPort = button;
startTimer();
			}
			
			const char* getMIDIportForUser() volatile const{
				// lg(midiPort);
				//return "ERROR";
				// lgc("seq::getMIDIport:");lg(midiPort);
				//return "!!!";
				return MIDIports::getName(midiPort);
			}
			
			uint8_t getMIDIPortNumber() const volatile {
				// lgc("seq::getMIDIportNumber:");lg(midiPort);
				return midiPort;
			}
			
			uint8_t getMIDIchannel() const volatile {//Returns 1-16
				return channel;
			}
			
			void setTrackMode(const int setter) volatile {
stopTimer();
				if(setter >= 0 && setter < static_cast<int>(trackType::numberOf)){
					m_trackType = static_cast<trackType>(setter);
				} else {
					notifications::dataOutOfRange.display();
				}
startTimer();
			}
			
			trackType getTrackMode() volatile const{
				return m_trackType;
			}
			
			const char* getTrackModeString() volatile const{
				return trackTypeString[static_cast<int>(m_trackType)];
			}
			
			bool isStretchToBar() volatile const {
				return realStretchToBar;
			}
			
			uint8_t getShuffle() volatile const {
				return realShuffle;
			}
			
			void toggleMutedNote(int note) volatile {
				mutedNotes.flip(note);
			}
			
			bool isNoteMuted(int note) volatile const {
				return mutedNotes.test(note);
			}
			
			patMem::position getPlayPosition() volatile const{
				const uint8_t bar	= currentStep / realStepsPerBar;
				const uint8_t step	= currentStep % realStepsPerBar;
				return {bar, step, static_cast<uint8_t>(currentSubstep)};
			}
			
			patMem::length_t getLength() volatile const{
				return {realPatternLength};
			}
			
			void addProcess(const int p, const int processID) volatile {
				pStack.addProcess(p, processID);
			}
	
			void removeProcess(const int p) volatile {
				pStack.removeProcess(p);
			}
			
			void reorderProcesses(const int from, const int to) volatile {
				pStack.reorderProcesses(from, to);
			}
			
			process::process_t getProcess(const int pos) volatile {
				return pStack.getProcess(pos);
			}
			
			int getNumProcesses() const volatile {
				return pStack.getNumProcesses();
			}
	
	};
	uint8_t track::channelInc = 0;
	volatile track trackArray[gc::numberOfTracks];
	//End of track class
	
	inline patMem::pattern_t getPattern(const int trackNum){
		return trackArray[trackNum].getActivePattern();
	}
	
	inline volatile track& getTrack(const int trackNum){
		return trackArray[trackNum];
	}
	
	volatile track& getActiveTrack(){
		return trackArray[interface::editTrack];
	}
	
	patMem::pattern_t getActivePattern() {
		return getPattern(interface::editTrack);
	}
	
	// /==
	// |=| equencing:
	// ==/
	
	enum class sequencerStatus : uint8_t {
		stopped = 0	,
		playing	,
		paused	,
	};
	sequencerStatus seqStatus;
	
	const char* const seqSta[] = {
		"stopped",
		"playing",
		"paused",
	};
	
	bool isSeqPlaying(){
		return seqStatus == sequencerStatus::playing;
	}
	
	static double roundingError = 0.0;
	long long tempoToMicrosPerBar(const double BPM){
		
		double measuresPerMinute = BPM / 4.0;
		double measuresPerMicrosecond = measuresPerMinute / (60.0 * 1000.0 * 1000.0);
		double microsecondsPerMeasure = 1.0 / measuresPerMicrosecond;
		int microsecondsPerMeasureInt = microsecondsPerMeasure;
		roundingError += microsecondsPerMeasure - (double)microsecondsPerMeasureInt;
		
		// static long long timeAtLastLog = 0;
		// if(millis() - timeAtLastLog > 100){
			// lgc("measuresPerMinute"	);lg(measuresPerMinute	);
			// lgc("measuresPerMicros"	);lg(measuresPerMicrosecond	);
			// lgc("microsPerMeasure"	);lg(microsecondsPerMeasure	);
			// lgc("microsPerMeasInt"	);lg(microsecondsPerMeasureInt	);
			// lgc("rounding   Error"	);lg(roundingError	);
			// lg();
			// timeAtLastLog = millis();
		// }
		
		if(roundingError >= 1.0){
			microsecondsPerMeasureInt ++;
			roundingError--;
		}
		return microsecondsPerMeasure;
	}
	
	double tempo = 120.00;
	
	void setTempo(const double newTempo){
		tempo = newTempo;
	}
	
	double getTempo(){
		return tempo;
	}

	volatile uint8_t playingBar = 0;
	volatile double lengthThroughBar = 0.0;
	volatile int loopStart = 0;
	volatile int loopEnd = 4;
	volatile uint8_t timeSignatureNumerator = 4;
	volatile long long timeAtLastCheck = 0;
	
	void setTimeAtLastCheck(){
		timeAtLastCheck = micros();
	}
	
	volatile bool syncOnBarChange = false;
	
	void setSyncOnBarChange(){
stopTimer();
		syncOnBarChange = true;
startTimer();
	}
	
	volatile int lastPlayedSubsteps[numberOfTimings] = {0}; //The steps for each timing that played last
	volatile int playingSubsteps[numberOfTimings] = {0}; //The steps that need to be played on this step - Does not really need to be static here
	
	//Just for testing:
	int ticksPerBarPerTrack[gc::numberOfTracks] = {0};
	
	void resetTicksPerBarPerTrack(){
		for(int i=0; i<gc::numberOfTracks;i++){
			ticksPerBarPerTrack[i] = 0;
		}
	}
	
	void printAndResetTicksPerBarPerTrack(){
		lgc("track");
		lgc(0);
		lgc(":");
		lg(ticksPerBarPerTrack[0]);
		resetTicksPerBarPerTrack();
	}
	//End for testing
	
	//Divide every 32 substeps (ie. two steps) into 16 divisions
	//For every shuffle value, precalculate on which of these divisions the substeps will fall on and save within a bitfield for each step
	//Foe every divisor step that plays, check if the the shuffle value is the same as the track value. If it is send a tick to the track
	
	//2steps * 16substeps * 16substepdivisors
	//EAch step is a bitfield of which shuffle values fall on this step
	
	const int tickDivisor = 16;
	const int numberOfUniqueShuffleSteps = 2;
	const int numberOfSubstepsPerStep = 16;
	
	//const int numberOfDivisions = 16 * numberOfSubstepsPerStep * tickDivisor;
	
	const int shuffleArrayLength = numberOfUniqueShuffleSteps * numberOfSubstepsPerStep * tickDivisor;
	
	uint16_t shuffleTicks[shuffleArrayLength]; //512;
	
	const uint16_t singleBits[16] = { //For bit masking
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};

	void setupShuffleTicks(){//Precalculated but maybe can be simplified
		for(int substep=0; substep<numberOfUniqueShuffleSteps*numberOfSubstepsPerStep; substep++){
			for(int shuffleLevel=0; shuffleLevel<shuffleLevels; shuffleLevel++){
				int shuffleMultiplier = (substep < 16 ) ? substep : 32 - substep;
				int index = (substep * 16) + (shuffleLevel * shuffleMultiplier); //Do as double and round to nearest reather than truncate?			
				// int index = (substep * 16); //Temp to check logic
				shuffleTicks[index] = shuffleTicks[index] | singleBits[shuffleLevel];
			}
		}
	}
	
	int getNumberOfDivisions(const int steps){
		int numberOfDivisions = steps * 16 * tickDivisor;
		numberOfDivisions = (numberOfDivisions / 4 ) * timeSignatureNumerator;
		return numberOfDivisions;
	}
	
	void resetTime(){//Reset everything
stopTimer();
		for(int i=0; i<numberOfTimings; i++){
			lastPlayedSubsteps[i] = ((i+1) * numberOfSubstepsPerStep * tickDivisor) - 1;
		}
		lengthThroughBar = 0.0;
		roundingError = 0.0;
		resetTicksPerBarPerTrack();
startTimer();
	}
	
	void setup(){
		setupShuffleTicks();
		resetTime();
	}
	
	volatile long times[10] = {0};

	void update(){
		const long long startTime = micros();
		//Get the time since the last check and then calculate how much further through the bar to go.
		
		//Calculate length through bar:
		if(midiSyncExternal){
			// Have an array of estimate starts calculated by counting ticks backwards
		} else {
			//
		}
		long long timeSinceLast = micros() - timeAtLastCheck;
		long long microsPerBar = tempoToMicrosPerBar(tempo); //Multiply by time sig here?
		double percentToAdd = (double)timeSinceLast / (double)microsPerBar;
		setTimeAtLastCheck(); //Should be here to switch between MIDI and internal?
		lengthThroughBar += percentToAdd;
		if(lengthThroughBar >= 1.0){
			playingBar ++;
			for(int i=0; i<gc::numberOfTracks; i++){//Sync the tracks to the main loop
				//trackArray[i].setStretchToBar();
				if(syncOnBarChange){
					trackArray[i].resetPattern();
					syncOnBarChange = false;
				}
			}
			if(playingBar >= loopEnd){
				playingBar = loopStart;
			}
			lengthThroughBar --;
			// lgc("playingBar: ");
			// lg(playingBar);
			//printAndResetTicksPerBarPerTrack(); //For testing
		}
		
		times[0] = micros() - startTime;
		
		//For every number of timings, convert the float length through bar into number of divisions
		for(int numberOfSteps = 1; numberOfSteps<=numberOfTimings; numberOfSteps++){
			uint8_t arrayIndex = numberOfSteps-1;
			int numberOfDivisions = getNumberOfDivisions(numberOfSteps);
			int lastSubstepThatShouldHavePlayed = (lengthThroughBar) * (double)(numberOfDivisions);
			playingSubsteps[arrayIndex] = lastSubstepThatShouldHavePlayed;
			if(numberOfSteps == 16){
				//
			}
		}
		
		times[1] = micros() - startTime;
		
		//Send Clock:
		
		///Commented out because it seems to cause a big lag
		// {
			// int clockPattern = 12;
			// int index = clockPattern-1;
			// int numberOfDivisions = getNumberOfDivisions(clockPattern);
			// int numberOfStepsPassed = ((playingSubsteps[index] - lastPlayedSubsteps[index]) + numberOfDivisions) % numberOfDivisions;
			// int clocksInABar = MIDIPPS * 16;
			// int ticksPerClock = numberOfDivisions / clocksInABar;
			// int numberOfTicksToPlay = ((lastPlayedSubsteps[index] % numberOfDivisions) + numberOfStepsPassed) / ticksPerClock; 
			// for(int i = 0; i<numberOfTicksToPlay; i++){
				// MIDIports::sendClock();
			// }
		// }
		
		times[2] = micros() - startTime;
		
		//cache type: - This is so for each of the 16 tracks * 16 possible shuffle levels, it doesn't have t recalculate which notes to play
		struct cache_entry{
			uint8_t value = 0;
			uint8_t step = 0;
			uint8_t shuffle = 0;
		} cache[gc::numberOfTracks];
		
		std::bitset<gc::numberOfTracks> trackAlreadyPlayed;
		
		//Go through each of the tracks and use appropriate timing
		
		bool redrawScreen = false; //Redraw screen if pattern changes - only do it once no matter how many tracks change
		
		
		for(int track = 0; track<gc::numberOfTracks; track++){
			if(track == 0){times[3] = micros() - startTime;}
			
			int stepNumber = trackArray[track].isStretchToBar() ? trackArray[track].getStepsPerBar() : 16;
			int index = stepNumber - 1;
			int shuffleNumber = trackArray[track].getShuffle();
			int numberOfDivisions = getNumberOfDivisions(stepNumber);
			
			//This is in case the track has gone past zero and allows the parameters of the track to change
			bool hasTrackAlreadyPlayed = trackAlreadyPlayed.test(track);
			
			//How many steps passed since last check:
			int numberOfStepsPassed = ((playingSubsteps[index] - lastPlayedSubsteps[index]) + numberOfDivisions) % numberOfDivisions;

			if(track == 0){times[4] = micros() - startTime;}
			
			//Check the cache:
			int numberOfStepsToPlay = 0;
			bool useCache = false;
			int cacheNum = 0;
			if(!hasTrackAlreadyPlayed){
				for(;cacheNum<gc::numberOfTracks; cacheNum++){
					if(cache[cacheNum].step == 0){
						break;
					}
					else if (cache[cacheNum].step == stepNumber && cache[cacheNum].shuffle == shuffleNumber){
						useCache = true;
						break;
					}
				}
			}
			
			if(track == 0){times[5] = micros() - startTime;}
			
			//Calculate what steps to play if cache unavailable:
			if(useCache){
				numberOfStepsToPlay = cache[cacheNum].value;
			} else {
				for(int i = 0; i < numberOfStepsPassed; i++){//Find what shuffle values are on the steps
					int actualStep = lastPlayedSubsteps[index] + 1 + i;// % numberOfDivisions; //number of divisions is not necessary here. The mod on next line deals with it
					if(hasTrackAlreadyPlayed && (numberOfStepsPassed - 1 + lastPlayedSubsteps[index] >= numberOfDivisions) && actualStep < numberOfDivisions){
						continue; //If the track has already played and changed, then carry on from zero so it stays in sync
					}
					numberOfStepsToPlay += (shuffleTicks[actualStep % shuffleArrayLength] & singleBits[shuffleNumber]) > 0;
				}
				if(!hasTrackAlreadyPlayed){
					cache[cacheNum].step = stepNumber;
					cache[cacheNum].shuffle = shuffleNumber;
					cache[cacheNum].value = numberOfStepsToPlay;
				}
			}
			
			if(track == 0){times[6] = micros() - startTime;}
			
			//Actually play the steps:
			for(int i = 0; i < numberOfStepsToPlay; i++){
				int actualStep = (lastPlayedSubsteps[index] + 1 + i) % numberOfDivisions;
				int shuffleStep = actualStep % shuffleArrayLength;
				bool hasChanged = false;
				if(actualStep == 0){
					hasChanged = trackArray[track].setStretchToBar() || trackArray[track].setShuffleFromPattern();
					if(interface::arrange::useArrangeMode){
						// lg(playingBar);
						patMem::pattern_t newPatt = arrangement::arrangeChannels[interface::arrange::activeArrangement][track].getCurrentPattern(playingBar);
						if(newPatt.isValid() && trackArray[track].getActivePattern() != newPatt){
							// scheduled::newEvent(
								// scheduled::lOE::misc, 
									// []{lg("pattswitch");}, 
								// 0
							// ); //Delay this so it doesn't operate from within doSequencing
							// // // // char name[9] = {0};
							// // // // newPatt.getName(name);
							// // // // lg(name);
							trackArray[track].switchPattern(newPatt);
							redrawScreen = true;
							//hasChanged = true;
						}
						else if(!newPatt.isValid()){
							trackArray[track].removePattern();
							redrawScreen = true;
							//hasChanged = true;
						}
					}//End useArrangeMode
				}
				else if(shuffleStep == 0 && (numberOfStepsToPlay - i < 16)){
					//Do not change if numberOfSteps to play is more than 16 as it may mess up timing. Can't remember why sixteen. I think it is the biggest number where all shuffle levels have the same step
					hasChanged = trackArray[track].setShuffleFromPattern();
				}
				if(hasChanged){//Redo track with new parameters
					trackAlreadyPlayed.set(track);
					track--;
					break;//I think this should be break. It was continue
				}
				trackArray[track].doTick();
				ticksPerBarPerTrack[track] ++ ;//For logging
			}
			if(track == 0){times[7] = micros() - startTime;}
			
		}//End for each track
		
		times[8] = micros() - startTime;
		
		if(redrawScreen){
			// if(modes::checkActive({modes::editSteps, modes::editNotes})){
				// scheduled::newEvent(
					// scheduled::lOE::drawPianoRoll, 
					// []{draw::updateScreen();}, 
					// 0
				// ); //Delay this so it doesn't operate from within sequencing
			// }
		}
		
		times[9] = micros() - startTime;
		
		//Set the last step to this one
		for(int stepIndex = 0; stepIndex<numberOfTimings; stepIndex++){
			lastPlayedSubsteps[stepIndex] = playingSubsteps[stepIndex];
		}
		
	} //End sequencer update function
	
	volatile long long lastSequencingTime = 0;
	volatile long long sequencingStartTime = 0;
	volatile bool slowFunc = false;//Use this to signal somewhere
	volatile long slowTime = 0;//Use this to signal somewhere
	
	volatile long averageDuration = 0;
	
	const int numberOfAverages = 10;
	const int multiplyFactor = numberOfAverages - 1;
	
	void setAverageDuration(long duration){
		averageDuration = (averageDuration * multiplyFactor + duration) / numberOfAverages;
	}
	
	void checkPlaying(){
stopTimer();
		sequencingStartTime = micros();
		const long long time = getTimeSinceLastSequence();
		if(time > 1000){slowFunc = true; slowTime = time;}
		lastSequencingTime = micros();
		if(seqStatus == sequencerStatus::playing){update();}
		long long sequenceEndTime = micros();
		long long duration = sequenceEndTime - sequencingStartTime;
		setAverageDuration(duration);
startTimer();
	}
	
	inline long long getTimeSinceLastSequence(){
		return micros() - lastSequencingTime;
	}
	
	int chaserUpdateFrequency = 10;
	
	int numberOfTimesBeginChaserIsCalled = 0;
	
	void beginChaser(){
		static bool alternator = false;
		numberOfTimesBeginChaserIsCalled++;
		//scheduled::lOE::listOfEvents type = alternator ? scheduled::lOE::arrangeChaser : scheduled::lOE::arrangeChaser2; //I think I fixed the error that this solves
		if(!isSeqPlaying()){return;}
		scheduled::newEvent(
			scheduled::lOE::arrangeChaser,
			[]{
				draw::arrangeChaser();
				beginChaser();
			}, 
			1000 / chaserUpdateFrequency
		);
		alternator = !alternator;
	}
	
	// void endChaser(){
		
	// }
	
	// void doSequencingIfTime(const bool regardless){
		// if((getTimeSinceLastSequence() > 500 || regardless) && !doingSequencing){
			// checkPlaying();
		// }
	// }
	
	void startSequencer(){
		if(seqStatus == sequencerStatus::playing){return;}
stopTimer();
		for(int i = 0; i<16; i++){
			trackArray[i].resetPattern(); //Reset each of the playing patterns to position zero
		}
		resetTime();
		clockReceive::resetClock();
		MIDIports::sendStart();
		setTimeAtLastCheck();
		playingBar = 0;
		lengthThroughBar = 0;
		seqStatus = sequencerStatus::playing;
		LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
		beginChaser();
startTimer();
	}
		
	void continueSequencer(){
		if(seqStatus == sequencerStatus::playing){return;}
stopTimer();
		setTimeAtLastCheck();
		MIDIports::sendContinue();
		seqStatus = sequencerStatus::playing;
		LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
		beginChaser();
startTimer();
	}
		
	void pauseSequencer(){
		if(seqStatus == sequencerStatus::paused){return;}
stopTimer();
		MIDIports::sendStop();
		setTimeAtLastCheck();
		seqStatus = sequencerStatus::paused;
		LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
		//endChaser();
startTimer();
	}
	
	void stopSequencer(){
		if(seqStatus == sequencerStatus::stopped){return;}
		if(seqStatus != sequencerStatus::paused){
			MIDIports::sendStop();
		}
		draw::drawPianoRoll(false);
		for(int i = 0; i<16; i++){
			trackArray[i].playAllNoteOffs(); //Reset each of the playing patterns to position zero
		}
		seqStatus = sequencerStatus::stopped;
		LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
		//endChaser();
	}
		
	std::vector<list::liElem*>* showPatternsInMemory(uint16_t* offset, uint16_t* previousPatternInList, const uint16_t limit){
		std::vector<list::liElem*>* names = new std::vector<list::liElem*>; //List elements with name and pattern number
		uint16_t counter = 0;
		//uint16_t counter2 = 0;
		uint16_t firstPatternFound = *offset;
		bool foundFirstPattern = false;
		for(uint16_t i = *offset; i<blocks::numberOfBlocks; i++){
			if(patMem::doesPatternExist(i)){
				counter++;
				if(!foundFirstPattern){//First pattern:
					foundFirstPattern = true;
					firstPatternFound = i;
					}
				char name[gc::maxPatternNameLength+1] = {0};
				patMem::pattern_t(i).getName(name);
				names->push_back(new list::liElemD(name, i));
			}
			if (counter >= limit){
				break;
			}
		}
		*previousPatternInList = 0; //For going up the list
		for(uint16_t i = *offset-1; i>=0; i--){
			if(patMem::doesPatternExist(i)){
				*previousPatternInList = i;
				break;
			}
		}
		*offset = firstPatternFound;
		return names;
	}//End showPatternsInMemory
	
	void numberToPatNum(const int num, char* string){
		int a =	num / 16;
		int b =	num % 16;
		sprintf(string, "%d", a);
		strcat(string, " ");
		char numb[3] = {0};
		sprintf(numb, "%d", b);
		strcat(string, numb);
	}
}
#endif