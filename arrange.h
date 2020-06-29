#ifndef arrange_h
#define arrange_h

// Work on seamless mix between pattern loop and arrange
	// Normal loop adds the patterns at start of arrange - adjust end loop point based on longest loop
	
namespace arrangement{
	const uint16_t addressSize = blocks::addressSize;
	const uint16_t bitMaskSize = 4; //2^3 = 8
	const uint16_t bitMaskNumber = B00001111;
	const uint16_t maxDepth = 3;
	      
	const uint16_t addressesPerNode = 16;
	const uint16_t spaceForNodes = (addressesPerNode * addressSize);	//The size to store the nodes
	const uint16_t spaceForPatternOrNot = (addressesPerNode + 7) / 8 ;	//The size to store whether it is a pattern or not - this could be done by depth instead
	const uint16_t arrangeBlockSize = spaceForNodes + spaceForPatternOrNot; //34
	const uint16_t maxPosition = utl::power(addressesPerNode, maxDepth); //One more than the actual max
	const uint16_t zeroPointerReplacer = blocks::zeroPointerReplacer;
	
		//(addressesPerNode * utl::getPowerOfTwo(maxDepth)) / 8;	//The size to store the level
			// = 19
	
	const int indexMultipliers[maxDepth] = {256, 16, 1};

	enum class nodeType{
		node	,
		pattern	,
	};
	
	class node : public blocks::block_t{
		private:
			nodeType nodePattType = nodeType::node; //Obtained from parent;
		public:
			node(const uint16_t p_address, const bool isPattern) : 
				blocks::block_t(p_address), 
				nodePattType(isPattern ? nodeType::pattern : nodeType::node)
				{}
			
			node(const bool isPattern){
				// lg("nodeConstr1");
				int32_t nextFreeNode = blocks::getNextFreeBlock(blocks::blockType::arrange);
				// lg(nextFreeNode);
				if(nextFreeNode == -1){return;}
				else{
					address = nextFreeNode;
				}
				// lg("nodeConstr2");
				initialise();
				// lg("nodeConstr3");
				nodePattType = isPattern ? nodeType::pattern : nodeType::node;
				// lg("nodeConstr4");
			}
			
			node() : blocks::block_t() {}
			
			bool isValid() const{
				return address != zeroPointerReplacer && !isPattern();//It can represent no pattern
			}
			
			void initialise(){
				// lg((long int)getPointer());
				// lg(address);
				// lg(size);
				// lg(*(getPointer()));
				// lg(address);
				// lgc("contents: ");
				// for(int i=0; i<size; i++){
					// lgc(*(getPointer()+i));
					// lgc(" ");
				// }
				// lg(spaceForNodes + spaceForPatternOrNot);
				//memset(getPointer()	, 255	, spaceForNodes	);
				// lg("A");
				memset(getPointer()	, 255	, size	);
				// lg("B");
				memset(getPointer() + spaceForNodes	, 0	, spaceForPatternOrNot	);
				// lg("C");
			}
			
			bool isValidPattern() const{
				return address != zeroPointerReplacer;
			}
			
			node getNodeAtPosition(const int position) const {
				//assert(position < addressesPerNode);
				// lg(address);
				// lg(1);
				// lg((long)getPointer());
				// lg((long)pointerToPosition);
				// lg(*pointerToPosition);
				// lg(*(pointerToPosition+1));
				// lg(2);
				uint8_t* pointerToPosition = getPointer() + position * addressSize;
				uint16_t address = ((*pointerToPosition) << 8) + (*(pointerToPosition+1));
				// lg(3);
				uint8_t secondByteOffset = position >= 8 ? 1 : 0;
				uint8_t* pointerToType = getPointer() + spaceForNodes + secondByteOffset;
				bool isPattern = bitRead(*pointerToType, position%8);
				// lg(4);
				return {address, isPattern};
			}

			void setNodeAtPosition(const int position, const node child){
				// lg(1);
				uint16_t address = child.getAddress();
				// lg(2);
				bool isPattern = child.isPattern();
				// lg(3);
				uint8_t* pointerToPosition = getPointer() + (position * addressSize);
				// lg(4);
				*pointerToPosition = address >> 8;
				*(pointerToPosition+1) = address & 255;
				// lg(5);
				uint8_t secondByteOffset = position >= 8 ? 1 : 0;
				uint8_t* pointerToType = getPointer() + spaceForNodes + secondByteOffset;
				// lg(*pointerToType);
				bitWrite(*pointerToType, position%8, isPattern);
				// lg(*pointerToType);
				// lg(6);
			}
			
			bool isPattern() const{
				return nodePattType == nodeType::pattern;
			}
			
			// bool isEmpty() const{
				// return address == zeroPointerReplacer;
			// }
			
			const char* showType(){
				return nodePattType == nodeType::pattern ? "patt" : "node";
			}
			
			patMem::pattern_t getPattern() const{
				return {address};
			}
			
			void printContents(){
				return;
				lgc("node: ");
				lgc(getAddress());
				lgc(" ");
				lg(isPattern() ? "patt" : "node");
				if(!isPattern() && isValid()){
					for(int j = 0; j<addressesPerNode; j++){
						node nodeAtPos = getNodeAtPosition(j);
						lgc(" ");
						lgc(nodeAtPos.getAddress());
						lgc(" ");
						lg(nodeAtPos.isPattern() ? "patt" : "node");
					}
				}
				//remlog-lg();
			}
			
			friend bool operator == (const node& one, const node& other);
			
			node& operator = (const node& other){
				address = other.address ; 
				nodePattType = other.nodePattType ;
				return *this;
			}
	};

	bool operator == (const node& one, const node& other){
		return
			one.address == other.address && 
			one.nodePattType == other.nodePattType ;
	}
	
	bool operator != (const node& one, const node& other){
		return !(one == other);
	}

	typedef std::array<int, maxDepth> nodeIndex;

	nodeIndex getIndexes(const int timelinePosition, const bool logging = false){
		nodeIndex output;
		if(logging){
			//remlog-lg("getIndex");
		}
		int position = 0;
		for(int i = bitMaskSize * (maxDepth-1) ; i >= 0; i = i-bitMaskSize){//8, 4, 0
			int value = (timelinePosition & (bitMaskNumber << i)) >> i;
			if(logging){
				//remlog-lgc("  pos:");
				//remlog-lgc(position);
				//remlog-lgc("i:");
				//remlog-lgc(i);
				//remlog-lgc("index:");
				//remlog-lg(value);
			}
			output[position] = value;
			position++;
		}
		return output;
	}
	
	int getPositionFromIndexes(nodeIndex indexes, const int depth){
		int position = 0;
		for(int i=0; i<depth+1; i++){
			position += indexes[i] * indexMultipliers[i];
		}
		return position;	
	}
	
	void printIndexes(nodeIndex index){
		for(int i = 0; i< maxDepth; i++){
			//remlog-lg(index[i]);
		}
	}

	// bool areAllNextIndexesZero(const nodeIndex indexes, const int initialIndex){
		// for(int i = initialIndex; i<maxDepth; i++){
			// if(indexes[i] != 0){return false;}
		// }
		// return true;
	// }
	
	// int getNextNonZeroIndexPosition(const nodeIndex indexes, const int initialIndex){
		// for(int i = initialIndex; i<maxDepth; i++){
			// if(indexes[i] != 0){return i;}
		// }
		// return -1;
	// }
	
	// bool mustNextNodeBePattern(const nodeIndex indexes, const int initialIndex){ 
		// //See how many of the next indexes are not zero.
		// //Should always be >=1 based on where this is called;
		// //If only one of the next indexes is not zero, then that pattern can be made
		// int numOfIndexes = 0;
		// for(int i = initialIndex; i<maxDepth; i++){
			// if(indexes[i] != 0) {numOfIndexes++;}
		// }
		// return numOfIndexes == 1;
	// }

	const int numberOfArrangements = 64;
	
	// bool isDoingRecursion = false;
	// long int timeAtStart;
	
	// void endTime(){
		// isDoingRecursion = false;
		// long int timeAtEnd = micros();
		// long int timePassed = timeAtEnd - timeAtStart;
		// if(timePassed > 3){
			// lgc("t");
			// lg((int)timePassed);
		// }
	// }

	class arrangeChannel{
		public:
			patMem::pattern_t getCurrentPattern (const int timelinePosition){
				//timeAtStart = micros();
				nodeIndex indexes = getIndexes(timelinePosition);
				node relevantBlock = getFirstPattern();
				
				//node nodeList[maxDepth+1]; //+1 just in case - probably not necessary;
				
				if(relevantBlock.isPattern()){
					//endTime();
					//patMem::pattern_t tmp = relevantBlock.getPattern();
					return relevantBlock.getPattern();
				}
				relevantBlock.printContents();
				
				relevantBlock = relevantBlock.getNodeAtPosition(indexes[0]);
				for(int depth=0; depth<maxDepth; depth++){
					//relevantBlock.printContents();
					if(relevantBlock.isPattern()){
						//remlog-lg("retIsP");
						//endTime();
						return relevantBlock.getPattern();
					}
					else if(!relevantBlock.isValid()){//Not a pattern but an empty node
						//remlog-relevantBlock.printContents();
						//endTime();
						return {};
					}
					
					if(depth < maxDepth-1){
						relevantBlock = relevantBlock.getNodeAtPosition(indexes[depth+1]);
					}
					//Node
				}
				lg("ERROR getCurrPatt");
				//endTime();
				return {};
			}
			
			void addPattern (const int timelinePosition, const patMem::pattern_t patt){//Check indexes
				// lgc("addP:");
				// lg(timelinePosition);
				//remlog-lgc("addPattern ");
				//remlog-lgc(patt.getAddress());
				//remlog-lgc(" at ");
				//remlog-lg(timelinePosition);
				// lg("b");
				if(timelinePosition >= maxPosition){return;}
				node parentBlock = {};
				node currentBlock = getFirstPattern();
				nodeIndex indexes = getIndexes(timelinePosition, false);
				// lg("c");
				node nodeList[maxDepth+1]; //+1 just in case - probably not necessary;
				
				node patternToAdd  = {patt.getAddress(), true};
				// lg("eoxn");
				//bool allNextIndexesZero = areAllNextIndexesZero(indexes, 0);
				bool isPatternBlock = currentBlock.isPattern();
				
				//If any patterns are encountered, they must be added later after nodes are added:
				int insertPatternAtPositions[4] = {0};
				node insertThesePatterns[4] = {};
				int numberOfPatternsToInsert = 0;
				// lg("d");
				//Deal with the first pattern in the arrangement:
				if(isPatternBlock && timelinePosition == 0){
					// lg("e1");
					setFirstPattern(patternToAdd);
					return;
				}
				else if(isPatternBlock){
					// lg("e2");
					node nextBlock = {false};
					if(!nextBlock.isValid()){
						// lg("f1");
						return;
					}
					// lg("f2");
					setFirstPattern(nextBlock);
					insertPatternAtPositions[numberOfPatternsToInsert] = 0;
					insertThesePatterns[numberOfPatternsToInsert] = currentBlock;
					numberOfPatternsToInsert++;
					currentBlock = nextBlock;
					//depth-1; //Not even in loop yet - hold your horses
					nodeList[0] = nextBlock;
				}
				else{
					// lg("e3");
					nodeList[0] = currentBlock;
				}
				// lg("header");
				
				//Deal with any of the others
				for(int depth=0; depth<maxDepth; depth++){
					int relevantIndex = indexes[depth];
					//bool allNextIndexesZero = areAllNextIndexesZero(indexes, depth);
					bool isAtPatternDepth = depth == maxDepth-1;
					bool isPatternBlock = currentBlock.isPattern();
					// lgc("depth: "); lg(depth);
					// lgc("pos: "); lg(relevantIndex);
					// lgc("type: "); lg(isPatternBlock ? "patt" : "node");
					
					bool shouldBreak = false;
					
					if(isAtPatternDepth && isPatternBlock){//Right block:
						parentBlock.setNodeAtPosition(relevantIndex, patternToAdd);
						//remlog-lg("added");
						currentBlock = patternToAdd; //Just for the nodeList
						//break;
						shouldBreak = true;
					} 
					else if (isPatternBlock){//Next indexes not zero
						node nextBlock = {false};
						//remlog-lg("pbsnap");
						parentBlock.setNodeAtPosition(relevantIndex, nextBlock);
						//remlog-lg("nbsnap");
						//nextBlock.setNodeAtPosition(0, currentBlock);
						
						
						//remlog-lgc("addedPattern at ");
						//remlog-lgc(numberOfPatternsToInsert);
						//remlog-lgc(" while adding");
						//remlog-lg(patt.getAddress());
						int thisPosition = getPositionFromIndexes(indexes, depth);
						insertPatternAtPositions[numberOfPatternsToInsert] = thisPosition;
						insertThesePatterns[numberOfPatternsToInsert] = currentBlock;
						numberOfPatternsToInsert++;
						
						currentBlock = nextBlock;
						//remlog-lg("madeNewBLockAndPutInPositionAbovethis");
					}
					else{ //Not a pattern block, indexes may or not be zero
						node nextBlock = currentBlock.getNodeAtPosition(relevantIndex);
						if (!nextBlock.isValid()){
							//allNextIndexesZero = areAllNextIndexesZero(indexes, depth+1);
							if(isAtPatternDepth){
								currentBlock.setNodeAtPosition(relevantIndex, patternToAdd);
								currentBlock = patternToAdd; //Just so it can be loggde later because the mission is already complete;
								//remlog-lg("addedPatternBlock");
								shouldBreak = true;
								//break;
							}
							else{
								nextBlock = {false}; //not a pattern
								currentBlock.setNodeAtPosition(relevantIndex, nextBlock);
								//remlog-lg("gotNextBlock");
							}
						}	
						parentBlock = currentBlock;
						currentBlock = nextBlock;
						//remlog-lg("moveToNextBlock");
					}
					nodeList[depth+1] = currentBlock;
					if(shouldBreak){break;}
				}//End loop
				
				//Show which blocks have been added:
				//remlog-lg();
				for(int i=0;i<maxDepth;i++){
					nodeList[i].printContents();
					//remlog-lg();
				}
				// lg("added: ");
				// node nodeAdded = getCurrentPattern(timelinePosition, false);
				// lgc(" ");
				// lgc(nodeAdded.getAddress());
				// lgc(" ");
				// lg(nodeAdded.isPattern() ? "patt" : "node");
				
				for(int i=0; i<numberOfPatternsToInsert; i++){
					addPattern(
						insertPatternAtPositions	[i],
						insertThesePatterns	[i].getPattern()
					);
				}
			}
			
			void deletePatternIfFound(const patMem::pattern_t patternToDelete){
				for(int i=0; i<maxPosition; i++){
					if(getCurrentPattern(i) == patternToDelete){
						removePattern(i);
					}
				}
			}
			
			node getFirstPattern(){
				return firstPattern;
			}
			
			void setFirstPattern(const node first){
				firstPattern = first;
			}
			
			void removePattern(const int timelinePosition){//FINISH THIS and check it's right
				addPattern(timelinePosition, {});
				//Must delete parent blocks
			}
		private:
			node firstPattern = {zeroPointerReplacer, true};
	};

	arrangeChannel arrangeChannels[numberOfArrangements][Sequencing::maxTracks];
}
#endif