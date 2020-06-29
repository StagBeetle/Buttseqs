#ifndef processes_h
#define processes_h

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
	typedef patMem::noteList (*procFunc_t) (volatile processStack*, patMem::position, dataArray);
	
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
			patMem::noteList getNotesOnStep(volatile processStack* stack, patMem::position pos, const dataArray data) const volatile {
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
	
	// patMem::noteList getNotesOnStep(volatile processStack* stack, patMem::position pos, const dataArray data) const volatile {
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
			
			process_t (const volatile process_t& other){address = other.getAddress();}
			
			process_t (const volatile process_t&& other){address = other.getAddress();}
			
			process_t& operator = (const volatile process_t other){
				address = other.getAddress();
				return *this;
			}
			
			process_t() : block_t(){}
			
			process_t(const int c_processID) : block_t(blocks::blockType::process) {
				initialise(c_processID);
			}
			
			volatile process_t& operator = (const volatile int processID) volatile {
				block_t(blocks::blockType::process);
				initialise(processID);
				return *this;
			}
				
			void initialise(const int ID) volatile;
			int getProcessID() const volatile;
			dataArray getParams() const volatile;
			//void writeParams(const dataArray data) volatile;
			patMem::noteList getNotesOnStep(volatile processStack* stack, patMem::position pos, const dataArray data) volatile;
	};

class processStack {
		volatile process_t processList[Sequencing::processesPerTrack];
		volatile int numberOfProcesses = 0; 
		volatile int position = 0; //Position for going along the stack
		const int trackNum = 0;
		static int trackEnumerator;
		//const Sequencing::track* parentTrack;
	public:
		processStack() : trackNum(trackEnumerator){
			trackEnumerator++;
		}
		volatile process_t getLastStage() volatile;
		volatile process_t getThisStage() volatile const;
		volatile Sequencing::track& getTrack() volatile;
		void removeProcess(const int p) volatile;
		void addProcess(const int p, const int index) volatile;
		void reorderProcesses(const int first, const int second) volatile;
		void decrementPosition() volatile;
		patMem::noteList getNotesOnStep(patMem::position pos) volatile;
		process_t getProcess(const int p) volatile;
		int getNumProcesses() const volatile;
};

int processStack::trackEnumerator = 0;

//Process_t:

void process_t::initialise(const int ID) volatile {
	memcpy(
		getPointer()+parameterOffset,
		processes[ID].getInitialisers().data(),
		processes[ID].getNumOfInitialisers()
	);
}

int process_t::getProcessID() const volatile {
	return *(getPointer()); //The first thing pointed to is the ID;
}

dataArray process_t::getParams() const volatile {
	dataArray data{address};
	return data;
}
// void process_t::writeParams(const dataArray data) volatile {
	// for(int i=0; i<spaceForParameters; i++){
		// *(getPointer()+parameterOffset) = data[i];
	// }
// }

patMem::noteList process_t::getNotesOnStep(volatile processStack* stack, patMem::position pos, const dataArray data) volatile {
	//stack->decrementPosition();
	//lgc("process_t.getNotesOnStep");
	lg(getProcessID());
	return processes[getProcessID()].getNotesOnStep(stack, pos, data);
}

//processStack:
volatile process_t processStack::getLastStage() volatile {
	decrementPosition();
	if(position == 0){
		return processList[0]; //The none process always gets directly from the track
	}
	return processList[position];
}

volatile process_t processStack::getThisStage() volatile const {
	if(position == 0){
		return processList[0]; //The none process always gets directly from the track
	}
	return processList[position];
}

volatile Sequencing::track& processStack::getTrack() volatile {
	return Sequencing::getTrack(trackNum);
}

void processStack::removeProcess(const int p) volatile {
	if(!processList[p].isValid()){
		notifications::noProcess.display();
		return;
	}
	processList[p].destroy();
	for(int i = p+1; i<Sequencing::processesPerTrack; i++){//Shift later processes forward:
		typecopy(processList[i-1], processList[i]);
	}
	numberOfProcesses--;
}

void processStack::addProcess(const int p, const int processIndex) volatile {
	if(numberOfProcesses >= Sequencing::processesPerTrack){
		notifications::noProcessSpace.display();
		return;
	}
	lg("ap1");
	for(int i = Sequencing::processesPerTrack-1; i > p; i--){//Shift later processes back:
	lg(i);
		typecopy(processList[i], processList[i-1]);
	}
	lg("ap2");
	volatile process_t newProcess = {processIndex};
	lg("ap3");
	typecopy(processList[p], newProcess);
	lg("ap4");
	numberOfProcesses++;
	lg("ap5");
}
	

void processStack::reorderProcesses(const int first, const int second) volatile {
	const volatile process_t f = processList[first];
	typecopy(processList[first], processList[second]);
	typecopy(processList[second], f);
}

void processStack::decrementPosition() volatile {
	position--;
}

patMem::noteList processStack::getNotesOnStep(patMem::position pos) volatile {
	position = numberOfProcesses;
	if(position == 0){
		return processes[0].getNotesOnStep(this, pos, {0}); //Return the none pattern. Therefore, the data does not matter so use an arbitrary number
	}
	decrementPosition();
	process_t firstProcess = processList[position];
	return firstProcess.getNotesOnStep(this, pos, firstProcess.getParams());
}

process_t processStack::getProcess(const int p) volatile{
	ASSERT(p >= 0 && p < Sequencing::processesPerTrack);
	return processList[p];
}

int processStack::getNumProcesses() const volatile{
	return numberOfProcesses;
}

} //end namespace
#endif