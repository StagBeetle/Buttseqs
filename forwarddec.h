#ifndef forwarddec_h
#define forwarddec_h
#include <vector>
#include <functional>
#include <stdint.h>

//These are things that are referenced throught the code and stuff that I have had to forward declare because of my lack of splitting .cpp and .h files, which I still think is rubbish.

namespace gc{//global constants
	struct keyNum{//How many of each key there are
		static const int steps	= 16;
		static const int notes	= 25;
		static const int vertical	= 2;
		static const int horizontal	= 2;
		static const int datas	= 16;
		static const int modes	= 16;
		static const int extras	= 4;
		static const int total	= steps + notes + vertical + horizontal + datas + modes + extras;
	};
	
	struct keyPos{//Where the keys are in the shift loop.
		static const int steps	= 0;
		static const int notes	= steps	+	keyNum::steps;
		static const int vertical	= notes 	+	keyNum::notes;
		static const int horizontal	= vertical 	+	keyNum::vertical;
		static const int datas	= horizontal	+	keyNum::horizontal;
		static const int modes	= datas	+	keyNum::datas;
		static const int extras	= modes	+	keyNum::modes;
		static const int max	= extras	+	keyNum::extras;
	};
	
	const uint8_t numberOfCharacters = 55;
	const char chars[numberOfCharacters] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9',' ','!','#','$','%','&','\'','(',')','-','@','^','_','`','{','}','~','<','s'};
	
	const uint8_t maxPatternNameLength = 16;
	
	const uint16_t numberOfSubsteps = 4096;
	const int numberOfTracks = 16;
	
	const int numberOfMIDIIns = 5;
	const int numberOfMIDIOuts = 5;
	
	const int numberOfEncoders = 4;
};

namespace patMem{
		extern const int patternDataSize;
		extern const int patternHeaderSize;
		extern const int stepBlockSize;
		extern const int barPointerBlockSize;
		extern const int patternBlockSize;
		extern const int contentsBlockSize;
		uint32_t getLastPatternNum();
		class notePos;
		class pattern_t;
		}
namespace buttons{
	class buttonEvent {public: enum be : uint8_t{
		press	= 0,
		release	= 1,
		hold	= 2,
		max	= 3,
	};};
	class keySet {public: enum ks : uint8_t{
		step	= 0,
		note	= 1,
		vertical	= 2,
		horizontal	= 3,
		data	= 4,
		max	= 5, //max different in each mode
		mode	= 6,
		extra	= 7,
		none	= 8,
	};};
	const char* const buttonEventNames [] = {
		"press"	,
		"release"	,
		"hold"	,
		"max"	,
	};
	const char* const keySetNames [] = {
		"step"	,
		"note"	,
		"vertical"	,
		"horizontal"	,
		"data"	,
		"max"	, //max different in each mode
		"mode"	,
		"extra"	,
		"none"	,
	};
	void loop(const bool);
	//void checkButtonsIfTime();
	}
namespace LEDfeedback{
	class LEDSet;
	LEDSet& getLEDSet(buttons::keySet::ks);
	void showMode(LEDSet&);
	void showExtras(LEDSet&);
	void updateLEDs();
	void stepChaser(LEDSet&);
	void clearAll();
}
namespace scrn{
	// extern const int width;
	// extern const int height;
	class displayType;
	// namespace colour{
		// class td{public: enum themeDescriptor{
			// bg   = 0,
			// acc1 = 1,
			// acc2 = 2,
			// fg   = 3
		// };};
		// uint16_t getThemeColour(td::themeDescriptor index);
		// }
	void blankScreen();
	void setScreenLock(bool);
	}
namespace modes{
	class mode;
	//class modeField;
	// mode& getActiveMode()
	// void switchToMode(mode newMode, const bool fixed);
	// const char* getModeString();
	}
namespace modal{
	void clearModalFunctionPointers();
}
namespace list{
	class liElem;
	class liElemD;
	template<class s0, class g0> class liElemF;
	class listController;
	class listControllerFixed;
	
	extern list::listControllerFixed editPatt;
	void setActiveList(listController*);
	enum settingsNames{
		editPriority = 0,
		advanceSteps,
		end,
		};
	 
	void doFunction(const modes::mode theMode, const uint8_t button);
	void drawList();
	void moveList(const modes::mode theMode, const int dir);
	
	//void setHeldSetter(liElem*);
	void clearHeldSetter();
	void useHeldSetter(const int button);
	bool isHeldSetter();
	}
namespace Sequencing{
	class track;
	extern long long getTimeSinceLastSequence();
	std::vector<list::liElem*>* showPatternsInMemory(uint16_t* offset, uint16_t* previousPatternInList, const uint16_t limit);
	patMem::pattern_t getActivePattern();
	volatile track& getTrack(const int trackNum);
	extern void setTempo(const double newTempo);
	const int maxTracks = 16;
	const int processesPerTrack = 16;
	extern void checkPlaying();
	
	}
namespace draw{
	void eventList();
	void drawPianoRoll(bool);
	void drawMode();
	void updateScreen();
	void drawPianoRollChaser();
	void drawPianoRollSteps();
	void editColoursVars();
	void editColours();
	void muteTracks();
	void muteNotes();
	void mute();
	void tempo();
	void arrangeTracks();
	void arrangeDetails();
	void arrangeChaser();
	void routingMatrix();
	void viewBar();
	void transpose();
	void modeList();
	void selection();
	void selectionVars();
	void selectionBorders();
	void process();
	void processSelection();
	void keyIcon(const int x, const int y, const buttons::keySet::ks k);
	namespace memoryUsageConstants{
		extern const int blockHeight; 
		extern const int blockWidth; 
		extern const int zoomedBlockWidth; 
	}
}
namespace card{
	const extern uint16_t maxDirectoryLength = 256;
	extern std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns);
}
namespace interface{
	extern uint8_t editTrack;
	extern bool isShiftHeld;
	extern uint8_t keyboardOctave;
	extern bool isRecording();
	
	namespace record{
		extern const int maxRecordingNotes = 16;
		extern patMem::notePos noteBuffer[maxRecordingNotes];
		extern void startNote(const int);
		extern void endNote(const int);
	}
	
	namespace settings{
		unsigned int buttonHoldDelay = 250;
		extern bool useSharpNotes;
		extern bool useFancyLEDChaser;
		extern bool pianoRollFollowPlay;
		extern bool recordOnlyFromExternal;
	}
	namespace pattUtils{
		//void changeLoadNumber(const int8_t num);
		char currentLoadPatternName[card::maxDirectoryLength+1] = {0};
	}
	
	namespace pattSwitch{
		uint16_t selectedPattern = 0;
	}
	namespace arrange{
		extern const int numberOfBars;
		extern uint8_t activeArrangement;
		extern volatile bool useArrangeMode;
	}
	namespace all{
		extern void keyboardOctShift(const int);
		extern void setTempo(const double);
	}
	namespace editStep{
		extern void setListPositionToLast(const bool redraw = false);
	}
}
namespace notifications{
	void displayError(uint8_t);
	void timeoutError(); 
}
namespace arrangement{
	extern const uint16_t arrangeBlockSize;
}
namespace functionDescriptions{
	extern void displayDescriptions();
}
// namespace audio{
	// class audioUnit;
// }

// void lg(const char* data = "");
// void lg(char* data);
// template<typename T>void lg(const T data);
// void lg(const double data);
	
// void lgc(const char* data = "");
// template<typename T>void lgc(const T data);
// void lgc(const double data);




//template <typename T = const char*> void lg(const T contents = ""){Serial.println(contents);}
//template <typename T = const char*> void lgc(const T contents = ""){Serial.print(contents);}

//Logging:
template <typename T>
void printDebug(const char* file , int line, T message = "", bool newLine = true) {
	char* fileName = strrchr(file, '/');
	Serial.print(fileName ? fileName : file);
	Serial.print(line);
	Serial.print(":");
	Serial.print(message);
	if(newLine){
		Serial.println();
	}
}

//Logging (to Serial) with or without newLine:
#define lg(message) (printDebug(__FILE__, __LINE__, (message)))
#define lgc(message) (printDebug(__FILE__, __LINE__, (message), false))

std::function<void(void)> updateLEDs;
void strcatc(char*, char);

IntervalTimer sequence;
inline void startTimer(){sequence.begin(Sequencing::checkPlaying, 500);}
inline void stopTimer(){sequence.end();}

#endif