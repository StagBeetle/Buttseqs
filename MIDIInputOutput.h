#pragma once

#include <MIDI.h>

namespace MIDIports{
	
// const int numberOfInPorts = gc::numberOfMIDIIns;
// const int numberOfOutPorts = gc::numberOfMIDIOuts;
	
	void begin();
	
	enum class routingType : uint8_t{
		none = 0,
		buttons,
		activePattern,
		track,
		trackFromChannel,
		port,
		max,
	};
	
	const char* const routingTypeNames[] = {
		"none"	,
		"buttons"	,
		"active pattern"	,
		"track"	,
		"chan->track"	,
		"port"	,
	};
	
	enum class notePreviewType : uint8_t{
		none = 0,
		noteOnly,
		recordOnly,
		stoppedOnly,
		priority,
		all,
		max,
	};
	
	const char* const notePreviewNames[] = {
		"off"	,
		"note only"	,
		"record only"	,
		"stopped only"	,
		"priority"	,
		"all"	,
	};
	
	enum class recordType : uint8_t{
		none = 0,
		record,
		invert,
		all,
		max,
	};
	
	const char* const recordNames[] = {
		"none"	,
		"record"	,
		"invert"	,
		"all"	,
	};
	
	enum class MIDIPort : uint8_t {
		none	= 0,
		usb	= 1,
		port1	= 2,
		port2	= 3,
		port3	= 4,
		port4	= 5,
		port5	= 6,
		outMax	= 7,
	};
	
	const char* const MIDIPortNames[] = {
		"none"	,
		"usb"	,
		"port1"	,
		"port2"	,
		"port3"	,
		"port4"	,
		"port5"	,
	};
	
	const char* getPortName(const int num);
	
	const int numberOfRoutings = 3;
	
	struct midiInputSettings{
		midiInputSettings(const char* c_name);
		const char* name;
		
		bool receiveClock = false;
		bool receiveStartStop = false;
		bool receiveSeqControl = false;
		bool receiveSSP = false;
		
		//inputType input = inputType::buttons;
		recordType recordMode = recordType::record;
		routingType routings[numberOfRoutings] = {routingType::none};
		uint8_t routingDestinatons[numberOfRoutings] = {0};
		notePreviewType notePreview = notePreviewType::none;
		
		const char* getPreviewTypeString();
		
		notePreviewType getPreviewType();
		
		const char* getRecordTypeString();
		
		routingType getRoutingType();
		
		recordType getRecordType();
		
		const char* getRoutingTypeString();
		
		bool isRoutingDestinationRelevant();
		
		uint8_t getRoutingDestination();
		
		void toggleReceiveClock	();
		void toggleReceiveStartStop	();
		void toggleReceiveSeqControl	();
		void toggleReceiveSSP	();
		
		void setRecordType	(const int val);
		void setPreviewType	(const int val);
		void setRoutingType	(const int val);
		void setRoutingDestination	(const int val);
	};
	
	struct midiOutputSettings{
		midiOutputSettings(const char* c_name);
		const char* name;
		
		bool sendClock = true;
		bool sendStartStop = true;
		bool sendSeqControl = true;
		bool sendSSP = true;
		
		void toggleSendClock	();
		void toggleSendStartStop	();
		void toggleSendSeqControl	();
		void toggleSendSSP	();
	};
	
	// const char* getInputName(const int index);

	// const char* getOutputName(const int index);
	
	////
	////FUNCTIONS
	
	void sendNoteOn(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const MIDIPort port);
	void sendNoteOff(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const MIDIPort port);
	void sendRealTime(const midi::MidiType type, const MIDIPort port);
	void sendCC(const uint8_t number, const uint8_t value, const uint8_t chan, const MIDIPort port);
	void sendStart();
	void sendContinue();
	void sendStop();
	void sendClock();
	void sendAllNotesOff();
	void readMIDI();
	const char* getName(const unsigned int num);
	
	midiOutputSettings& getMIDIOutputSettings(const int port);
	midiInputSettings& getMIDIInputSettings(const int port);

	midi::MidiInterface<midi::SerialMIDI<HardwareSerial> >& getMIDIInputHardwarePort(const int port);
	
	// //Outputs
	// void toggleSendClock(const int port);
	// void toggleSendStartStop(const int port);
	// void toggleSendSeqControl(const int port);
	// void toggleSendSSP(const int port);
	
	// //Inputs
	// void toggleReceiveClock(const int port);
	// void toggleReceiveStartStop(const int port);
	// void toggleReceiveSeqControl(const int port);
	// void toggleReceiveSSP(const int port);
	
	// void setRecordType(const int port, const int button);
	// void setPreviewType(const int port, const int button);
	// void setRoutingType(const int port, const int button);
	// void setRoutingDestination(const int port, const int button);
}