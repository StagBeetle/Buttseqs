//Storing the note data and exporting it
#ifndef Sequencing_h
#define Sequencing_h
#include <vector> 
#include "Utility.h"
#include "forwarddec.h" //Volatile
#include "bitsetlite.h"
#include "processes.h" 
#include "list.h" 
#include "MIDIInputOutput.h"

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
	extern uint8_t legatoOffset; //The number of ticks after a note has played to schedule a noteoff for the legato note
	extern uint8_t midiSyncExternal;
	extern bool adjustNoteLengthByShuffle;
	const uint8_t nameLength = 16;
	extern const int maxRecordingNotes;
	extern int8_t globalTranspose;
	//bool isSeqPlaying = false;
	patMem::notePos& getRecordingNote(const int note);
	int16_t loadPatternToMemory(const char* patternName);
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
	//Some are volatile beacuse they are changed from within interrupt
	class track{
		private:
			static const int bankSlots = 16;
			static const unsigned int maxTick = 16 * maxBarsPerPattern * PPS; //12288;
			int8_t transpose = 0;
			const uint8_t trackNum;
			uint8_t channel = 1; //1-16
			MIDIports::MIDIPort midiPort = MIDIports::MIDIPort::usb;
			bool isMuted = false;
			bitset_lite<bankSlots> usedBankSlots;
			// uint8_t 
			volatile patMem::pattern_t activePattern;
			 patMem::pattern_t trackBank[bankSlots];
			volatile int8_t currentSubstep = -1;
			volatile int16_t currentStep = -1;
			volatile noteOff scheduledNoteOffs[noteOffPolyphony]; //Notes that end on each step
			 trackType m_trackType = trackType::poly;
			volatile uint8_t realShuffle = 0;
			volatile uint8_t realPatternLength = 0; //The actual length of the pattern
			volatile uint8_t realStepsPerBar = 0; //The actual length of the pattern
			//uint8_t realBarsPerPattern = 0; //The actual length in bars of the pattern
			volatile bool realStretchToBar = false;
			volatile note legatoNote;
			volatile uint8_t legatoNoteChannel; //In the possible situation channel is changed midway through legato note
			 MIDIports::MIDIPort legatoNotePort = MIDIports::MIDIPort::none;
			 bitset_lite<128> mutedNotes;
			 process::processStack pStack;
			 uint8_t soundSourceIndex = 0;
			volatile bool temporaryMute = false; //Mute when new notes are being played when recording.
			char name[nameLength+1] = {0};
		public:
			static uint8_t channelInc;
			track();
			const char* getName() ;
			void setName(const char*) ;
			void setSoundEngine(const int index) ;
			const char* getSoundEngineName() const ;
			void playNoteOn(const note theNote)  const;
			void playNoteOff(const noteOff theNote)  const;
			void playNoteOff(const uint8_t theNote)  const;
			//Mute
			void toggleMute() ;
			void mute() ;
			void unmute()  ;
			bool getMuted()  const;
			void setTemporaryMute()  ; //For live playing mute
			//Pattern stuff
			bool hasActivePattern() const;
			patMem::pattern_t getActivePattern() const  ;
			void removePattern()  ;
			void switchPattern(const patMem::pattern_t newPattern) ;
			patMem::pattern_t newPattern()  ;
			void addOrRemoveNote(const note noteToAdd, const position pos, const bool isMonoPatt)  ;
			void addOrUpdateNote(const note noteToAdd, const position pos, const bool isMonoPatt)  ;
			void removeNote(const uint8_t pitch, const position pos)  ;
			//Other stuff
			void setTranspose(int8_t transposeAmount)  ;
			bool doesTrackBankHavePattern(const int bankSlotNum)  const;
			patMem::pattern_t getTrackBankPattern(const int bankSlotNum)  const;
			void addPatternToBank(const patMem::pattern_t newPattern)  ;
			void removePatternFromBank(const int bankSlotNum)  ;
			void removePatternFromBankIfExists(patMem::pattern_t patt)  ;
			bool areTrackBankSlotsAvailable()  const;
			void removePatternFromTrack()  ;
			void removePatternFromTrackIfActive(const uint32_t patternNum)  ;
			void assignPatternToTrack(uint32_t newPattern)  ;
			void advanceStep()  ;
			void resetPattern()  ;//Set the pattern play point to -1 so next tick it is zero
			bool setShuffleFromPattern()  ;
			void setTrackVarsFromPattern()  ;//Runs every tick
			bool setStretchToBar()  ;
			int getShuffleOffset()  ;//Looks at the current step and the shuffle amount		
			void playNotesOnStep() ;
			void doTick()  ;//TIMER FUNCTION
			uint8_t getCurrentStep() const ;
			uint8_t getCurrentBar() const ;
			uint8_t getStepsPerBar() const ;
			noteList getNotesOnStep(const patMem::position pos, const bool applyProcess = true)  ;//SHow process determines whether to use processed or original
			noteList getNotesOnCurrentStep()  ;
			signed int addNoteOffToScheduled (const note theNote, const uint8_t theChannel, const MIDIports::MIDIPort thePort)  ;
			void checkForNoteOffs()  ;	
			void endLegatoNote()  ;
			void playAllNoteOffs()  ;
			void setMIDIChannel(const int button)  ;
			void setMIDIPort(const int button)  ;
			const char* getMIDIportForUser()  const;
			uint8_t getMIDIPortNumber() const  ;
			uint8_t getMIDIchannel() const  ;
			void setTrackMode(const int setter)  ;
			trackType getTrackMode()  const;
			const char* getTrackModeString()  const;
			bool isStretchToBar()  const ;
			uint8_t getShuffle()  const ;
			void toggleMutedNote(int note)  ;
			bool isNoteMuted(int note)  const ;
			patMem::position getPlayPosition()  const;
			patMem::length_t getLength()  const;
			void addProcess(const int p, const int processID)  ;
			void removeProcess(const int p)  ;
			void reorderProcesses(const int from, const int to)  ;
			process::process_t getProcess(const int pos)  ;
			int getNumProcesses() const  ;
	};
	//End of track class
	
	patMem::pattern_t getPattern(const int trackNum);
	track& getTrack(const int trackNum);
	track& getActiveTrack();
	patMem::pattern_t getActivePattern();
	
	// /==
	// |=| equencing:
	// ==/
	enum class sequencerStatus : uint8_t {
		stopped = 0	,
		playing	,
		paused	,
	};
	const char* const seqSta[] = {
		"stopped",
		"playing",
		"paused",
	};
	
	sequencerStatus getSequencerStatus();
	bool isSeqPlaying();
	void setTempo(const double newTempo);
	double getTempo();
	void setSyncOnBarChange();
	void begin();
	void update();
	void checkPlaying();
	void beginChaser();
	void startSequencer();
	void continueSequencer();
	void pauseSequencer();
	void stopSequencer();
	std::vector<list::liElem*>* showPatternsInMemory(uint32_t* offset, uint32_t* previousPatternInList, const uint16_t limit);
	void numberToPatNum(const int num, char* string);
	//Loop functions
	void setLoopStart(const int start);
	void setLoopEnd(const int end);
	int getLoopStart();
	int getLoopEnd();
	int getPlayingBar();
	double getLengthThroughBar();
	bool isSequencerSlow();
}
#endif