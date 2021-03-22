#include "processes.h"
#include "notifications.h"
#include "utility.h"
#include "compatibility.h"
#include "forwarddec.h"
namespace process{
int processStack::trackEnumerator = 0;

//Process_t:

void process_t::initialise(const int ID)  {
	memcpy(
		getPointer()+parameterOffset,
		processes[ID].getInitialisers().data(),
		processes[ID].getNumOfInitialisers()
	);
}

int process_t::getProcessID() const {
	return *(getPointer()); //The first thing pointed to is the ID;
}

dataArray process_t::getParams() const {
	dataArray data{address};
	return data;
}
// void process_t::writeParams(const dataArray data)  {
	// for(int i=0; i<spaceForParameters; i++){
		// *(getPointer()+parameterOffset) = data[i];
	// }
// }

patMem::noteList process_t::getNotesOnStep( processStack* stack, patMem::position pos, const dataArray data)  {
	//stack->decrementPosition();
	//lgc("process_t.getNotesOnStep");
	lg(getProcessID());
	return processes[getProcessID()].getNotesOnStep(stack, pos, data);
}

//processStack:
 process_t processStack::getLastStage()  {
	decrementPosition();
	if(position == 0){
		return processList[0]; //The none process always gets directly from the track
	}
	return processList[position];
}

 process_t processStack::getThisStage()  const {
	if(position == 0){
		return processList[0]; //The none process always gets directly from the track
	}
	return processList[position];
}

 Sequencing::track& processStack::getTrack()  {
	return Sequencing::getTrack(trackNum);
}

void processStack::removeProcess(const int p)  {
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

void processStack::addProcess(const int p, const int processIndex)  {
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
	 process_t newProcess = {processIndex};
	lg("ap3");
	typecopy(processList[p], newProcess);
	lg("ap4");
	numberOfProcesses++;
	lg("ap5");
}
	

void processStack::reorderProcesses(const int first, const int second)  {
	const  process_t f = processList[first];
	typecopy(processList[first], processList[second]);
	typecopy(processList[second], f);
}

void processStack::decrementPosition()  {
	position--;
}

patMem::noteList processStack::getNotesOnStep(patMem::position pos)  {
	position = numberOfProcesses;
	if(position == 0){
		return processes[0].getNotesOnStep(this, pos, {0}); //Return the none pattern. Therefore, the data does not matter so use an arbitrary number
	}
	decrementPosition();
	process_t firstProcess = processList[position];
	return firstProcess.getNotesOnStep(this, pos, firstProcess.getParams());
}

process_t processStack::getProcess(const int p) {
	ASSERT(p >= 0 && p < Sequencing::processesPerTrack);
	return processList[p];
}

int processStack::getNumProcesses() const {
	return numberOfProcesses;
}

}//end namespace