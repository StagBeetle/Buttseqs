#include <bitset>
#include <functional>
#include "Arduino.h"
#include "patternStorage.h"
#include <stdint.h>

#include <stdint.h>
#include <stdio.h>
#include "Arduino.h"
#include "utility.h"
#include "forwardDec.h"
#include "notifications.h"
#include "compatibility.h"
#include "debug.h"

//#include "chipmemory.h"
//#include "comms.h"

namespace patMem{
	
//Settings
	bool extendPatternWhenSettingBars = true;

const int shuffleMax = 15;

const int addressSize = blocks::addressSize;
const int patternHeaderSize = hO::size;
const int patternBlockSize = patternHeaderSize + addressSize; // 25 - The pattern heaeder data and the block

class eO{public:
	enum noteElementOffset{
		velocity	= 0,
		pitch	= 1,
		length	= 2,
		flags	= 3,
		max	= 4,
	};
};

const char* const contentsTypeNames[] = {
	"none"	,
	"child"	,
	"note"	,
	"cc"	,
	"sysex"	,
	"nrpn"	,
	"exec"	,
	"process"	,
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

//Length_t
	length_t::length_t(const int p_length) : 
		length(p_length == 256 ? 0 : p_length){}
	length_t::length_t(bool) : 
		length (0){}
	int length_t::getLength() const{
		return length == 0 ? 256 : length;
		}
	uint16_t length_t::getSubstepEquivalent() const{
		return getLength() * 16;
		}

//Position
		position::position(const uint8_t bar, const uint8_t step, const uint8_t substep){
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

		position::position(bool u){
			//Overlimit is just for wrapping
			value = u ? overLimit : maxSize;
		}

		position::position(uint16_t num){
			value = num;
		}

		uint8_t position::getBar()	const {return (value >> 8)	;}
		uint8_t position::getStep()	const {return (value >> 4) % 16	;}
		uint8_t position::getSubstep()	const {return (value >> 0) % 16	;}

		void position::set(const uint8_t bar, const uint8_t step, const uint8_t substep) {
			value = (bar<<8) + (step<<4) + substep;
		}
		
		position position::offset(const int8_t bar, const int8_t step, const int8_t substep) {
			value += (bar*256);
			value += (step*16);
			value += (substep);
			value += overLimit;
			value %= overLimit;
			return *this;
		}

		uint16_t position::getValue() const{
			return value;
		}

		void position::setBar(const uint8_t bar){
			value = (value & (stepMask + substepMask)) + (bar<<8);
		}

		void position::setStep(const uint8_t step){
			value = (value & (barMask + substepMask)) + (step<<4);
		}

		void position::setSubstep(const uint8_t substep){
			value = (value & (barMask + stepMask)) + (substep);
		}

		position& position::operator = (const position& other){
			value = other.value;
			return *this;
		}

		position position::quantise(uint8_t level, length_t wrapper){
			int nearestSubstep  = 256 / level;
			int modValue = value % nearestSubstep; //Plus one so a halfway note rounds up - e.g. 16th notes would quantise to next 8th note rather than last
			value = (modValue+1 > (nearestSubstep / 2)) ? value + (nearestSubstep - modValue) : value - modValue;
			// lg(wrapper.getSubstepEquivalent());
			value %= wrapper.getSubstepEquivalent();
			// print();
			// lg();
			return {value};
		}

		position position::shuffle(uint8_t level){
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

		void position::makeString(char* outArray, char spacer, uint8_t depth) const{
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

		void position::print() const{
			char str[9] = {0};
			makeString(str, ':');
			lg(str);
		}

		contentsType position::getLargestTypeRequiredToHold() const {
			return
				getBar()	> 0 ? contentsType::pattern	:
				getStep()	> 0 ? contentsType::bar	:
				getSubstep()	> 0 ? contentsType::step	:
				contentsType::child;
		}

		bool position::cancontentsTypeHoldPos(contentsType type){
			return static_cast<int>(type) >= static_cast<int>(getLargestTypeRequiredToHold());
		}

		// friend position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper);
		// friend position add(const position add1, const position add2, const length_t wrapper);
		// friend bool operator == (const position& one, const position& other);
		// friend bool operator <	(const position& one, const position& other);
		// friend position operator -	(const position& one, const position& other);

position subtract(const position subtractFrom, const position subtractThis, const length_t wrapper){//For lengths
	// lg(subtractFrom.getValue());
	// lg(subtractThis.getValue());
	int32_t answer = subtractFrom.getValue() - subtractThis.getValue();
	answer = (answer + position::overLimit) % wrapper.getSubstepEquivalent();
	// lg(wrapper.getSubstepEquivalent());
	// lg(answer);
	// lg();
	return {static_cast<uint16_t>(answer)};
}

position add(const position add1, const position add2, const length_t wrapper){
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

//Note
		note::note(uint8_t p){//When only a pitch is needed
			setPitch(p);
		}

		note::note(uint8_t p, uint8_t v, position l){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			//setSubstepLength(15);
			}

		note::note(uint8_t p, uint8_t v, position l, uint8_t f){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			setFlags(f);
			//setPosition(o);
			}

		note::note(uint8_t p, uint8_t v, position l, bool a, bool leg){
			setPitch(p);
			setVelocity(v);
			setLength(l);
			setAccent(a);
			setLegato(leg);
			}

		note::note(const note &c_note):
			pitch(c_note.getPitch()),
			velocity(c_note.getVelocity()),
			length(c_note.getLength()),
			flags(c_note.getFlags())
			{}

		void note::setNote(uint8_t p, uint8_t v, position l){
			pitch = constrain(p, 0, 127);
			velocity = constrain(v, 1, 127);
			length = l;
			}
			
		void note::print(){
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

		void note::setPitch(int p){
			pitch = constrain(p, 0, 127);
		}

		void note::setLength(position l){
			length = l;
		}

		void note::setLength(bool isShiftHeld){//For recording notes
			length = isShiftHeld ? patMem::position{0,0,0} :  patMem::position{0,0,1};
		}

		position& note::getLength(){
			return length;
		}

		void note::setLengthWithoutSubstep(position l){
			int substep = length.getSubstep();
			length = l;
			length.setSubstep(substep);
		}

		// void setSubstepLength(int l){
			// length = constrain(l,0,15);
		// }

		void note::setVelocity(int v){velocity = constrain(v, 1, 127);}
		void note::setFlags(int f){flags = f;}
		bool note::isValid(){return (velocity == 0) ? false : true;} //Checks if the note is there or just blank
		void note::clear(){velocity = 0;}
		uint8_t note::getPitch() const {return pitch;}
		uint8_t note::getVelocity() const {return velocity;}
		position note::getLength() const {return length;}

		uint16_t note::getLengthInSubsteps() const {return length.getValue();}

		uint8_t note::getFlags() const {return flags;}
		bool note::getAccent() const {return bitRead(flags,accentFlag);}
		bool note::getLegato() const {return bitRead(flags,legatoFlag);}
		const char* note::getAccentString() const {
			return bitRead(flags,accentFlag) ? "A" : "_";
			}
		const char* note::getLegatoString() const {
			return bitRead(flags,legatoFlag) ? "L" : "_";
			}
		void note::toggleAccent(){bitWrite(flags,accentFlag,!getAccent());}
		void note::toggleLegato(){bitWrite(flags,legatoFlag,!getLegato ());}
		void note::setAccent(bool state){bitWrite(flags,accentFlag,state);}
		void note::setLegato(bool state){bitWrite(flags,legatoFlag ,state);}
		bool note::operator==(const note &other) const {
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

//notePos
		notePos::notePos(const note c_note, const position c_position):
			n(c_note),
			p(c_position)
			{}
		bool notePos::isValid(){
			return n.isValid();
			}

//noteoff

		uint8_t noteOff::getPitch() const
			{return pitch;}
		uint8_t noteOff::getChannel() const
			{return (channelAndPort & 15)+1;}
		uint8_t noteOff::getPort() const
			{return (channelAndPort >> 4) & 15;}
		int noteOff::getLength() const
			{return length;}
		noteOff::noteOff()
			{channelAndPort = 0; pitch = 128; length = 0;}
		noteOff::noteOff(const int c_pitch, const int channel, const int port) ://For playing a noteOff straight away, so length does not matter
			pitch(c_pitch),
			length(0),
			channelAndPort(channel-1 + (port << 4))
			{}
		void noteOff::setNoteOff(uint8_t p, int l, uint8_t c, uint8_t port)
			{pitch = constrain(p,0,127); length = l; channelAndPort = (port << 4) + (c-1);}
		void noteOff::clearNoteOff()
			{pitch = 128;}
		void noteOff::decrementLength()
			{length -- ;}
		void noteOff::setLength(const int newLength){
			length = newLength;
			}
		bool noteOff::isValid()
			{return (pitch != 128);} //Checks if the note is there or just blank

enum class playMode{
	forward,
	reverse,
	bounce,
	random
	};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------- Memory allocation -------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//For control:
//uint_t patternsInMemory = 0;
const uint8_t falseZero = 127;//In some unsigned bytes, this represents zero.

bool givePlaceholderNameToNewPatterns = true;
bool incrementNumberToDuplicatePatterns = true;

uint32_t makeAddress(uint8_t MSB, uint8_t LSB){
	return (MSB << 8) + LSB;
}
uint32_t makeAddress(uint8_t* pointer){
	uint8_t MSB = *pointer;
	uint8_t LSB = *(pointer+1);
	return makeAddress(MSB, LSB);
}

void writeAddress(uint8_t* pointer, uint32_t address){
	//TODO: DO NOT DISREGARD THE HIGH BITS
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

		savedNote::savedNote(uint8_t* newPointer){
			notePointer = newPointer;
			}

		bool savedNote::isValid() const{
			return (notePointer != nullptr);
			}

		void savedNote::setVelocity(const uint8_t newVal){
			if (isValid()){
				*(notePointer + eO::velocity) = constrain(newVal,1,127);
			}
		}

		void savedNote::setLengthWithoutSubstep (const position newPos){//Readjust for substep
			if (isValid()){
				*(notePointer + eO::length) = newPos.getValue() >> 4;
			}
		}

		void savedNote::setLength (const position newVal){//Readjust for substep
			if (isValid()){
				uint16_t lengthInSubsteps = newVal.getValue();
				*(notePointer + eO::length) = lengthInSubsteps >> 4;
				uint8_t flags = *(notePointer + eO::flags) & B00000011;
				*(notePointer + eO::flags) = flags + ((lengthInSubsteps % 16) << 4);
			}
		}

		void savedNote::setLengthBars(const position val){
			if (isValid()){
				uint8_t oldLength = *(notePointer + eO::length);
				*(notePointer + eO::length) = (oldLength & B00001111) + (val.getBar() << 4);
			}
		}

		void savedNote::setLengthSteps(const position val){
			if (isValid()){
				uint8_t oldLength = *(notePointer + eO::length);
				*(notePointer + eO::length) = (oldLength & B11110000) + val.getStep();
			}
		}

		void savedNote::setLengthSubsteps(const position val){
			if (isValid()){
				uint8_t oldFlags = *(notePointer + eO::flags);
				*(notePointer + eO::flags) = (oldFlags & B00000011) + (val.getSubstep() << 4);
			}
		}

		void savedNote::toggleAccent(){
			if (isValid()){
				bool state = bitRead((*(notePointer + eO::flags)),accentFlag);
				bitWrite((*(notePointer + eO::flags)),accentFlag,!state);
			}
		}

		void savedNote::toggleLegato(){
			if (isValid()){
				bool state = bitRead((*(notePointer + eO::flags)),legatoFlag);
				bitWrite((*(notePointer + eO::flags)),legatoFlag,!state);
			}
		}

		note savedNote::getNote() const {
			if (isValid()){
				note returnNote = readNote(notePointer);
				return returnNote;
			} else {//This should never happen:
				return note();
			}
		}

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
const uint32_t zeroPointerReplacer = blocks::zeroPointerReplacer;
const int blocksPerBlock = 16;
const int notesPerBlock = 8;
const int noteSize = eO::max;
const int blockDataSize = blocksPerBlock * addressSize;
const int contentsBlockSize = blockDataSize + addressSize + 1; //35 = 16 storage addresses, one concurrent address (like cc), type in one uint8_t- Block type could be part of the master type below (I think)

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

		contents_t(uint32_t ref) : block_t(ref){}

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

char currentPatternName[17] = {0}; //This is bad

uint32_t getLastPatternNum(){
	return blocks::getLastBlockOfType(blocks::blockType::pattern);
	}

//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------PATTERN OBJECT:-----------------------------------------------------------------------

		pattern_t::pattern_t(bool u){//Create new pattern and do all memory stuff
			int32_t nextFreeBlockNum = blocks::getNextFreeBlock(blocks::blockType::pattern);
			if(nextFreeBlockNum == -1){
				address = zeroPointerReplacer;
				return;
			} else {
				address = nextFreeBlockNum;
				setType(blocks::blockType::pattern);
				clearData();
				set(hO::stepsPerBar, 16);
				set(hO::numberOfBars, 1);
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

		pattern_t::pattern_t (uint32_t p_address) : blocks::block_t(p_address){} //Assign a pattern to object
		//pattern_t (const pattern_t other) : blocks::block_t(other.p_address){} //Assign a pattern to object
		//pattern_t& operator= ( pattern_t& other) : blocks::block_t(other.p_address){} 
		
		pattern_t::pattern_t (const pattern_t& p) : blocks::block_t(p.getAddress()){}
		//pattern_t (const pattern_t& p) : blocks::block_t(p.getAddress()){}
		
		pattern_t::pattern_t() : blocks::block_t(){}//Blank pattern
		
		pattern_t pattern_t::operator=(const patMem::pattern_t& p) {
			address = p.getAddress();
			return *this;
		}

		void pattern_t::set(hO::headerOffset offset, uint8_t value)  {
			if(!isValid()){return;}
			*(getPointer()+offset) = value;
		}

		uint8_t pattern_t::get(hO::headerOffset offset) const {
			if(!isValid()){return 0;}
			return *(getPointer()+offset);
			//return 1;
		}

		void pattern_t::destroy()  {//This is not a destructor but probably should be, though the object itself is justa stupid two uint8_tpiece of shit
			if(!isValid()){return;}
			clearAll();
			blocks::block_t::destroy();
		}
	//Getters
		length_t pattern_t::getLength() const  {
			if(!isValid()){return {true};}
			int patternLength = get(patMem::hO::patternLength);
			return {patternLength == 0 ? 256 : patternLength};
		}
		
		int pattern_t::getLengthInSteps() const  {
			if(!isValid()){return {true};}
			int patternLength = get(patMem::hO::patternLength);
			return patternLength == 0 ? 256 : patternLength;
		}

		contents_t pattern_t::getFirstBlock() const {
			if(!isValid()){return {};}
			return  makeAddress(getPointer() + patternHeaderSize);
		}

		int pattern_t::getStepsPerBar() const   {//Only runs when the pattern is valid
			if(!isValid()){return 16;}
			return get(patMem::hO::stepsPerBar);
		}

		int8_t pattern_t::getVelocity() const  {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::velocity) - 127;
		}

		int8_t pattern_t::getAccentVelocity() const  {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::accentVelocity) - 127;
		}

		uint8_t pattern_t::getShuffle() const  {//Only runs when the pattern is valid
			if(!isValid()){return 0;}
			return get(patMem::hO::shuffle);
		}

		bool pattern_t::isStretchToBar() const  {//Only runs if pattern valid;
			if(!isValid()){return 0;}
			return get(patMem::hO::stretchToBar);
		}

		int8_t pattern_t::getTranspose() const  {//Only runs when the pattern is valid
			if(!isValid()){return 0;}
			signed int transpose = get(patMem::hO::transpose) - 127;
			return transpose;
		}

	//Setters
		void pattern_t::clearData()  {
			if(!isValid()){return;}
			memset(getPointer(), 0, patternHeaderSize);
			memset(getPointer() + patternHeaderSize, 255, addressSize);
		}

		void pattern_t::clearNotes()  {//Inefficient
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
		
		void pattern_t::clearAll()  {
			clearData();
			clearNotes();
		}

		void pattern_t::setFirstBlock(contents_t blockToSet)  {
			if(!isValid()){return;}
			writeAddress(getPointer()+patternHeaderSize, blockToSet.getAddress());
		}

		void pattern_t::clearFirstBlock()  {
			if(!isValid()){return;}
			writeAddress(getPointer()+patternHeaderSize, zeroPointerReplacer);
		}

		void pattern_t::setStepsPerBar(int stepsPerBar)  {
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
		
		void pattern_t::setLength(int length)  {//1-256
			if(!isValid()){return;}
			int stepsPerBar = get(patMem::hO::stepsPerBar);
			if(length > stepsPerBar * 16){
				notifications::patternTooLong.display();
				return;
			}
			set(patMem::hO::patternLength, length % 256);
		}
		
		int pattern_t::getNumberOfBars() const {
			if(!isValid()){return 1;}
			int numberOfBars = get(patMem::hO::numberOfBars);
			return numberOfBars;
		}
		
		//This set how many bars and will set the length of the pattern if bars * steps is longer than the current pattrern (it will only extend)
		void pattern_t::setNumberOfBars(int numberOfBars)  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			
			
			
			if(numberOfBars >=1 && numberOfBars <= 16){
				set(patMem::hO::numberOfBars, numberOfBars);
				//Maybe extend pattern
				if(extendPatternWhenSettingBars){
					int lengthProduct = numberOfBars * getStepsPerBar();
					if(lengthProduct > getLength().getLength()){
						setLength(lengthProduct);
					}
				}
			}
		}

		void pattern_t::setVelocity(int8_t value)  {
			//Value between -127 and 127
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int velocity = constrain(value, -127, 127);
			set(patMem::hO::velocity, velocity+127);
		}

		void pattern_t::setAccentVelocity(int8_t value)  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int velocity = constrain(value, -127, 127);
			set(patMem::hO::accentVelocity, velocity+127);
		}

		void pattern_t::setShuffle(int8_t value)  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			set(patMem::hO::shuffle, constrain(value,0 , shuffleMax));
		}

		void pattern_t::toggleStretchToBar()  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			uint8_t stretchToBar = get(patMem::hO::stretchToBar);
			set(patMem::hO::stretchToBar, 1-stretchToBar);
		}

		void pattern_t::setStretchToBar(const bool stretch)  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			set(patMem::hO::stretchToBar, stretch);
		}

		void pattern_t::setTranspose(int value)  {
			if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			signed int transpose = constrain(value, -127, 127);
			set(patMem::hO::transpose, transpose + 127);
		}

	//Name

		void pattern_t::getName(char* name) const {
			strncpy(
			name,
			isValid() ? reinterpret_cast<char*>(getPointer()+hO::name) : "<nopatt>",
			gc::maxPatternNameLength);
		}
		
		const char* pattern_t::getName() const  {
			getName(currentPatternName);
			return currentPatternName;
		}

		void pattern_t::setName(const char* newName)  {
			if(!isValid()){return;}
			strncpy(reinterpret_cast<char*>(getPointer()+hO::name), newName, gc::maxPatternNameLength);
		}
		
		//Removed due to different keyboard methods:
		// void pattern_t::addCharToName(const char character)  {
			// if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			// char name[gc::maxPatternNameLength + 1] = {0};
			// getName(name);
			// int length = strlen(name);
			// if(length >= gc::maxPatternNameLength){
				// notifications::nameFull.display();
				// return;
			// } else {
				// name[length] = character;
				// setName(name);
				// scrn::pattName.update(name);
			// }
		// }

		// void pattern_t::removeCharFromName()  {
			// if(!isValid()){notifications::noPatternOnTrack.display(); return;}
			// char name[gc::maxPatternNameLength + 1] = {0};
			// getName(name);
			// int length = strlen(name);
			// if(length >= 1){
				// name[length-1] = 0;
				// setName(name);
				// scrn::pattName.update(name);
			// }
		// }

	//Complex
		contents_t pattern_t::getChild (position pos, contentsType typeToFind) const {//Based on createChild
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

		contents_t pattern_t::createChild (position pos, contentsType typeToMake) {
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

		void pattern_t::checkContentsAndPotentiallyDestroyBlock(position pos, contents_t noteBlock) {
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

		void pattern_t::addOrRemoveNote(note noteToAdd, position pos)  {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = createChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks
			bool rem = noteBlock.addOrRemoveNoteFromBlock(noteToAdd);

			if(rem){checkContentsAndPotentiallyDestroyBlock(pos, noteBlock);}
		}

		void pattern_t::removeNote(note noteToRemove, position pos)  {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = getChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks or nir block on the step
			bool rem = noteBlock.removeNoteFromBlock(noteToRemove);
			if(rem){checkContentsAndPotentiallyDestroyBlock(pos, noteBlock);}
		}

		void pattern_t::addOrUpdateNote(note noteToAdd, position pos)  {//CAlled by pattern in sequencing.h
			if(!isValid()){return;}
			contents_t noteBlock = createChild(pos, contentsType::note); //Get or create the relevant block
			if(!noteBlock.isValid()){return;}//No more blocks
			noteBlock.addOrUpdateNoteFromBlock(noteToAdd);
		}

		void pattern_t::clearStepAndSubstepOfNotes(position pos)  {
			if(!isValid()){return;}
			for(int i=0; i<blocksPerBlock; i++){
				contents_t blockToClear = getChild(pos, contentsType::note);
				if(!blockToClear.isValid()){continue;}
				blockToClear.destroy();
				checkContentsAndPotentiallyDestroyBlock(pos, blockToClear);
			}
		}

		void pattern_t::clearStepOfNotes(position pos) {
			if(!isValid()){return;}
			contents_t blockToClear = getChild(pos, contentsType::note);
			if(!blockToClear.isValid()){return;}
			blockToClear.emptyNotes();
			checkContentsAndPotentiallyDestroyBlock(pos, blockToClear);
		}

		noteList pattern_t::getNotesOnStep(position pos)  const{
			if(!isValid()){return {};}
			contents_t noteBlock = getChild(pos, contentsType::note);
			return noteBlock.isValid() ? noteBlock.getNotesInBlock() : noteList();
		}

		void pattern_t::clearStepAndMaybeEraseParent(position pos)  {
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

		std::vector<uint8_t> pattern_t::getPitchesOnStep(position pos, const bool searchSubstepInsteadOfStep, const bool searchSubstepsWithinStep)  {
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
		std::vector<uint8_t> pattern_t::getStepsFromNote(const note pitchToFind, const uint8_t bar, const int8_t step/*-1 for search within bar*/, const bool searchSubstepsToo)  {
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

		savedNote pattern_t::getNoteOnStep(const position pos, const note noteToFind)  {
			if(!isValid()){return {};}
			contents_t noteBlock = getChild(pos, contentsType::note);
			return noteBlock.isValid() ? noteBlock.getNoteInBlock(noteToFind) : savedNote();
		}

		void pattern_t::copyStep(pattern_t sourcePattern, const position pos, const position destinationPos, const bool clear)  {//Two copies using a buffer
			//Maybe check if buffer is needed? Doesn't really matter but would be good

			//length_t wrapPos = wrap ? getLength() : length_t{true};
			if(clear){clearStepAndMaybeEraseParent(destinationPos);}
			noteList notesOnStep = sourcePattern.getNotesOnStep(pos);
			for(auto n : notesOnStep){
				addOrUpdateNote(n, destinationPos);
			}
			
		}

		void pattern_t::copyFromPattern(const pattern_t sourcePattern)  {
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

		void pattern_t::quantiseStep(const position pos, const uint8_t quantiseLevel, const bool wrapQuantise)  {
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
		std::array<bool, gc::keyNum::notes> pattern_t::getNoteLeds(const uint8_t startPitch, const position pos, const bool isInSubstep, const bool searchSubstepsInStep)  {
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
		std::array<bool, gc::keyNum::steps> pattern_t::getStepLeds(const uint8_t pitch, const int bar, const int step, const bool searchSubstepToo)  {
			std::array<bool, gc::keyNum::steps> returnLEDs = {false};
			if(!isValid()){return returnLEDs;}
			auto steps = getStepsFromNote({pitch}, bar, step, searchSubstepToo);
			for (const auto step : steps){
				returnLEDs[step] = true;
			}
			return returnLEDs;
		}

		void pattern_t::copyHeader(pattern_t otherPatt)  {
			if(!isValid() || !otherPatt.isValid()){return;}
			memcpy(getPointer(), otherPatt.getPointer(), patternHeaderSize);
		}

		//Saving and loading
		void pattern_t::saveToCard(){
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
//};//End pattern_t

//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
//------------------------------------------------------------------------------------------FINAL STUFF-----------------------------------------------------------------------
bool doesPatternExist(pattern_t patt){
	return patt.getType() == blocks::blockType::pattern;
}
bool doesPatternExist(uint32_t patt){
	return pattern_t(patt).getType() == blocks::blockType::pattern;
}

void forEachPattern(std::function<void(pattern_t&)> func){
	blocks::forEachBlock([func](blocks::block_t block){
		if(block.getType() == blocks::blockType::pattern){
			auto patt = pattern_t{static_cast<uint32_t>(block.getAddress())};
			func(patt);
		}
	});
}

}//End namespace