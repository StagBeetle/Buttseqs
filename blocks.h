#pragma once

#include <stdint.h>
#include <functional>
	
namespace blocks{
	
	extern const uint16_t addressSize; //2 bytes for pointing to the pointer blocks
	extern const uint16_t numberOfBlocks;//4000 //59918; This includes an additional 1 for the temp pattern buffer which is used when copying to ensure the sections don't overlap (same issue as memcpy)
	extern const uint16_t zeroPointerReplacer;
	
	//Other
	enum class blockType{ //Max allowable is 16
		none	= 0,
		pattern	= 1,
		contents	= 2,
		arrange	= 3,
		tempoTime	= 4,
		mute	= 5,
		patternSet	= 6,
		process	= 7,
		other	= 8,
		max	= 9,
	};
	
	const char* const blockTypeNames[static_cast<int>(blockType::max)] = {
		"none"	,
		"pattern"	,
		"contents"	,
		"arrange"	,
		"tempoTime"	,
		"mute"	,
		"patternSet"	,
		"process"	,
		"other"	,
	};
	
	//uint32_t getNextFreeBlock(const blockType type);	
	//------------------------------------------- block_t:
	
	class block_t{ //This only holds an address to the data which is in a malloc - maybe it should also contain the actual data
		protected:
			static uint8_t size;
			//static uint8_t* memoryOffset;
			uint16_t address;// = zeroPointerReplacer;
		public:
			/**
			 * Does the address point to a valid
			 */
			bool isValid() const ;
			uint16_t getAddress() const;
			uint8_t* getPointer() const ;
			//Constructors:
			/**
			 * Create a block and add it to memory
			 */
			block_t(blockType type);
			/**
			 * a Copy of a block
			 */
			block_t(const block_t& other) = default;
			/**
			 * An invalid block
			 */
			block_t();
			/**
			 * a Copy of a block
			 */
			block_t(uint16_t ref);
			/**
			 * Clear the block, but not necssarily all references
			 */
			void destroy()  ;
			
			//Not constructors:
			void invalidate() ;
			/**
			 * a Copy of a block
			 */
			blockType getType()  ;
			const char* getTypeName()  ;
			void setType(const blockType type)  ;
			bool isFree()  ;
			void printContents()  ;
			void writeData(const int pos, const int size, const uint8_t* data)  ;
			void readData(const int pos, const int size, uint8_t* data)  ;
	};//end of block_t

	
//<!---------------------------------------------------------- Miscellaneous block functions

bool operator == (const  block_t one, const  block_t other);
bool operator != (const  block_t one, const  block_t other);

uint32_t getNextFreeBlock(const blockType type);
uint32_t getLastBlockOfType(const blockType type);
void forEachBlock(std::function<void(block_t&)> func);
uint32_t getMaxBlockNum();
uint32_t getLastPatternBlock();

}