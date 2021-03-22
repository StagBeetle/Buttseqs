#ifndef processes_h
#define processes_h

#include <stdint.h>
#include <algorithm>
#include "blocks.h"	
#include "patternStorage.h"	//How the patterns are saved in memory
	
namespace process{
	const int parameterOffset = 1; //ID comes first
	const int spaceForParameters = 35 - parameterOffset; //In order to constrain to the block size that note blocks use, we take the space for the ID from the pattern block size
	const int numberOfParameters = 16;
	
	class processStack;
	
	typedef std::array<uint8_t, spaceForParameters> dataArrayForConstruction;
	
	class dataArray{
		//std::array<uint8_t, spaceForParameters> data;
		const uint16_t blockID;
	public:
		dataArray(const int c_blockID) : 
			blockID(c_blockID) {}
		
		//Bit of a hack:
		uint8_t& operator [](size_t index) {
			return *(blocks::block_t{blockID}.getPointer()+parameterOffset+index);
		}
	};
	
	typedef std::array<const char*, numberOfParameters> nameArray;
	typedef patMem::noteList (*procFunc_t) ( processStack*, patMem::position, dataArray);
	
	class processType {
		private:
			//const uint8_t processId;
			const char*	name	;
			const uint8_t 	maxLatency	; //For calculating something mayve later
			const procFunc_t	processFunc	; //how it works
			dataArrayForConstruction	initialisers	; //How the block memory should be initialised (after the process ID + steps)
			const uint8_t	numInitialisers	; //How many initialisers;
			nameArray	parameterNames	; //The name of the parameters
		public:
			processType(
				const char*	c_name	,
				const uint8_t 	c_maxLatency	,
				const procFunc_t	c_processFunc	,
				const std::vector<uint8_t>	c_initialisers	,
				const std::vector<const char*>	c_parameterNames	
				) : 
				name	(c_name	),
				maxLatency	(c_maxLatency	),
				processFunc	(c_processFunc	),
				numInitialisers	(c_initialisers.size()	){
		
					std::copy_n(c_initialisers.begin(), numInitialisers, initialisers.begin());
					std::copy_n(c_parameterNames.begin(), c_parameterNames.size(), parameterNames.begin());
				}
			patMem::noteList getNotesOnStep( processStack* stack, patMem::position pos, const dataArray data) const  {
				patMem::noteList notes = processFunc(stack, pos, data);
				return notes;
			}
			procFunc_t getProcessFunc() const {
				return processFunc;
			}
			const char* getName() const {
				return name;
			}
			dataArrayForConstruction getInitialisers() const {
				return initialisers;
			}
			uint8_t getNumOfInitialisers() const {
				return numInitialisers;
			}
	};
	
	const extern processType processes[];
	
	// patMem::noteList getNotesOnStep( processStack* stack, patMem::position pos, const dataArray data) const  {
		// return processFunc(stack, pos, data);
	// }
	
	class process_t : public blocks::block_t{
		private:
			//Store these in block or somethin: 35 bytes
			// uint8_t processId;	 0	× 1
			// std::bitset<256> relevantSteps;	 1	× 8
			// uint8_t parameterValues[spaceForParameters];	 9	× remaining

			
		public:
			process_t(processType& type) : 
				blocks::block_t(blocks::blockType::process)
			{}
			
			process_t (const  process_t& other){address = other.getAddress();}
			
			process_t (const  process_t&& other){address = other.getAddress();}
			
			process_t& operator = (const  process_t other){
				address = other.getAddress();
				return *this;
			}
			
			process_t() : block_t(){}
			
			process_t(const int c_processID) : block_t(blocks::blockType::process) {
				initialise(c_processID);
			}
			
			 process_t& operator = (const  int processID)  {
				block_t(blocks::blockType::process);
				initialise(processID);
				return *this;
			}
				
			void initialise(const int ID) ;
			int getProcessID() const ;
			dataArray getParams() const ;
			//void writeParams(const dataArray data) ;
			patMem::noteList getNotesOnStep( processStack* stack, patMem::position pos, const dataArray data) ;
	};

class processStack {
		 process_t processList[Sequencing::processesPerTrack];
		 int numberOfProcesses = 0; 
		 int position = 0; //Position for going along the stack
		const int trackNum = 0;
		static int trackEnumerator;
		//const Sequencing::track* parentTrack;
	public:
		processStack() : trackNum(trackEnumerator){
			trackEnumerator++;
		}
		 process_t getLastStage() ;
		 process_t getThisStage()  const;
		 Sequencing::track& getTrack() ;
		void removeProcess(const int p) ;
		void addProcess(const int p, const int index) ;
		void reorderProcesses(const int first, const int second) ;
		void decrementPosition() ;
		patMem::noteList getNotesOnStep(patMem::position pos) ;
		process_t getProcess(const int p) ;
		int getNumProcesses() const ;
};



} //end namespace
#endif