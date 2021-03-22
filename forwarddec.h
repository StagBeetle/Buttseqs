#ifndef forwarddec_h
#define forwarddec_h
#include <vector>
#include <functional>
#include <stdint.h>
#include "Arduino.h"

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
	
	const int numberOfMIDIIns = 4; //Includes USB
	const int numberOfMIDIOuts = 6; //Includes USB:
	
	const int numberOfEncoders = 4;
	
	//This is for the lists and is most likely applicable elsewhere
	const int W = 0;
	const int B = 1;
	static const uint8_t keyColours[12] = {W,B,W,B,W,W,B,W,B,W,B,W};// colours of the notes ascending
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
	class position;
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
	extern LEDSet& getLEDSet(buttons::keySet::ks);
	extern void showMode(LEDSet&);
	extern void showExtras(LEDSet&);
	extern void updateLEDs();
	extern void stepChaser(LEDSet&);
	extern void clearAll();
	extern void sendLEDs();
	extern void scheduleChange();
	
	extern void setActiveHue(const int newHue);
	extern const char* getActiveHue();
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
	extern void blankScreen();
	extern void setScreenLock(bool);
	}
namespace modes{
	class mode;
	//class modeField;
	// mode& getActiveMode()
	// void switchToMode(mode newMode, const bool fixed);
	// const char* getModeString();
	enum class focusedContext{
		none = 0	,
		error	,
		modal	,
		modalNum	,
		keyboardPiano	, //Has a name and pointer to function
		keyboardText	, 
		heldSetter	, 
	};
	
	const char* const focusedContextNames[] = {
		"none"	,
		"error"	,
		"modal"	,
		"modalNum"	,
		"keyboardPiano"	, 
		"keyboardText"	, 
		"heldSetter"	, 
	};
	
	extern void setDialog(const focusedContext type);//Has an error or a modal popped up
		//This dialogue just tells the mode not do do stuff with buttons
		//DRawing the actual dialogue box and options etc is done separately.
	extern void clearDialog();//Has an error or a modal popped up
}
namespace modal{
	extern void clearModalFunctionPointers();
}
namespace list{
	class liElem;
	class liElemD;
	template<class s0, class g0> class liElemF;
	class listController;
	class listControllerCard;
	class listControllerFixed;
	
	extern list::listControllerFixed editPatt;
	extern list::listControllerCard cardFiles;
	extern void setActiveList(listController*);
	enum settingsNames{
		editPriority = 0,
		advanceSteps,
		end,
		};
	 
	extern void doFunction(const modes::mode theMode, const uint8_t button);
	extern void drawList();
	extern void moveList(const modes::mode theMode, const int dir);
	extern void clearHeldSetter();
	extern void useHeldSetter(const int button);
	extern bool isHeldSetter();
	}
namespace Sequencing{
	class track;
	extern long long getTimeSinceLastSequence();
	std::vector<list::liElem*>* showPatternsInMemory(uint32_t& offset, uint32_t& previousPatternInList, const unsigned int limit);
	extern patMem::pattern_t getActivePattern();
	extern track& getTrack(const int trackNum);
	extern void setTempo(const double newTempo);
	const int maxTracks = 16;
	const int processesPerTrack = 16;
	extern void checkPlaying();
	
	}
namespace draw{
	extern void eventList();
	extern void drawPianoRoll(bool);
	extern void drawMode();
	extern void updateScreen();
	extern void drawPianoRollChaser();
	extern void drawPianoRollSteps();
	extern void editColoursVars();
	extern void editColours();
	extern void muteTracks();
	extern void muteNotes();
	extern void mute();
	extern void tempo();
	extern void arrangeTracks();
	extern void arrangeDetails();
	extern void arrangeChaser();
	extern void routingMatrix();
	extern void viewBar();
	extern void transpose();
	extern void modeList();
	extern void selection();
	extern void selectionVars();
	extern void selectionBorders();
	extern void process();
	extern void processSelection();
	extern void recordIcon();
	extern void keyboardButtons();
	extern void modeGrid(const int xOffset = 10, const int yOffset = 235);
	extern void buttons(const int x, const int y, const buttons::keySet::ks setToHighlight, const int scale = 1);
	namespace memoryUsageConstants{
		extern const int blockHeight; 
		extern const int blockWidth; 
		extern const int zoomedBlockWidth; 
	}
}
namespace card{
	const uint16_t maxDirectoryLength = 1024;
	extern std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns);
}
namespace interface{
	extern uint8_t editTrack;
	extern bool isShiftHeld;
	extern uint8_t keyboardOctave;
	extern bool isRecording();
	
	namespace record{
		extern void startNote(const int);
		extern void endNote(const int);
	}
	
	namespace settings{
		extern unsigned int buttonHoldDelay;
		extern unsigned int doubleClickSpeed;
		extern bool useSharpNotes;
		extern bool useFancyLEDChaser;
		extern bool pianoRollFollowPlay;
		extern bool recordOnlyFromExternal;
		extern bool givePlaceholderNameToNewPatterns;
		extern bool incrementNumberToDuplicatePatterns;
	}
	namespace pattUtils{
		//void changeLoadNumber(const int8_t num);
		extern char currentLoadPatternName[card::maxDirectoryLength+1];
	}
	
	namespace pattSwitch{
		extern uint32_t selectedPattern;
	}
	namespace arrange{
		extern const int numberOfBars;
		extern uint8_t activeArrangement;
		extern bool useArrangeMode;
	}
	namespace all{
		extern void keyboardOctShift(const int);
		extern void setTempo(const double);
		extern void exitError(const int);
		extern void keyboardTypePress(const int);
		extern void setKeyboardContext(std::function<void(const char*)> function, std::function<const char*()> currentName, const char* name, const int maxLength);
	}
	namespace modals{
		extern void doModalFunction(const int);
		extern void doModalNumFunction(const int);
	}
	namespace editStep{
		extern void setListPositionToLast(const bool redraw = false);
	}
}
namespace notifications{
	extern void displayError(uint8_t);
	extern void timeoutError(); 
}
namespace arrangement{
	extern const uint16_t arrangeBlockSize;
}
namespace functionDescriptions{
	extern void displayDescriptions();
}

extern std::function<void(void)> updateLEDs;
extern void strcatc(char*, char);

extern void startTimer();
extern void stopTimer();
extern void setSequencePriority(const int priority = 64);
#endif