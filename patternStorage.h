#pragma once

#include <bitset>
#include <functional>
#include "blocks.h"
#include <stdint.h>
#include <vector>
#include "forwarddec.h"
#include "dataEvents.h"
#include "Utility.h"

namespace patMem{
	extern const int addressSize;
	
	extern bool incrementNumberToDuplicatePatterns;
	extern bool givePlaceholderNameToNewPatterns;
	
struct length_t{//For pattern length
	
	private:
		uint8_t length	= 0; //Zero means 256
	public:
	//Constructors:
		length_t(const int p_length);
		length_t(bool);
	//Getters:
		int getLength() const;
		uint16_t getSubstepEquivalent() const;
};

enum class contentsType : uint8_t{//These are ordered, somewhow
	none	= 0,
	child	= 1, //Not saved in blocks - just used as a generic for note, CC and sysex
	note	= 2,
	cc	= 3,
	sysex	= 4,
	nrpn	= 5,
	exec	= 6,
	process	= 7,
	step	= 8, //Contains 16 children
	bar	= 9, //Contains 16 steps
	pattern	= 10, //Contains 16 bars
};

class hO{public: enum headerOffset{
	stepsPerBar	= 0,
	numberOfBars	= 1,
	patternLength	= 2,
	velocity	= 3,
	accentVelocity	= 4,
	transpose	= 5,
	shuffle	= 6,//4bit
	stretchToBar	= 7,//bool - Could possibly pack that uint8_t with the name somewhere and have 12 char names
	name	= 8,//16 chars 
	size	= 24,
};};

struct position{//(Also holds length of note or pattern) - that may not be true
	private:
		uint16_t value; // ----BBBBSSSSssss
		const static uint16_t flagsMask	= 4096 + 8192 + 16384 + 32768; //For include substep, perhaps?
		const static uint16_t barMask	= 256 + 512 + 1024 + 2048;
		const static uint16_t stepMask	= 16 + 32 + 64 + 128;
		const static uint16_t substepMask	= 1 + 2 + 4 + 8;
		const static uint16_t maxSize	= barMask + stepMask + substepMask;//4095
		const static uint16_t overLimit	= maxSize + 1;//4096  //used in 16 bar 16 step pattern //for wrapping
		// ----------------
		// ----Bar-StepSubs
	public:
		position(const uint8_t bar, const uint8_t step, const uint8_t substep = 0);

		position() = default;

		position(bool u);

		position(uint16_t num);

		uint8_t getBar()	const;
		uint8_t getStep()	const;
		uint8_t getSubstep()	const;

		void set(const uint8_t bar, const uint8_t step, const uint8_t substep=0);
		
		position offset(const int8_t bar, const int8_t step = 0, const int8_t substep=0);

		uint16_t getValue() const;

		void setBar(const uint8_t bar);

		void setStep(const uint8_t step);

		void setSubstep(const uint8_t substep);

		position& operator = (const position& other);

		position quantise(uint8_t level, length_t wrapper = {true});

		position shuffle(uint8_t level);

		void makeString(char* outArray, char spacer = ' ', uint8_t depth = 3) const;

		void print() const;

		contentsType getLargestTypeRequiredToHold() const ;

		bool cancontentsTypeHoldPos(contentsType type);

		friend position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper);
		friend position add(const position add1, const position add2, const length_t wrapper);
		friend bool operator == (const position& one, const position& other);
		friend bool operator <	(const position& one, const position& other);
		friend position operator -	(const position& one, const position& other);

};

position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper = {true});

position add(const position add1, const position add2, const length_t wrapper = {true});

bool operator ==	(const position& one, const position& other);
bool operator !=	(const position& one, const position& other);
bool operator <	(const position& one, const position& other);
bool operator >	(const position& one, const position& other);
bool operator <=	(const position& one, const position& other);
bool operator >= 	(const position& one, const position& other);
position operator - (const position &one, const position &other);

//The notes that are stored:
struct note{//Uses 5 bytes - 4 when saved
	private:
		uint8_t pitch = 0;
		uint8_t velocity = 0;
		position length; //In ticks - max 16*16*16
		uint8_t flags = 0;//  ... accent legato //Could potentially combine this using the free bit in pitch and velocity
	public:
		static const int accentFlag = 1;
		static const int legatoFlag  = 0;
		static const int substepLengthFlagOffset  = 4;// + 5 6 7
	//Constructors
		note(uint8_t p);

		note(uint8_t p, uint8_t v, position l);

		note(uint8_t p, uint8_t v, position l, uint8_t f);

		note(uint8_t p, uint8_t v, position l, bool a, bool leg);

		note() = default;

		note(const note &c_note);

		void setNote(uint8_t p, uint8_t v, position l);
			
		void print();

		void setPitch(int p);

		void setLength(position l);

		void setLength(bool isShiftHeld);

		position& getLength();

		void setLengthWithoutSubstep(position l);


		void setVelocity(int v);
		void setFlags(int f);
		bool isValid(); //Checks if the note is there or just blank
		//Use pitch value of 128
		void clear();
		uint8_t getPitch() const	;
		uint8_t getVelocity() const	;
		position getLength() const	;

		uint16_t getLengthInSubsteps() const;

		uint8_t getFlags() const	;
		bool getAccent() const	;
		bool getLegato() const	;
		const char* getAccentString() const ;
		const char* getLegatoString() const ;
		void toggleAccent();
		void toggleLegato();
		void setAccent(bool state);
		void setLegato(bool state);
		bool operator==(const note &other) const ;
	};
	
typedef std::vector<note> noteList;


struct notePos{//For screen display
	public:
		notePos(const note c_note, const position c_position);
		bool isValid();
		notePos() = default;
		note n;
		position p;
	};

struct noteOff{//Used for the scheduled note offs
	private:
		uint8_t pitch;
		int length; //In ticks
		uint8_t channelAndPort;
	public:
		uint8_t getPitch() const;
		uint8_t getChannel() const;
		uint8_t getPort() const;
		int getLength() const;
		noteOff();
		noteOff(const int c_pitch, const int channel, const int port);//For playing a noteOff straight away, so length does not matter
		void setNoteOff(uint8_t p, int l, uint8_t c, uint8_t port);
		void clearNoteOff();
		void decrementLength();
		void setLength(const int newLength);
		bool isValid();
	};


//------------------------------//------------------------------//------------------------------BLOCK DEFINITION

//Patterns:
extern const int patternHeaderSize;// = hO::size;
extern const int patternBlockSize;// = patternHeaderSize + addressSize; // 25 - The pattern heaeder data and the block

//------------------------------//------------------------------//------------------------------OTHER BLOCK FUNCTONS

//------------------------------------------------------------------------------------------PatternFunctions:

uint32_t getLastPatternNum();

//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
class contents_t;

class savedNote{
	private:
		uint8_t* notePointer = nullptr;
		static const int accentFlag = 1;
		static const int legatoFlag  = 0;
	public:
		savedNote(uint8_t* newPointer);

		savedNote() = default;

		bool isValid() const;

		void setVelocity(const uint8_t newVal);

		void setLengthWithoutSubstep (const position newPos);

		void setLength (const position newVal);

		void setLengthBars(const position val);

		void setLengthSteps(const position val);

		void setLengthSubsteps(const position val);

		void toggleAccent();

		void toggleLegato();

		note getNote() const ;
};

class pattern_t : public blocks::block_t{
	public:
	//Constructors
		pattern_t(bool u);

		pattern_t (uint32_t p_address);//Assign a pattern to object 
		
		pattern_t (const pattern_t& p);
		
		pattern_t operator=(const patMem::pattern_t& p) ;

		pattern_t();

		void set(hO::headerOffset offset, uint8_t value) ;

		uint8_t get(hO::headerOffset offset) const ;

		void destroy() ;
	//Getters
		length_t getLength() const ;
		int getLengthInSteps() const;
		int getStepsPerBar() const /*volatile*/; //1 - 16
		int getNumberOfBars() const /*volatile*/;
		contents_t getFirstBlock() const ;
		int8_t getVelocity() const ;
		int8_t getAccentVelocity() const ;
		uint8_t getShuffle() const ;
		bool isStretchToBar() const ;
		int8_t getTranspose() const ;
		//Setters
		void clearData() ;
		void clearNotes() ;
		void clearAll();
		void setFirstBlock(contents_t blockToSet) ;
		void clearFirstBlock()  ;
		
		void setLength(int length) /*volatile*/;
		void setStepsPerBar(int stepsPerBar) /*volatile*/ ;
		void setNumberOfBars(int numberOfBars) /*volatile*/;
		
		void setVelocity(int8_t value) ;
		void setAccentVelocity(int8_t value)  ;
		void setShuffle(int8_t value)  ;
		void toggleStretchToBar() ;
		void setStretchToBar(const bool stretch) ;
		void setTranspose(int value) ;
		
		
		//Name:
		void setName(const char* newName);
		void getName(char* name) const ;
		const char* getName() const;

		//Complex
		contents_t getChild (position pos, contentsType typeToFind) const ;

		contents_t createChild (position pos, contentsType typeToMake) ;

		void checkContentsAndPotentiallyDestroyBlock(position pos, contents_t noteBlock) ;

		void addOrRemoveNote(note noteToAdd, position pos)  ;

		void removeNote(note noteToRemove, position pos)  ;

		void addOrUpdateNote(note noteToAdd, position pos)  ;

		void clearStepAndSubstepOfNotes(position pos)  ;

		void clearStepOfNotes(position pos) ;

		noteList getNotesOnStep(position pos)  const;

		void clearStepAndMaybeEraseParent(position pos) ;

		std::vector<uint8_t> getPitchesOnStep(position pos, const bool searchSubstepInsteadOfStep, const bool searchSubstepsWithinStep)  ;

		//For LEDs - Given a (bar or step) and a note, provide what steps or substeps contain that note
		std::vector<uint8_t> getStepsFromNote(const note pitchToFind, const uint8_t bar, const int8_t step/*-1 for search within bar*/, const bool searchSubstepsToo)  ;

		savedNote getNoteOnStep(const position pos, const note noteToFind) ;
		
		void copyStep(pattern_t sourcePattern, const position pos, const position destinationPos, const bool clear)  ;

		void copyFromPattern(const pattern_t sourcePattern)  ;

		void quantiseStep(const position pos, const uint8_t quantiseLevel, const bool wrapQuantise)  ;

			//Given a step or substep, see which notes are on it
		std::array<bool, gc::keyNum::notes> getNoteLeds(const uint8_t startPitch, const position pos, const bool isInSubstep, const bool searchSubstepsInStep)  ;
		
		//Given a note, see on which steps or substeps it appears
		std::array<bool, gc::keyNum::steps> getStepLeds(const uint8_t pitch, const int bar, const int step, const bool searchSubstepToo)  ;

		void copyHeader(pattern_t otherPatt)  ;

		//Saving and loading
		void saveToCard();
		
};

//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
bool doesPatternExist(pattern_t patt);
bool doesPatternExist(uint32_t patt);

void forEachPattern(std::function<void(pattern_t&)> func);

}//End namespace
