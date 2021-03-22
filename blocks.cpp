#include "forwarddec.h"
#include "blocks.h"
#include "notifications.h"
	
namespace blocks{
	const uint16_t addressSize = 2; //2 bytes for pointing to the pointer blocks
	const uint16_t numberOfBlocks = 40000;//4000 //59918; This includes an additional 1 for the temp pattern buffer which is used when copying to ensure the sections don't overlap (same issue as memcpy)
	const uint16_t zeroPointerReplacer = 65535;
	
	// //Block sizes:
	// const uint16_t patternBlockSize = patMem::patternBlockSize; // 25 - The pattern heaeder data and the block
	// const uint16_t contentsBlockSize = patMem::contentsBlockSize;
	// const uint16_t arrangeBlockSize = arrangement::arrangeBlockSize;
	
	//For all:
	const int blockSize = 35;//max(max(patternBlockSize, contentsBlockSize), arrangeBlockSize);
	
	//const int bitsForBlockInfo = 4;
	/*EXTMEM*/ uint8_t blockData[numberOfBlocks/2] = {0};//Hold the type/existence of all of the blocks
	//std::bitset<numberOfBlocks> usedBlocks;
	//long int spaceToAllocate = numberOfBlocks * blockSize;
	//uint8_t* blockStart = static_cast<uint8_t*>(calloc(spaceToAllocate, 1)); //105000

	//Bit hacky and inconsistent
	struct blockInMemory{
		uint8_t data[blockSize];
	};
	
	EXTMEM blockInMemory memoryBlocks[numberOfBlocks];
	
	//----//-----------------
	//---//------------------
	//--//------------------- FUNCTIONS
	//-//--------------------
	////---------------------
	
	uint8_t MSBitmask = 240;
	uint8_t LSBitmask = 15;
	
	uint32_t getNextFreeBlock(const blockType type);	
	//------------------------------------------- block_t:

			bool block_t::isValid() const {
				return address != zeroPointerReplacer;
			}

			uint16_t block_t::getAddress() const {
				return address;
			}

			uint8_t* block_t::getPointer() const {
				return &(memoryBlocks[address].data[0]);
			}

			//Constructors:
			block_t::block_t(blockType type) {
				int32_t nextFreeBlockNum = getNextFreeBlock(type);//Get the next empty block
				if(nextFreeBlockNum == -1){
					address = zeroPointerReplacer;
					return;
				} else {
					address = nextFreeBlockNum;
				}
			}

			block_t::block_t(){//Invalid object
				address = zeroPointerReplacer;
			}

			block_t::block_t(uint16_t ref){
				address = ref;
			}

			void block_t::destroy()  {//Clear
				setType(blockType::none);
				address = zeroPointerReplacer;
			}
			
			//Not constructors:
			
			void block_t::invalidate()  {//Invalidate the reference but do not affect the pointer
				address = zeroPointerReplacer;
			}

			blockType block_t::getType()  {
				uint8_t dualBlock = blockData[address/2];
				bool isEven = address % 2 == 0;
				uint8_t data = isEven ? dualBlock >> 4 : dualBlock & LSBitmask;
				return static_cast<blockType>(data);
			}
			
			const char* block_t::getTypeName()  {
				blockType type = getType();
				return type == blockType::none ? "----" : blockTypeNames[static_cast<int>(getType())];
			}
			
			void block_t::setType(const blockType type)  {
				uint8_t& dualBlock = blockData[address/2];
				bool isEven = address % 2 == 0;
				dualBlock = isEven ? 
					(dualBlock & LSBitmask) + (static_cast<uint8_t>(type) << 4) : 
					(dualBlock & MSBitmask) + static_cast<uint8_t>(type);
			}
			
			bool block_t::isFree()  {
				return (getType() == blockType::none);
			}

			void block_t::printContents()  {
				lgc("block ");
				lgc(getAddress());
				lgc(" - ");
				lg(getTypeName());
				for(int i = 0; i < blockSize; i++){
					lgc(" ");
					lg(*(getPointer()+i));
				}
				lg("");
			}
			
			void block_t::writeData(const int pos, const int size, const uint8_t* data)  {
				memmove(getPointer()+pos, data, size);
			}
			void block_t::readData(const int pos, const int size, uint8_t* data)  {
				memmove(data, getPointer()+pos, size);
			}


	// uint8_t* block_t::memoryOffset	= blockStart;
	uint8_t block_t::size	= blockSize;
	
	//<!---------------------------------------------------------- Miscellaneous block functions

	bool operator == (const  block_t one, const  block_t other){
		return one.getAddress() == other.getAddress();
	}

	bool operator != (const  block_t one, const  block_t other){
		return one.getAddress() != other.getAddress();
	}

	uint32_t getNextFreeBlock(const blockType type){
		for(uint16_t i = 0; i<numberOfBlocks; i++){
			block_t possibleBlock = block_t(i);
			if(possibleBlock.isFree()){
				possibleBlock.setType(type);
				return i;
			}
		}
		notifications::memoryFull.display();
		return -1;
	}

	uint32_t getLastBlockOfType(const blockType type){
		for(uint16_t i = numberOfBlocks-1; i>= 0; i--){
			block_t possibleBlock = block_t(i);
			if(possibleBlock.getType() == type){
				return i;
			}
		}
		return -1;
	}

	void forEachBlock(std::function<void(block_t&)> func){
		for(uint16_t b = 0; b < numberOfBlocks; b++){
			//memoryBlocks
			auto block = block_t{b};
			if(block.isValid()){
				func(block);
			}
		}
	}


	uint32_t getMaxBlockNum(){
		return numberOfBlocks-1;
	}


	uint32_t getLastPatternBlock(){
		for(unsigned int i = getMaxBlockNum(); i>= 0; i--){
			block_t testblock(i);
			if(testblock.getType() == blockType::pattern){
				return i;
			}
		}
		return 0;
	}

}