#ifndef pattMem_h
#define pattMem_h
#include <bitset>
#include <functional>
//#include "chipmemory.h"
//#include "comms.h"

namespace patMem{

bool incrementNumberToDuplicatePatterns = true;
const int shuffleMax = 15;

class eO{public:
	enum noteElementOffset{
		velocity	= 0,
		pitch	= 1,
		length	= 2,
		flags	= 3,
		max	= 4,
	};
};

enum class contentsType : uint8_t{//These are ordered
	none	= 0,
	child	= 1, //Not saved in blocks - just used as a generic for note, CC and sysex
	note	= 2,
	cc	= 3,
	sysex	= 4,
	step	= 5, //Contains 16 children
	bar	= 6, //Contains 16 steps
	pattern	= 7, //Contains 16 bars
};

const char* const contentsTypeNames[] = {
	"none"	,
	"child"	,
	"note"	,
	"cc"	,
	"sysex"	,
	"step"	,
	"bar"	,
	"pattern"	,
};

void printcontentsType(const contentsType toPrint){
	lg(contentsTypeNames[static_cast<int>(toPrint)]);
}

bool isChildType(contentsType type){//Find if the container is for cc, notes or sysex
	if(	type == contentsType::child	||
		type == contentsType::note	||
		type == contentsType::cc	||
		type == contentsType::sysex	){
		return true;
	} else {
		return false;
	}
}

struct length_t{//For pattern length
	
	private:
		uint8_t length	= 0; //Zero means 256
		//uint8_t substep	= 0;
	public:
	//Constructors:
		length_t(const int p_length) : 
			length(p_length == 256 ? 0 : p_length)
			
		{}
		length_t(bool) : 
			length (0)
		{}
	//Getters:
		// uint8_t getSubstep()	const {return substep	;}
		int getLength() const{
			return length == 0 ? 256 : length;
		}
		uint16_t getSubstepEquivalent() const{
			return getLength() * 16;
		}
};

struct position{//(Also holds length of note or pattern) - that may not be true
	private:
		uint16_t value; // ----BBBBSSSSssss
		const static uint16_t barMask	= 256 + 512 + 1024 + 2048;
		const static uint16_t stepMask	= 16 + 32 + 64 + 128;
		const static uint16_t substepMask	= 1 + 2 + 4 + 8;
		const static uint16_t maxSize	= barMask + stepMask + substepMask;//4095
		const static uint16_t overLimit	= maxSize + 1;//4096  //used in 16 bar 16 step pattern //for wrapping
		// ----------------
		// ----Bar-StepSubs
	public:
		position(const uint8_t bar, const uint8_t step, const uint8_t substep = 0){
			if(bar >=16){
				lg("bar error");
			}
			if(step >= 16){
				lg("step error");
			}
			if(substep >= 16){
				lg("substep error");
			}
			set(bar, step, substep);
		}

		position() = default;

		position(bool u){
			//Overlimit is just for wrapping
			value = u ? overLimit : maxSize;
		}

		position(uint16_t num){
			value = num;
		}

		uint8_t getBar()	const {return (value >> 8)	;}
		uint8_t getStep()	const {return (value >> 4) % 16	;}
		uint8_t getSubstep()	const {return (value >> 0) % 16	;}

		void set(const uint8_t bar, const uint8_t step, const uint8_t substep=0) {
			value = (bar<<8) + (step<<4) + substep;
		}
		
		position offset(const int8_t bar, const int8_t step = 0, const int8_t substep=0) {
			value += (bar*256);
			value += (step*16);
			value += (substep);
			value += overLimit;
			value %= overLimit;
			return *this;
		}

		uint16_t getValue() const{
			return value;
		}

		void setBar(const uint8_t bar){
			value = (value & (stepMask + substepMask)) + (bar<<8);
		}

		void setStep(const uint8_t step){
			value = (value & (barMask + substepMask)) + (step<<4);
		}

		void setSubstep(const uint8_t substep){
			value = (value & (barMask + stepMask)) + (substep);
		}

		position& operator = (const position& other){
			value = other.value;
			return *this;
		}

		position quantise(uint8_t level, length_t wrapper = {true}){
			int nearestSubstep  = 256 / level;
			int modValue = value % nearestSubstep; //Plus one so a halfway note rounds up - e.g. 16th notes would quantise to next 8th note rather than last
			value = (modValue+1 > (nearestSubstep / 2)) ? value + (nearestSubstep - modValue) : value - modValue;
			// lg(wrapper.getSubstepEquivalent());
			value %= wrapper.getSubstepEquivalent();
			// print();
			// lg();
			return {value};
		}

		position shuffle(uint8_t level){
			const int sPS = 16;
			bool isStepOdd = getStep() % 2;
			int positionInTwoSteps = isStepOdd * sPS + getSubstep();
			int lastEvenStep = getStep() - isStepOdd;
			int shufflePoint = sPS + level;
			if(positionInTwoSteps < shufflePoint){
				int positionInStep = (positionInTwoSteps * sPS) / shufflePoint;
				setStep(lastEvenStep);
				setSubstep(positionInStep);
			} else {
				int positionInStep = ((positionInTwoSteps - shufflePoint) * sPS) / (sPS * 2 - shufflePoint);
				setSubstep(positionInStep);
			}
			return {value};
		}

		void makeString(char* outArray, char spacer = ' ', uint8_t depth = 3) const{
			char numberHolder[3] = {0};
			sprintf(numberHolder, "%d", getBar());
			strcat(outArray, numberHolder);
			if(depth == 1){return;}
			strcatc(outArray, spacer);
			sprintf(numberHolder, "%d", getStep());
			strcat(outArray, numberHolder);
			if(depth == 2){return;}
			strcatc(outArray, spacer);
			sprintf(numberHolder, "%d", getSubstep());
			strcat(outArray, numberHolder);
		}

		void print() const{
			char str[9] = {0};
			makeString(str, ':');
			lg(str);
		}

		contentsType getLargestTypeRequiredToHold() const {
			return
				getBar()	> 0 ? contentsType::pattern	:
				getStep()	> 0 ? contentsType::bar	:
				getSubstep()	> 0 ? contentsType::step	:
				contentsType::child;
		}

		bool cancontentsTypeHoldPos(contentsType type){
			return static_cast<int>(type) >= static_cast<int>(getLargestTypeRequiredToHold());
		}

		friend position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper);
		friend position add(const position add1, const position add2, const length_t wrapper);
		friend bool operator == (const position& one, const position& other);
		friend bool operator <	(const position& one, const position& other);
		friend position operator -	(const position& one, const position& other);

};

position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper = {true}){//For lengths
	// lg(subtractFrom.getValue());
	// lg(subtractThis.getValue());
	int32_t answer = subtractFrom.getValue() - subtractThis.getValue();
	answer = (answer + position::overLimit) % wrapper.getSubstepEquivalent();
	// lg(wrapper.getSubstepEquivalent());
	// lg(answer);
	// lg();
	return {static_cast<uint16_t>(answer)};
}

position add(const position add1, const position add2, const length_t wrapper = {true}){
	int32_t answer = add1.getValue() + add2.getValue();
	answer = (answer + position::overLimit) % wrapper.getSubstepEquivalent();
	return {static_cast<uint16_t>(answer)};
}

bool operator == (const position& one, const position& other){
	return	one.value == other.value;
}

bool operator !=	(const position& one, const position& other){return !(one == other);	}

bool operator <	(const position& one, const position& other){return one.value < other.value;	}
bool operator >	(const position& one, const position& other){return other < one;	}
bool operator <=	(const position& one, const position& other){return !(one > other);	}
bool operator >= 	(const position& one, const position& other){return !(one < other);	}

position operator - (const position &one, const position &other){
	return position(static_cast<uint16_t>(one.value - other.value));
}

// void forPositionRangeSteps(position first, const position last, std::function<void(position)> funcy, const length_t wrapper = {true}){
	// //Increment first and wrap until it reaches alst
	// while(first <= last){
		// funcy(first);
		// if(first == position{15,15,15} && last == position{15,15,15}){return;}//Else it will loop round
		// first.incrementStep(wrapper);
	// }
// }

// void forPositionRangeSubsteps(position first, const position last, std::function<void(position, bool&)> funcy){
	// //Increment first and wrap until it reaches alst
	// bool earlyExit = false;
	// while(first <= last){
		// funcy(first, earlyExit);
		// if(earlyExit){return;}
		// if(first == position{15,15,15} && last == position{15,15,15}){return;}//Else it will loop round
		// first.incrementSubstep();
	// }
// }

//The notes that are stored:
struct note{//Uses 5 bytes - 4 when saved
	private:
		byte pitch = 0;
		byte velocity = 0;
		position length; //In ticks - max 16*16*16
		byte flags = 0;//  ... accent legato //Could potentially combine this using the free bit in pitch and velocity
	public:
		static const int accentFlag = 1;
		static const int legatoFlag  = 0;
		static const int substepLengthFlagOffset  = 4;// + 5 6 7
	//Constructors
		note(uint8_t p){//When only a pitch is needed
			setPitch(p);
		}

		note(uint8_t p, uint8_t v, position l){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			//setSubstepLength(15);
			}

		note(uint8_t p, uint8_t v, position l, uint8_t f){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			setFlags(f);
			//setPosition(o);
			}

		note(uint8_t p, uint8_t v, position l, bool a, bool leg){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			setAccent(a);
			setLegato(leg);
			}

		note() = default;

		note(const note &c_note):
			pitch(c_note.getPitch()),
			velocity(c_note.getVelocity()),
			length(c_note.getLength()),
			flags(c_note.getFlags())
			{}

		void setNote(uint8_t p, uint8_t v, position l){
			pitch = constrain(p, 0, 127);
			velocity = constrain(v, 1, 127);
			length = l;
			}
			
		void print(){
			lgc("p");
			lgc(pitch);
			lgc(" v:");
			lgc(velocity);
			lgc(" l:");
			lgc(length.getValue());
			lgc(" a:");
			lgc(getAccent());
			lgc(" g:");
			lg(getLegato());
		}
		//Multiples of 16 set the bar part of length. 0-15 sets the step part
		//void modLength(int l){length = (l >= 16) : ((l-1) + length % 16) ? ((length/16)*16 + l);}

		void setPitch(int p){
			pitch = constrain(p, 0, 127);
		}

		void setLength(position l){
			length = l;
		}

		void setLength(bool isShiftHeld){//For recording notes
			length = isShiftHeld ? patMem::position{0,0,0} :  patMem::position{0,0,1};
		}

		position& getLength(){
			return length;
		}

		void setLengthWithoutSubstep(position l){
			int substep = length.getSubstep();
			length = l;
			length.setSubstep(substep);
		}

		// void setSubstepLength(int l){
			// length = constrain(l,0,15);
		// }

		void setVelocity(int v){velocity = constrain(v, 1, 127);}
		void setFlags(int f){flags = f;}
		//void setPosition(int o){position = o;}
		bool isValid(){return (velocity == 0) ? false : true;} //Checks if the note is there or just blank
		//Use pitch value of 128
		void clear(){velocity = 0;}
		byte getPitch() const {return pitch;}
		byte getVelocity() const {return velocity;}
		position getLength() const {return length;}

		uint16_t getLengthInSubsteps() const {return length.getValue();}

		byte getFlags() const {return flags;}
		//byte getPosition(){return position;}
		bool getAccent() const {return bitRead(flags,accentFlag);}
		bool getLegato() const {return bitRead(flags,legatoFlag);}
		const char* getAccentString() const {
			return bitRead(flags,accentFlag) ? "A" : "_";
			}
		const char* getLegatoString() const {
			return bitRead(flags,legatoFlag) ? "L" : "_";
			}
		void toggleAccent(){bitWrite(flags,accentFlag,!getAccent());}
		void toggleLegato(){bitWrite(flags,legatoFlag,!getLegato ());}
		void setAccent(bool state){bitWrite(flags,accentFlag,state);}
		void setLegato(bool state){bitWrite(flags,legatoFlag ,state);}
		bool operator==(const note &other) const {
			if(
			(pitch == other.pitch)       &&
			(velocity == other.velocity) &&
			(length == other.length)     &&
			(flags == other.flags)       ){
				return true;
			} else {
				return false;
			}
		}
	};
	
typedef std::vector<note> noteList;

struct notePos{//For screen display
	public:
		notePos(const note c_note, const position c_position):
			n(c_note),
			p(c_position)
			{}
		bool isValid(){
			return n.isValid();
			}
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
		byte getPitch() const
			{return pitch;}
		byte getChannel() const
			{return (channelAndPort & 15)+1;}
		byte getPort() const
			{return (channelAndPort >> 4) & 15;}
		int getLength() const
			{return length;}
		noteOff()
			{channelAndPort = 0; pitch = 128; length = 0;}
		noteOff(const int c_pitch, const int channel, const int port) ://For playing a noteOff straight away, so length does not matter
			pitch(c_pitch),
			length(0),
			channelAndPort(channel-1 + (port << 4))
			{}
		void setNoteOff(uint8_t p, int l, uint8_t c, uint8_t port)
			{pitch = constrain(p,0,127); length = l; channelAndPort = (port << 4) + (c-1);}
		void clearNoteOff()
			{pitch = 128;}
		void decrementLength()
			{length -- ;}
		void setLength(const int newLength){
			length = newLength;
			}
		bool isValid()
			{return (pitch != 128);} //Checks if the note is there or just blank
	};

struct cc{
	uint8_t number;
	uint8_t value;
	};

enum class playMode{
	forward,
	reverse,
	bounce,
	random
	};

class hO{public: enum headerOffset{
	stepsPerBar	= 0,//4bit
	patternLength	= 1,
	velocity	= 2,
	accentVelocity	= 3,
	transpose	= 4,
	shuffle	= 5,//4bit
	stretchToBar	= 6,//bool - Could possibly pack that byte with the name somewhere and have 12 char names
	name	= 7,//16 chars 
	size	= 23,
};};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- Memory allocation -------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//For control:
//uint16_t patternsInMemory = 0;
const uint8_t falseZero = 127;//In some unsigned bytes, this represents zero.

bool givePlaceholderNameToNewPatterns = true;

// void setup(){
	// usedPatterns.set(maxNumberOfPatternsInMemory);
// }

uint16_t makeAddress(uint8_t MSB, uint8_t LSB){
	return (MSB << 8) + LSB;
}
uint16_t makeAddress(uint8_t* pointer){
	uint8_t MSB = *pointer;
	uint8_t LSB = *(pointer+1);
	return makeAddress(MSB, LSB);
}

void writeAddress(uint8_t* pointer, uint16_t address){
	uint8_t MSB = address >> 8;
	uint8_t LSB = address % 256;
	*pointer = MSB;
	*(pointer +1) = LSB;
}

inline void writeNoteData(uint8_t* pointerToElement, const int positionOfDataWithinElement, const uint8_t dataToWrite){
	//Set note variables from a pointer
	*(pointerToElement + positionOfDataWithinElement) = dataToWrite;
}

inline uint8_t readNoteData(const uint8_t* pointerToElement, const int positionOfDataWithinElement){
	//Get notes variables from a pointer
	return *(pointerToElement + positionOfDataWithinElement);
}

inline note readNote(const uint8_t* pointerToElement){
	uint8_t v = readNoteData(pointerToElement, eO::velocity	);
	if(v == 0){//Return invalid note
		return {};
	}
	uint8_t p = readNoteData(pointerToElement, eO::pitch	);
	uint8_t f = readNoteData(pointerToElement, eO::flags	);
	uint8_t l = readNoteData(pointerToElement, eO::length	);
	position length = {
		static_cast<uint8_t>(l >> 4),
		static_cast<uint8_t>(l & B00001111),
		static_cast<uint8_t>(f >> 4)
		};
	f &= B00001111;
	note returnNote = {p, v, length, f};
	return returnNote;
}

class savedNote{
	private:
		uint8_t* notePointer = nullptr;
		static const int accentFlag = 1;
		static const int legatoFlag  = 0;
	public:
		savedNote(uint8_t* newPointer){
			notePointer = newPointer;
			}

		savedNote() = default;

		inline bool isValid() const{
			return (notePointer != nullptr);
			}

		void setVelocity(const uint8_t newVal){
			if (isValid()){
				*(notePointer + eO::velocity) = constrain(newVal,1,127);
			}
		}

		void setLengthWithoutSubstep (const position newPos){//Readjust for substep
			if (isValid()){
				*(notePointer + eO::length) = newPos.getValue() >> 4;
			}
		}

		void setLength (const position newVal){//Readjust for substep
			if (isValid()){
				uint16_t lengthInSubsteps = newVal.getValue();
				*(notePointer + eO::length) = lengthInSubsteps >> 4;
				uint8_t flags = *(notePointer + eO::flags) & B00000011;
				*(notePointer + eO::flags) = flags + ((lengthInSubsteps % 16) << 4);
			}
		}

		void setLengthBars(const position val){
			if (isValid()){
				uint8_t oldLength = *(notePointer + eO::length);
				*(notePointer + eO::length) = (oldLength & B00001111) + (val.getBar() << 4);
			}
		}

		void setLengthSteps(const position val){
			if (isValid()){
				uint8_t oldLength = *(notePointer + eO::length);
				*(notePointer + eO::length) = (oldLength & B11110000) + val.getStep();
			}
		}

		void setLengthSubsteps(const position val){
			if (isValid()){
				uint8_t oldFlags = *(notePointer + eO::flags);
				*(notePointer + eO::flags) = (oldFlags & B00000011) + (val.getSubstep() << 4);
			}
		}

		void toggleAccent(){
			if (isValid()){
				bool state = bitRead((*(notePointer + eO::flags)),accentFlag);
				bitWrite((*(notePointer + eO::flags)),accentFlag,!state);
			}
		}

		void toggleLegato(){
			if (isValid()){
				bool state = bitRead((*(notePointer + eO::flags)),legatoFlag);
				bitWrite((*(notePointer + eO::flags)),legatoFlag,!state);
			}
		}

		note getNote() const {
			if (isValid()){
				note returnNote = readNote(notePointer);
				return returnNote;
			} else {//This should never happen:
				return note();
			}
		}
};

////------------------------------------------------------------- Block Functions:


	position clearPositionForFindingNextBlock(position pos, const contentsType type){
		//This allows for, after creating new blocks, canBlockContainHierarchy to actually contain the hierarchy, otherwise, for example, the new block may still be trying to fit a bar in it.
		position newPos = pos;
		switch (type){
			default:
			case contentsType::child:
			case contentsType::step:
				newPos.setSubstep(0);
			case contentsType::bar:
				newPos.setStep(0);
			case contentsType::pattern:
				newPos.setBar(0);
			case contentsType::none:
				break;
		}
		return newPos;
	}

uint8_t getIndexFromPositionAndcontentsType(position pos, contentsType type){//Find what index to look for given the block type of the parent
	uint8_t num =
		type == contentsType::pattern	? pos.getBar()	:
		type == contentsType::bar	? pos.getStep()	:
		type == contentsType::step	? pos.getSubstep()	:
		16; // Not possible
		return num;
}
//------------------------------//------------------------------//------------------------------BLOCK DEFINITION

//Contents:
const int addressSize = blocks::addressSize;
const uint16_t zeroPointerReplacer = blocks::zeroPointerReplacer;
const int blocksPerBlock = 16;
const int notesPerBlock = 8;
const int noteSize = eO::max;
const int blockDataSize = blocksPerBlock * addressSize;
const int contentsBlockSize = blockDataSize + addressSize + 1; //35 = 16 storage addresses, one concurrent address, type in one byte - Block type could be part of the master type below (I think)

//Patterns:
const int patternHeaderSize = hO::size;
const int patternBlockSize = patternHeaderSize + addressSize; // 25 - The pattern heaeder data and the block

class contents_t : public blocks::block_t{ //This only holds an address to the data which is in a malloc
	public:
		contents_t(contentsType type){
			int32_t nextFreeBlockNum = blocks::getNextFreeBlock(blocks::blockType::contents);//Get the next empty block
			if(nextFreeBlockNum == -1){
				address = zeroPointerReplacer;
				return;
			} else {
				address = nextFreeBlockNum;
				initialise(type);
			}
		}

		contents_t() : block_t(){}//Invalid object

		contents_t(uint16_t ref) : block_t(ref){}

		contentsType getContentsType(){
			return isValid() ? static_cast<contentsType>(*(getPointer()+size-1)) : contentsType::none;
		}

		const char* getContentsTypeName(){
			contentsType type = getContentsType();
			return type == contentsType::none ? "----" : contentsTypeNames[static_cast<int>(getContentsType())];
		}

		void setContentsType(contentsType type){//(used in cosntructor)
			*(getPointer()+size-1) = static_cast<uint8_t>(type);
		}

		bool isChildBlock(){//Find if the container is for cc, notes or sysex
			if(!isValid()){return false;}
			return isChildType(getContentsType());
		}

		void initialise(contentsType type){//Initialise the block
			memset(getPointer()	, (isChildType(type) ? 0 : 255)	, blockDataSize);	//Main block
			memset(getPointer() + blockDataSize	, 255	, addressSize);	//NextBlock
			memset(getPointer() + blockDataSize + addressSize	, 0	, 1);	//Type
			setContentsType(type);
			// memset(getPointer(), 69, size);
		}

		contents_t getBlockAtIndex(const int index){//For parent blocks
			contents_t blockAtIndex = {makeAddress(getPointer() + (index * addressSize))};
			return blockAtIndex;
		}

		void setBlockAtIndex(const int index, contents_t blockToAdd){//For parentblocks
			uint8_t* pointer = (getPointer() + (index * addressSize));
			writeAddress(pointer, blockToAdd.getAddress());
		}

		void clearBlockAtIndex(const int index){
			uint8_t* pointer = (getPointer() + (index * addressSize));
			writeAddress(pointer, zeroPointerReplacer);
		}

		contents_t getNextBlock(){//Get the next block
			return {makeAddress((getPointer() + ((blocksPerBlock+1) * addressSize)))};
		}

		void setNextBlock(contents_t nextBlock){
			writeAddress(
				getPointer() + ((blocksPerBlock+1) * addressSize),
				nextBlock.getAddress()
			);
		}

		void clearNextBlock(){
			writeAddress(
				getPointer() + ((blocksPerBlock+1) * addressSize),
				zeroPointerReplacer
			);
		}

		// void printContents(){//Do a specific override for this
			// lgc("block ");
			// lgc(getAddress());
			// lgc(" - ");
			// lg(getContentsTypeName());
			// for(int i = 0; i < blockSize; i++){
				// lgc(" ");
				// lg(*(getPointer()+i));
			// }
			// lg();
		// }

		bool isEmptyOrHasBlockAtZero(){
			if(isChildBlock()){//CHANGE THIS FOR sysex / CC...
				if(readNote(getPointer()).isValid()){return false;}//If there is no first note
			} else {
				for(int i = 1; i<blocksPerBlock; i++){//Check child blocks: DO not check in block zero as this is a special case
					contents_t checkBlock = getBlockAtIndex(i);
					if(checkBlock.isValid()){return false;}
				}
			}
			return true;
		}

		void writeNote(const uint8_t index, note noteToAdd){
			uint8_t* pointerToElement = getPointer() + (noteSize * index);
			writeNoteData(pointerToElement, eO::pitch	, noteToAdd.getPitch()	);
			writeNoteData(pointerToElement, eO::velocity	, noteToAdd.getVelocity()	);
			uint16_t lengthInSubsteps = noteToAdd.getLengthInSubsteps();
			uint8_t flags = noteToAdd.getFlags();
			uint8_t lengthToWrite = lengthInSubsteps >> 4;
			uint8_t flagsToWrite = flags + ((lengthInSubsteps & B00001111) << 4);
			writeNoteData(pointerToElement, eO::length	, lengthToWrite	);
			writeNoteData(pointerToElement, eO::flags	, flagsToWrite	);
		}

		int findPositionOfNoteOnStep(const note noteToFind){
			for(int i=0; i<notesPerBlock; i++){
				note noteToCheck = readNote(getPointer()+(i*noteSize));
				if(noteToCheck.isValid() && noteToCheck.getPitch() == noteToFind.getPitch()){
					return i;
				}
			}
			return -1;
		}

		bool doesBlockContainPitch(const note noteToFind){
			return findPositionOfNoteOnStep(noteToFind) >= 0;
		}

		int getNumberOfNotesOnStep(){
			for(int i=0; i<notesPerBlock; i++){
				note noteToCheck = readNote(getPointer()+(i*noteSize));
				if(!noteToCheck.isValid()){
					return i;
				}
			}
			return notesPerBlock;
		}

		int findNextHighestNote(const note noteToFind){
			for(int i=0; i<notesPerBlock; i++){
				note noteToCheck = readNote(getPointer()+(i*noteSize));
				if(!noteToCheck.isValid() || noteToCheck.getPitch() > noteToFind.getPitch()){
					return i;
				}
			}
			return -1;//Should never reach here
		}

		noteList getNotesInBlock() const{
			noteList notesToReturn;
			for(int i=0; i<notesPerBlock; i++){
				note noteToCheck = readNote(getPointer()+(i*noteSize));
				if(noteToCheck.isValid()){
					notesToReturn.push_back(noteToCheck);
				} else {
					break;
				}
			}
			return notesToReturn;
		}

		void addOrUpdateNoteFromBlock(note noteToAdd){
			int notePosition = findPositionOfNoteOnStep(noteToAdd); //return -1 if no note found
			int numberOfNotesOnStep = getNumberOfNotesOnStep();
			if(notePosition != -1){//Edit note
				writeNote(notePosition, noteToAdd);
			} else {
				if(numberOfNotesOnStep >= notesPerBlock){
					notifications::stepFull.display();
					return;
				} else {
					int nextHighestNotePosition = findNextHighestNote(noteToAdd);
					memmove(//Move all later notes forward
						getPointer()+((nextHighestNotePosition+1)*noteSize),
						getPointer()+(nextHighestNotePosition*noteSize),
						(numberOfNotesOnStep-nextHighestNotePosition) * noteSize
					);
					writeNote(nextHighestNotePosition, noteToAdd);
					return;
				}
			}
		}

		bool addOrRemoveNoteFromBlock(note noteToAdd){//Returns block is now empty
			int notePosition = findPositionOfNoteOnStep(noteToAdd); //return -1 if no note found
			int numberOfNotesOnStep = getNumberOfNotesOnStep();
			if(notePosition == -1){//Note note found. Add it
				addOrUpdateNoteFromBlock(noteToAdd);
				return false;
			} else { //Must remove note
				memmove(//Move all later notes back
					getPointer()+(notePosition*noteSize),
					getPointer()+((notePosition+1)*noteSize),
					(numberOfNotesOnStep - notePosition - 1) * noteSize
				);
				memset(getPointer()+((numberOfNotesOnStep-1)*noteSize), 0, noteSize);//Clear the last note
				return numberOfNotesOnStep - 1 == 0; //If there are now no notes on this step
			}
		}

		bool removeNoteFromBlock(note noteToRemove){//Returns block is now empty
			int notePosition = findPositionOfNoteOnStep(noteToRemove); //return -1 if no note found
			int numberOfNotesOnStep = getNumberOfNotesOnStep();
			if(notePosition == -1){//Note not found
				return false;
			} else { //Must remove note
				memmove(//Move all later notes back
					getPointer()+(notePosition*noteSize),
					getPointer()+((notePosition+1)*noteSize),
					(numberOfNotesOnStep - notePosition - 1) * noteSize
				);
				memset(getPointer()+((numberOfNotesOnStep-1)*noteSize), 0, noteSize);//Clear the last note
				return numberOfNotesOnStep - 1 == 0; //If there are now no notes on this step
			}
		}

		void emptyNotes(){
			memset(getPointer(), 0, blockDataSize);
			//Does not deal with child steps
		}

		savedNote getNoteInBlock(const note noteToFind){//Returns note or nullptr note
			int pos = findPositionOfNoteOnStep(noteToFind);
			if(pos >= 0){
				return {getPointer()+(noteSize*pos)};
			}
			else{
				return {};
			}
		}

};//end of contents_t

//------------------------------//------------------------------//------------------------------OTHER BLOCK FUNCTONS

contents_t getParentToBlock(position pos, contents_t rootBlock, contents_t findThisBlock){//Recursive
	if(rootBlock == findThisBlock){
		return {};//The parent is THE PATTERN!!!!
	}
	contentsType type = rootBlock.getContentsType();
	contents_t nextBlock =
		type == contentsType::pattern	? rootBlock.getBlockAtIndex(pos.getBar()) :
		type == contentsType::bar	? rootBlock.getBlockAtIndex(pos.getStep()) :
		type == contentsType::step	? rootBlock.getBlockAtIndex(pos.getSubstep()) :
		rootBlock.getNextBlock(); //When using CC, sysex etc.
	if(nextBlock == findThisBlock){//This is here so  when it searches again, the function does not think the parent is the apttern
		return rootBlock;
	}
	return getParentToBlock(pos, nextBlock, findThisBlock);
}

//So repeated reads are not required:
//Data array buggers
// uint8_t lastBlock [contentsBlockSize] = {0};
// uint8_t thisBlock [contentsBlockSize] = {0};
// uint8_t nextBlock [contentsBlockSize] = {0};

//------------------------------------------------------------------------------------------PatternFunctions:

uint32_t getLastPatternNum(){
	return blocks::getLastBlockOfType(blocks::blockType::pattern);
	}

//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------

class pattern_t : public blocks::block_t{
	public:
	//Constructors
		pattern_t(bool u){//Create new pattern and do all memory stuff
			int32_t nextFreeBlockNum = blocks::getNextFreeBlock(blocks::blockType::pattern);
			if(nextFreeBlockNum == -1){
				address = zeroPointerReplacer;
				return;
			} else {
				address = nextFreeBlockNum;
				setType(blocks::blockType::pattern);
				clearData();
				set(hO::stepsPerBar, 16);
				set(hO::patternLength, 16);
				set(hO::transpose, falseZero);
				set(hO::velocity, falseZero);
				set(hO::accentVelocity, falseZero);
				if(givePlaceholderNameToNewPatterns){
					char placeholderName[gc::maxPatternNameLength+1] = "pt";
					char number[5] = {0};
					sprintf(number, "%d", address);
					strcat(placeholderName, number);
					setName(placeholderName);
				}
			}
		}

		pattern_t (uint16_t p_address) : blocks::block_t(p_address){} //Assign a pattern to object
		//pattern_t (const pattern_t other) : blocks::block_t(other.p_address){} //Assign a pattern to object
		//pattern_t& operator= (volatile pattern_t& other) : blocks::block_t(other.p_address){} 
		
		pattern_t (const volatile pattern_t& p) : blocks::block_t(p.getAddress()){}
		pattern_t (const pattern_t& p) : blocks::block_t(p.getAddress()){}
		//pattern_t (const pattern_t& p) : blocks::block_t(p.getAddress()){}
		
		pattern_t operator=(const patMem::pattern_t& p) volatile {
			address = p.getAddress();
			return *this;
		}

		pattern_t() : blocks::block_t(){}//Blank pattern

		void set(hO::headerOffset offset, uint8_t value) volatile {
			if(!isValid()){return;}
			*(getPointer()+offset) = value;
		}

		uint8_t get(hO::headerOffset offset) const volatile{
			if(!isValid()){return 0;}
			return *(getPointer()+offset);
			//return 1;
		}

		void destroy() volatile {
			if(!isValid()){return;}
			clearNotes();
			blocks::block_t::destroy();
		}
	//Getters
		length_t getLength() const volatile {
			if(!isValid()){return {true};}
			int patternLength = get(patMem::hO::patternLength);
			return {patternLength == 0 ? 256 : patternLength};
		}
		
		int getLengthInSteps() const volatile {
			if(!isValid()){return {true};}
			int patternLength = get(patMem::hO::patternLength);
			return patternLength == 0 ? 256 : patternLength;
		}

		contents_t getFirstBlock() const volatile{
			if(!isValid()){return {};}
			return  makeAddress(getPointer() + patternHeaderSize);
		}

		int getStepsPerBar() const  volatile {//Only runs when the pattern is valid
			if(!isValid()){return 0;}
			return get(patMem::hO::stepsPerBar);
		}

		int8_t getVelocity() const volatile {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::velocity) - 127;
		}

		int8_t getAccentVelocity() const volatile {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::accentVelocity) - 127;
		}

		uint8_t getShuffle() const volatile {//Only runs when the pattern is valid
			if(!isValid()){return 0;}
			return get(patMem::hO::shuffle);
		}

		bool isStretchToBar() const volatile {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::stretchToBar);
		}

		int8_t getTranspose() const volatile {//Only runs when the pattern is valid
			if(!isValid()){return 0;}
			signed int transpose = get(patMem::hO::transpose) - 127;
			return transpose;
		}

	//Setters
		void clearData() volatile {
			if(!isValid()){return;}
			memset(getPointer(), 0, patternHeaderSize);
			memset(getPointer() + patternHeaderSize, 255, addressSize);
		}

		void clearNotes() volatile {//Inefficient
			if(!isValid()){return;}
			int barLimit = 16;
			int stepLimit = 16;
			int substepLimit = 16;
			contentsType largest = getFirstBlock().getContentsType();
			printcontentsType(largest);
			switch(largest){
				default: //None / Child : Note / sysex / cc
					substepLimit = 1;
				case contentsType::step:
					stepLimit = 1;
				case contentsType::bar:
					barLimit = 1;
				case contentsType::pattern:
					break;
			}
			// lg(barLimit);
			// lg(stepLimit);
			// lg(substepLimit);
			int counter = 0;
			for(uint8_t bar = 0; bar < barLimit; bar ++){
				for(uint8_t step = 0; step < stepLimit; step ++){
					for(uint8_t substep = 0; substep < substepLimit; substep ++){
						counter ++;
						position pos = {bar, step, substep};
						// pos.print();
						contents_t blockToClear = getChild(pos, contentsType::note);
						if(blockToClear.isValid()){
							// lg("v");
							// lg();
							//printcontentsType(blockToClear.getContentsType());
							blockToClear.emptyNotes();
							//blockToClear.destroy();
							//block_t parentBlock = getParentToBlock(pos, getFirstBlock(), blockToClear);
							//int relevantIndex = getIndexFromPositionAndcontentsType(pos, parentBlock.getContentsType());
							//parentBlock.clearBlockAtIndex(relevantIndex);
							checkContentsAndPotentiallyDestroyBlock(pos, blockToClear);
						}
					}
				}
			}
			if(counter != barLimit * stepLimit * substepLimit){
				lg("CLEARNOTES ERROR!");
			}
		}

		void setFirstBlock(contents_t blockToSet) volatile {
			if(!isValid()){return;}
			writeAddress(getPointer()+patternHeaderSize, blockToSet.getAddress());
		}

		void clearFirstBlock() volatile {
			if(!isValid()){return;}
			writeAddress(getPointer()+patternHeaderSize, zeroPointerReplacer);
		}

		void setStepsPerBar(int stepsPerBar) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			int patternLength = getLengthInSteps();
			if (stepsPerBar < patternLength / 16){
				notifications::barTooShort.display();
				return;
			}
			
			if(stepsPerBar >=1 && stepsPerBar <= 16){
				set(patMem::hO::stepsPerBar, stepsPerBar);
			}
		}
		
		void setLength(int length) volatile {//1-256
			if(!isValid()){return;}
			int stepsPerBar = get(patMem::hO::stepsPerBar);
			if(length > stepsPerBar * 16){
				notifications::patternTooLong.display();
				return;
			}
			set(patMem::hO::patternLength, length % 256);
		}

		void setVelocity(int8_t value) volatile {
			//Value between -127 and 127
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int velocity = constrain(value, -127, 127);
			set(patMem::hO::velocity, velocity+127);
		}

		void setAccentVelocity(int8_t value) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int velocity = constrain(value, -127, 127);
			set(patMem::hO::accentVelocity, velocity+127);
		}

		void setShuffle(int8_t value) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			set(patMem::hO::shuffle, constrain(value,0 , shuffleMax));
		}

		void toggleStretchToBar() volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			uint8_t stretchToBar = get(patMem::hO::stretchToBar);
			set(patMem::hO::stretchToBar, 1-stretchToBar);
		}

		void setStretchToBar(const bool stretch) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			set(patMem::hO::stretchToBar, stretch);
		}

		void setTranspose(int value) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int transpose = constrain(value, -127, 127);
			set(patMem::hO::transpose, transpose + 127);
		}

	//Name

		void getName(char* name) volatile {
			strncpy(
			name,
			isValid() ? reinterpret_cast<char*>(getPointer()+hO::name) : "<nopatt>",
			gc::maxPatternNameLength);
		}

		void setName(const char* newName) volatile {
			if(!isValid()){return;}
			strncpy(reinterpret_cast<char*>(getPointer()+hO::name), newName, gc::maxPatternNameLength);
		}

		void addCharToName(const char character) volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			char name[gc::maxPatternNameLength + 1] = {0};
			getName(name);
			int length = strlen(name);
			if(length >= gc::maxPatternNameLength){
				notifications::nameFull.display();
				return;
			} else {
				name[length] = character;
				setName(name);
				scrn::pattName.update(name);
			}
		}

		void removeCharFromName() volatile {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			char name[gc::maxPatternNameLength + 1] = {0};
			getName(name);
			int length = strlen(name);
			if(length >= 1){
				name[length-1] = 0;
				setName(name);
				scrn::pattName.update(name);
			}
		}

	//Complex
		contents_t getChild (position pos, contentsType typeToFind) const volatile{//Based on createChild
			if(!isValid()){return {};}
			position checkPos = pos;
			contents_t currentBlock = getFirstBlock();
			if(currentBlock.isValid()){
				contentsType currentcontentsType = currentBlock.getContentsType();
				while(true){
					bool canHoldBlock = checkPos.cancontentsTypeHoldPos(currentcontentsType);
					bool isRightBlock = currentcontentsType == typeToFind;
					if(canHoldBlock && isRightBlock){
						return currentBlock;
					}
					else if(!canHoldBlock){
						return {};
					}
					else{ //!isRightBlock
						uint8_t relevantIndex = getIndexFromPositionAndcontentsType(checkPos, currentcontentsType);
						currentBlock = currentBlock.getBlockAtIndex(relevantIndex);
						if(!currentBlock.isValid()){
							return {};
						}
						checkPos = clearPositionForFindingNextBlock(checkPos, currentcontentsType);
						currentcontentsType = currentBlock.getContentsType();
					}
				}
			}
			return {};
		}

		contents_t createChild (position pos, contentsType typeToMake) volatile{
			if(!isValid()){return {};}
			contents_t parentBlock = {};
			position checkPos = pos;
			contents_t currentBlock = getFirstBlock();
			contentsType currentcontentsType = currentBlock.isValid() ? currentBlock.getContentsType() : contentsType::none;

			while(true){
				bool canHoldBlock = checkPos.cancontentsTypeHoldPos(currentcontentsType);
				bool isRightBlock = currentcontentsType == typeToMake;
				// lgc("canHold:");
				// lgc(canHoldBlock);
				// lgc(" ");
				// lg(isRightBlock);
				// lg();
				if(canHoldBlock && isRightBlock){
					return currentBlock;
				}
				else if(!canHoldBlock){//If this block can't hold it, then create a new block, put this block in it and then "keep looking" to create a new one
					contentsType newType = checkPos.getLargestTypeRequiredToHold();
					if(newType == contentsType::child){newType = typeToMake;}
					contents_t newBlock = {newType};
					if(!newBlock.isValid()){	return {};	}
					if(currentBlock.isValid()){newBlock.setBlockAtIndex(0, currentBlock);}
					if(parentBlock.isValid()){
						uint8_t relevantIndex = getIndexFromPositionAndcontentsType(pos, parentBlock.getContentsType());
						parentBlock.setBlockAtIndex(relevantIndex, newBlock);
					} else{
						setFirstBlock(newBlock);
					}
					//Do it again for the new block
					//checkPos = checkPos(checkPos, newType); //maybe not required because working on same level?
					currentBlock = newBlock;
					currentcontentsType = newBlock.getContentsType();
				}
				else{ //!isRightBlock
					uint8_t relevantIndex = getIndexFromPositionAndcontentsType(checkPos, currentcontentsType);
					contents_t nextBlock = currentBlock.getBlockAtIndex(relevantIndex);
					checkPos = clearPositionForFindingNextBlock(checkPos, currentcontentsType);
					if(!nextBlock.isValid()){//If there is no next block, create a new one:
						contentsType newType = checkPos.getLargestTypeRequiredToHold();
						if(newType == contentsType::child){newType = typeToMake;}
						contents_t newBlock = {newType};
						if(!newBlock.isValid()){	return {};	}
						currentBlock.setBlockAtIndex(relevantIndex, newBlock);
						nextBlock = newBlock;
					}
					parentBlock = currentBlock;
					currentBlock = nextBlock;
					currentcontentsType = currentBlock.getContentsType();
				}
			}
			return {};
		}

		void checkContentsAndPotentiallyDestroyBlock(position pos, contents_t noteBlock) volatile{
			if(!isValid()){return;}
			contents_t blockToCheck = noteBlock;
			while(true){
				bool isEmpty = blockToCheck.isEmptyOrHasBlockAtZero();
				// printcontentsType(blockToCheck.getContentsType());
				// lg(blockToCheck.getAddress());
				// lg(isEmpty ? "empty" : "not empty");
				// lg();
				contents_t parentBlock = getParentToBlock(pos, getFirstBlock(), blockToCheck);
				if(!isEmpty){return;}//Nothing must be done
				contents_t zeroBlock = blockToCheck.isChildBlock() ? contents_t{} : blockToCheck.getBlockAtIndex(0);
				bool hasBlockOnZero = zeroBlock.isValid();
				if(hasBlockOnZero){
					if(parentBlock.isValid()){
						uint8_t relevantIndex = getIndexFromPositionAndcontentsType(pos, parentBlock.getContentsType());
						parentBlock.setBlockAtIndex(relevantIndex, zeroBlock);
					}
					else { //Pattern is parent
						setFirstBlock(zeroBlock);
					}
					blockToCheck.destroy();
					return; //Parent is not empty
				}
				blockToCheck.destroy();
				if(!parentBlock.isValid()){
					clearFirstBlock();
					return;//Reached pattern. All clear
				} else {
					int relevantIndex = getIndexFromPositionAndcontentsType(pos, parentBlock.getContentsType());
					parentBlock.clearBlockAtIndex(relevantIndex);
					blockToCheck = parentBlock;
				}
			}
		}

		void addOrRemoveNote(note noteToAdd, position pos) volatile {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = createChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks
			bool rem = noteBlock.addOrRemoveNoteFromBlock(noteToAdd);

			if(rem){checkContentsAndPotentiallyDestroyBlock(pos, noteBlock);}
		}

		void removeNote(note noteToRemove, position pos) volatile {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = getChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks or nir block on the step
			bool rem = noteBlock.removeNoteFromBlock(noteToRemove);
			if(rem){checkContentsAndPotentiallyDestroyBlock(pos, noteBlock);}
		}

		void addOrUpdateNote(note noteToAdd, position pos) volatile {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = createChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks
			noteBlock.addOrUpdateNoteFromBlock(noteToAdd);
		}

		void clearStepAndSubstepOfNotes(position pos) volatile {
			if(!isValid()){return;}
			for(int i=0; i<blocksPerBlock; i++){
				contents_t blockToClear = getChild(pos, contentsType::note);
				if(!blockToClear.isValid()){continue;}
				blockToClear.destroy();
				checkContentsAndPotentiallyDestroyBlock(pos, blockToClear);
			}
		}

		void clearStepOfNotes(position pos) volatile{
			if(!isValid()){return;}
			contents_t blockToClear = getChild(pos, contentsType::note);
			if(!blockToClear.isValid()){return;}
			blockToClear.emptyNotes();
			checkContentsAndPotentiallyDestroyBlock(pos, blockToClear);
		}

		noteList getNotesOnStep(position pos) volatile const{
			if(!isValid()){return {};}
			contents_t noteBlock = getChild(pos, contentsType::note);
			return noteBlock.isValid() ? noteBlock.getNotesInBlock() : noteList();
		}

		void clearStepAndMaybeEraseParent(position pos) volatile {
			// lg("a");
			if(!isValid()){return;}
			// lg("b");
			contents_t noteBlock = getChild(pos, contentsType::note);
			// lg("c");
			//Error here:
			if(noteBlock.isValid()){
				// lg("d");
				noteBlock.emptyNotes();
				// lg("e");
				checkContentsAndPotentiallyDestroyBlock(pos, noteBlock);
			}
			lg("f");
		}

		// void clearStepsFromRange(position first, position last){
			// if(!isValid()){return;}
			// patMem::forPositionRangeSubsteps(first, last, [p=*this](position toClear, bool& earlyExit) mutable{
				// //patMem::pattern_t newPatt = p; //This is to fix const stuff
				// p.clearStepOfNotes(toClear);
				// });
			// }

		std::vector<uint8_t> getPitchesOnStep(position pos, const bool searchSubstepInsteadOfStep, const bool searchSubstepsWithinStep) volatile {
			if(!isValid()){return {};}
			noteList notes;
			if(!searchSubstepInsteadOfStep){//If we just want to see what pitches are on a step, maybe get  all the notes on all the substeps too,
				int limit = searchSubstepsWithinStep ? 16 : 1;
				for(int substep = 0; substep <limit; substep ++){
					pos.setSubstep(substep);
					noteList temp = getNotesOnStep(pos);
					notes.insert(std::end(notes), std::begin(temp), std::end(temp));
				}
			} else {//Wnat the notes specifically on a substep
				notes = getNotesOnStep(pos);
			}
			std::vector<uint8_t> pitches;
			for(note n : notes){
				pitches.push_back(n.getPitch());
			}
			return pitches;
		}

		//For LEDs - Given a (bar or step) and a note, provide what steps or substeps contain that note
		std::vector<uint8_t> getStepsFromNote(const note pitchToFind, const uint8_t bar, const int8_t step/*-1 for search within bar*/, const bool searchSubstepsToo) volatile {
			if(!isValid()){return {};}
			//return {};
			
			//pos.print();
			std::vector<uint8_t> stepsFromNote;
			if(step == -1){//If looking within a bar
				for(uint8_t s = 0; s <16; s ++){//For each step or substep
					// lg(s);
					uint8_t substepLimit = searchSubstepsToo ? 16 : 1 ; //Only search step one or all 16 if searching substeps too
					// lg("a");
					for(uint8_t substep = 0; substep <substepLimit; substep ++){
						// lgc("FOR: ");
						// lgc(bar);
						// lgc(" ");
						// lgc(s);
						// lgc(" ");
						// lg(substep);
						position pos = {bar, s, substep};
						// lg("b");
						contents_t noteBlock = getChild(pos, contentsType::note);
						// lg("c");
						if(noteBlock.isValid()){
							// lgc("found: ");
							// pos.print();
							// lg("d");
							bool containPitch = noteBlock.doesBlockContainPitch(pitchToFind);
							// lg("e");
							if(containPitch){
								// lg("f");
								stepsFromNote.push_back(s);
								break;//Only needs one substep to turn on so can skip the rest
							}
						}
					}
				}
			}
			else {//If are looking within a step
				for(uint8_t substep = 0; substep <16; substep ++){
					position pos = {bar, (uint8_t)step, substep};
					contents_t noteBlock = getChild(pos, contentsType::note);
					if(noteBlock.isValid()){
						bool containPitch = noteBlock.doesBlockContainPitch(pitchToFind);
						if(containPitch){
							stepsFromNote.push_back(substep);
						}
					}
				}
			}
			//return {};
			return stepsFromNote;
		}

		savedNote getNoteOnStep(const position pos, const note noteToFind) volatile {
			if(!isValid()){return {};}
			contents_t noteBlock = getChild(pos, contentsType::note);
			return noteBlock.isValid() ? noteBlock.getNoteInBlock(noteToFind) : savedNote();
		}
	
		//Old method with range - unused
		// void copySteps(pattern_t sourcePattern, const position sourceStart, const position sourceEnd, const position destinationPos, const bool clear, const bool wrap){//Two copies using a buffer
			// if(!isValid()){
				// notifications::noDestinationPattern.display();
				// return;
				// }
			// if(!sourcePattern.isValid()){
				// notifications::noSourcePattern.display();
				// return;
			// }
			// //Maybe check if buffer is needed? Doesn't really matter but would be good
			// lg("zero");
			// pattern_t tempBuffer = pattern_t{true};
			// if(!tempBuffer.isValid()){
				// return;
			// }
			// tempBuffer.clearData();
			// lg("first copy:");
			// tempBuffer.copyStage(sourcePattern, sourceStart, sourceEnd, {0,0}, false, false);
			// lg("second copy:");
			// //Getting error in clear mode:
			// copyStage(tempBuffer, {0,0}, sourceEnd-sourceStart, destinationPos, clear, wrap);
			// lg(3);

			// // copyStage(sourcePattern, sourceStart, sourceEnd, destinationPos, clear, wrap);

			// tempBuffer.clearNotes();
			// lg(4);
		// }

		// void copyStage(pattern_t sourcePattern, const position pos, position destinationPos, const bool clear, const bool wrap){//A single copy without buffer
			// length_t wrapPos = wrap ? getLength() : length_t{true};
			// if(clear){clearStepAndMaybeEraseParent(destinationPos);}
			// noteList notesOnStep = sourcePattern.getNotesOnStep(pos);
			// for(auto n : notesOnStep){
				// addOrUpdateNote(n, destinationPos);
			// }
			// destinationPos.incrementSubstep(wrapPos);
		// }
		
		void copyStep(pattern_t sourcePattern, const position pos, const position destinationPos, const bool clear) volatile {//Two copies using a buffer
			//Maybe check if buffer is needed? Doesn't really matter but would be good

			//length_t wrapPos = wrap ? getLength() : length_t{true};
			if(clear){clearStepAndMaybeEraseParent(destinationPos);}
			noteList notesOnStep = sourcePattern.getNotesOnStep(pos);
			for(auto n : notesOnStep){
				addOrUpdateNote(n, destinationPos);
			}
			
		}

		void copyFromPattern(const pattern_t sourcePattern) volatile {
			//copyStage(sourcePattern, {}, {false}, {}, false, false); //No buffer required
			for(uint16_t i = 0; i < gc::numberOfSubsteps; i++){
				copyStep(sourcePattern, {i}, {i}, false);
			}
			
			if(incrementNumberToDuplicatePatterns){
				char patternName[gc::maxPatternNameLength + 1];
				getName(patternName);
				utl::incrementString(patternName, gc::maxPatternNameLength);
				setName(patternName);
			}
		}

		void quantiseStep(const position pos, const uint8_t quantiseLevel, const bool wrapQuantise) volatile {
			if(!isValid()){
				notifications::noPatternOnTrack.display();
				return;
			}
			noteList notesOnStep = getNotesOnStep(pos);
			position quantisedPosition = pos;
			quantisedPosition.quantise(quantiseLevel, wrapQuantise ? getLength() : length_t{true});
			for(auto n : notesOnStep){
				addOrRemoveNote(n, pos);
				addOrUpdateNote(n, quantisedPosition);
			}
		}
	//LEDs
	
	//Given a step or substep, see which notes are on it
		std::array<bool, gc::keyNum::notes> getNoteLeds(const byte startPitch, const position pos, const bool isInSubstep, const bool searchSubstepsInStep) volatile {
			std::array<bool, gc::keyNum::notes> returnLEDs = {false};
			if(!isValid()){return returnLEDs;}
			auto pitches = getPitchesOnStep(pos, isInSubstep, searchSubstepsInStep);
			for (const auto pitch : pitches){
				signed int LEDpitch = pitch - startPitch;
				if (LEDpitch >= 0 && LEDpitch < gc::keyNum::notes){
					returnLEDs[LEDpitch] = true;
				}
			}
			return returnLEDs;
		}
		
		//Given a note, see on which steps or substeps it appears
		std::array<bool, gc::keyNum::steps> getStepLeds(const uint8_t pitch, const int bar, const int step, const bool searchSubstepToo) volatile {
			std::array<bool, gc::keyNum::steps> returnLEDs = {false};
			if(!isValid()){return returnLEDs;}
			auto steps = getStepsFromNote({pitch}, bar, step, searchSubstepToo);
			for (const auto step : steps){
				returnLEDs[step] = true;
			}
			return returnLEDs;
		}

		void copyHeader(pattern_t otherPatt) volatile {
			if(!isValid() || !otherPatt.isValid()){return;}
			memcpy(getPointer(), otherPatt.getPointer(), patternHeaderSize);
		}

		//Saving and loading
		void saveToCard(){
			//comms::send(comms::MOSIType_t::savePatternHeader, getPointer(), hO::size); //Send Header
			//const uint8_t status = comms::waitForAcknowledge();
			//if(status == -1){return;}
			// forPositionRangeSubsteps({}, {false}, [this](position pos, bool& earlyExit){
				// const int status = comms::waitForAcknowledge();
				// if(status == -1){earlyExit = true; return;}//This only returns from one substep
				// contents_t childBlock = getChild(pos, contentsType::child);
				// if(childBlock.isValid()){
					// uint8_t sendBuffer[contentsBlockSize + 2] = {0};
					// sendBuffer[0] = pos.getValue() >> 8;
					// sendBuffer[1] = pos.getValue() % 256;
					// uint8_t* childBlockPointer = childBlock.getPointer();
					// for(int i=0; i < contentsBlockSize; i++){
						// sendBuffer[2+i] = *(childBlockPointer+i);
					// }
					// comms::send(comms::MOSIType_t::savePatternStep, sendBuffer, contentsBlockSize);
				// }
			// });
			// comms::send(comms::MOSIType_t::saveEnd);
			// const int status = comms::waitForAcknowledge();
			// if(status == 0){
				// notifications::saved.display();
			// }
			//
		}
};//End pattern_t

//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
bool doesPatternExist(pattern_t patt){
	return patt.getType() == blocks::blockType::pattern;
}
bool doesPatternExist(uint16_t patt){
	return pattern_t(patt).getType() == blocks::blockType::pattern;
}

}//End namespace
#endif