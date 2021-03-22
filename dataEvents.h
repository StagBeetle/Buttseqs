#pragma once
//How are they modified by the user - what is their functionality (cc mode)
//screen:
	//Show the 16 steps across
	//show a list of what is on each step
//buttons:
	//step jumps to steps
	//Note butons affect what data buttons do@
		//Change dataBlockType
		//Set CC/NRPN//exec
		//For sysex, just type bytes on data pad
		//delete byte
//Data blocks are how cc, (N)RPN, sysex, and arbitrary code execution
//control byte + 32 bytes + next byte
//how output them(easy) 

//Be careful with precision of MSB/LSB RPNs - Should the value be saved as MSB + LSB or together? Let the user decide how to send things?

//When to execute functions? Probably delay them with a scheduled event, or perhaps immediately 

// class pattern_t {
	// std::vector<dataEvent> getEventsOnStep(const pos step, const int includeSubstep = false);
	
// }

//Have a way to add mu;ltiple events in a line or curve

//How does pattern storage deal with these?
#include "blocks.h"


namespace dataEvents{
	
	enum class dataEventType : uint8_t {
		none	,
		cc	, //2 bytes
		NRPN	, //4 bytes
		sysex	, //34 bytes
		exec	, //6 bytes
		process	, //4 bytes (probably)
	};
	
	// class dataEvent{//This is not saved but an intermediate format for transporting data so space is not too important
		// dataEventType m_type;
		// uint16_t property; //cc / NRPN/RPN combo memoryLocation/functionIndex
		// uint16_t value[4];//Four spaces to allow multiple parameters for a function. This is just a placeholder for now as some functions might take more than 8 bytes of data
	// }
	
//Storage classes so they need to be compact
struct cc{//The data needed to change a CC:
	uint8_t number;
	uint8_t value;
	};

/* Also for RPNs*/
struct NRPN{
	uint8_t high; //7 bits
	uint8_t low;//7 bits
	uint16_t value;//14 bits
	public:
		bool isNPRN();//Encoded within the first bit of high byte
		bool uses14bits();//encoded within the first bit of value
		uint8_t getAddressMSB();
		uint8_t getAddressLSB();
		uint16_t getValue();
};

struct sysex{
	uint8_t bytes[32];
	blocks::block_t nextBlock;
};
	
struct exec{//Take a pointer, hope it aligns right, and do an operation on it.
	//checkSafety against a list of known good functions
	uint16_t functionIndex;
	uint16_t value[4];
};

struct processChange{//Take a pointer, hope it aligns right, and do an operation on it.
	//checkSafety against a list of known good functions
	uint8_t type;
	uint8_t index; //If there are multiple of the same type
	uint16_t value;
};

}