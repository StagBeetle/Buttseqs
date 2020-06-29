#ifndef blocks_h
#define blocks_h
	
namespace blocks{
	const uint16_t addressSize = 2; //2 bytes for pointing to the pointer blocks
	const uint16_t numberOfBlocks = 1000;//4000 //59918; This includes an additional 1 for the temp pattern buffer which is used when copying to ensure the sections don't overlap (same issue as memcpy)
	
	//Block sizes:
	const uint16_t patternBlockSize = patMem::patternBlockSize; // 25 - The pattern heaeder data and the block
	const uint16_t contentsBlockSize = patMem::contentsBlockSize;
	const uint16_t arrangeBlockSize = arrangement::arrangeBlockSize;
	
	//For all:
	const uint16_t blockSize = max(max(patternBlockSize, contentsBlockSize), arrangeBlockSize);

	//Other
	enum class blockType{
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
	
	//const int bitsForBlockInfo = 4;
	uint8_t blockData[numberOfBlocks/2] = {0};//Hold the type/existence of all of the blocks
	//std::bitset<numberOfBlocks> usedBlocks;
	long int spaceToAllocate = numberOfBlocks * blockSize;
	uint8_t* blockStart = static_cast<uint8_t*>(calloc(spaceToAllocate, 1)); //105000

	//For control:
	const uint16_t zeroPointerReplacer = 65535;
	
	//----//-----------------
	//---//------------------
	//--//------------------- FUNCTIONS
	//-//--------------------
	////---------------------
	
	uint8_t MSBitmask = 240;
	uint8_t LSBitmask = 15;
	
	uint32_t getNextFreeBlock(const blockType type);	
	//------------------------------------------- block_t:
	
	class block_t{ //This only holds an address to the data which is in a malloc - maybe it should also contain the actual data
		protected:
			static uint8_t size;
			static uint8_t* memoryOffset;
			volatile uint16_t address = zeroPointerReplacer;
		public:

			bool isValid() const volatile{
				return address != zeroPointerReplacer;
			}

			uint16_t getAddress() const volatile{
				return address;
			}

			uint8_t* getPointer() const volatile{
				return memoryOffset + (address * size);
			}

			//Constructors:
			block_t(blockType type) {
				int32_t nextFreeBlockNum = getNextFreeBlock(type);//Get the next empty block
				if(nextFreeBlockNum == -1){
					address = zeroPointerReplacer;
					return;
				} else {
					address = nextFreeBlockNum;
				}
			}
			
			block_t(const block_t& other) = default;

			block_t(){//Invalid object
				address = zeroPointerReplacer;
			}

			block_t(uint16_t ref){
				address = ref;
			}

			void destroy() volatile {//Clear
				setType(blockType::none);
				address = zeroPointerReplacer;
			}
			
			//Not constructors:
			
			void invalidate() volatile {//Invalidate the reference but do not affect the pointer
				address = zeroPointerReplacer;
			}

			blockType getType() volatile {
				uint8_t dualBlock = blockData[address/2];
				bool isEven = address % 2 == 0;
				uint8_t data = isEven ? dualBlock >> 4 : dualBlock & LSBitmask;
				return static_cast<blockType>(data);
			}
			
			const char* getTypeName() volatile {
				blockType type = getType();
				return type == blockType::none ? "----" : blockTypeNames[static_cast<int>(getType())];
			}
			
			void setType(const blockType type) volatile {
				uint8_t& dualBlock = blockData[address/2];
				bool isEven = address % 2 == 0;
				dualBlock = isEven ? 
					(dualBlock & LSBitmask) + (static_cast<uint8_t>(type) << 4) : 
					(dualBlock & MSBitmask) + static_cast<uint8_t>(type);
			}
			
			bool isFree() volatile {
				return (getType() == blockType::none);
			}

			void printContents() volatile {
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
			
			void writeData(const int pos, const int size, const uint8_t* data) volatile {
				memmove(getPointer()+pos, data, size);
			}
			void readData(const int pos, const int size, uint8_t* data) volatile {
				memmove(data, getPointer()+pos, size);
			}
	};//end of block_t

	uint8_t* block_t::memoryOffset	= blockStart;
	uint8_t block_t::size	= blockSize;
	
//<!---------------------------------------------------------- Miscellaneous block functions

bool operator == (const volatile block_t one, const volatile block_t other){
	return one.getAddress() == other.getAddress();
}

bool operator != (const volatile block_t one, const volatile block_t other){
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

}
#endif