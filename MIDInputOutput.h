#ifndef MIDIports_h
#define MIDIports_h

#include <MIDI.h>

namespace MIDIports{
	
	const int numberOfInPorts = gc::numberOfMIDIIns;
	const int numberOfOutPorts = gc::numberOfMIDIOuts;
	
	
#if defined(__MK66FX1M0__) //Teensy 3.6
	
	midi::MidiInterface<HardwareSerial> outPorts[] = {
		{Serial1},
		{Serial3},
		{Serial4},
		{Serial5},
	};
	
	midi::MidiInterface<HardwareSerial> inPorts[] = {
		{Serial1},
		{Serial3},
		{Serial4},
		{Serial5},
	};
	
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x
	
	midi::MidiInterface<HardwareSerial> outPorts[] = {
		{Serial1},
		{Serial4},
		{Serial6},
		{Serial7},
	};
	
	midi::MidiInterface<HardwareSerial> inPorts[] = {
		{Serial1},
		{Serial4},
		{Serial6},
		{Serial7},
	};
	
#endif
//Don't need an else case.
	
	void setup(){
		for(auto p : outPorts){
			p.begin();
		}
	}
	
	//usbMIDI
	
	// enum class inputType : uint8_t{
		// buttons = 0,
		// record,
		// recordInvert,
	// };
	
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
	
	const int numberOfRoutings = 3;
	
	struct midiInput{
		midiInput(const char* c_name) : 
			name(c_name)
		{}
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
		
		const char* getPreviewTypeString(){
			return notePreviewNames[static_cast<int>(notePreview)];
		}
		
		notePreviewType getPreviewType(){
			return notePreview;
		}
		
		// const char* getInputType(){
			// return inputTypeNames[static_cast<int>(input)];
		// }
		
		const char* getRecordTypeString(){
			return recordNames[static_cast<int>(recordMode)];
		}
		
		routingType getRoutingType(){
			return routings[0];
		}
		
		recordType getRecordType(){
			return recordMode;
		}
		
		const char* getRoutingTypeString(){
			return routingTypeNames[static_cast<int>(routings[0])];
		}
		
		bool isRoutingDestinationRelevant(){
			routingType r = routings[0];
			return r == routingType::track || r == routingType::port;
		}
		
		uint8_t getRoutingDestination(){
			int r = routingDestinatons[0];
			return (routings[0] == routingType::port) ? constrain(r, 0, numberOfOutPorts-1): r ;
		}
		
		void toggleReceiveClock	(){receiveClock =	!receiveClock;	}
		void toggleReceiveStartStop	(){receiveStartStop =	!receiveStartStop;	}
		void toggleReceiveSeqControl	(){receiveSeqControl =	!receiveSeqControl;	}
		void toggleReceiveSSP	(){receiveSSP =	!receiveSSP;	}
		
		void setRecordType	(const int val){if(val<static_cast<int>(recordType::max	)){recordMode	= static_cast<recordType	>(val);} }
		void setPreviewType	(const int val){if(val<static_cast<int>(notePreviewType::max	)){notePreview	= static_cast<notePreviewType	>(val);} }
		void setRoutingType	(const int val){if(val<static_cast<int>(routingType::max	)){routings[0]	= static_cast<routingType	>(val);} }
		void setRoutingDestination	(const int val){if(val<static_cast<int>(16/*so it lines up*/	)){routingDestinatons[0]	= static_cast<int	>(val);} }
	};
	
	struct midiOutput{
		midiOutput(const char* c_name) : 
			name(c_name)
		{}
		const char* name;
		
		bool sendClock = true;
		bool sendStartStop = true;
		bool sendSeqControl = true;
		bool sendSSP = true;
		
		void toggleSendClock	(){sendClock =	!sendClock;	}
		void toggleSendStartStop	(){sendStartStop =	!sendStartStop;	}
		void toggleSendSeqControl	(){sendSeqControl =	!sendSeqControl;	}
		void toggleSendSSP	(){sendSSP =	!sendSSP;	}
	};
	
	midiOutput outputs[numberOfOutPorts] = {
		{"Prt1"	}, 
		{"Prt2"	}, 
		{"Prt3"	}, 
		{"Prt4"	}, 
		{"USB"	},
	};
		
	midiInput inputs[numberOfInPorts] = {
		{"Prt1"	}, 
		{"Prt2"	}, 
		{"Prt3"	}, 
		{"Prt4"	}, 
		{"USB"	},
	};
	
	const char* getInputName(const int index){
		lgc("MIDIIO::getInputNameIndex:");lg(index);
		if(index >=numberOfInPorts){return "error";}
		return inputs[index].name;
	}
	
	const char* getOutputName(const int index){
		lgc("MIDIIO::getOutputNameIndex:");lg(index);
		if(index >=numberOfOutPorts){return "error";}
		return outputs[index].name;
	}
	
	////
	////FUNCTIONS
	
	void sendNoteOn(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const uint8_t port){
		// lgc("MIDIIO::sendNoteOn:");lg(port);
		if(port == 0){return;}
		else if(port < numberOfOutPorts-1){	outPorts[port-1].sendNoteOn(pitch, velocity, chan);}
		else{	usbMIDI.sendNoteOn(pitch, velocity, chan);}
		//utl::LEDflash();
	}

	void sendNoteOff(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const uint8_t port){
		// lgc("MIDIIO::sendNoteOff:");lg(port);
		//lg("noteoff");
		if(port == 0){return;}
		else if(port < numberOfOutPorts-1){	outPorts[port-1].sendNoteOff(pitch, velocity, chan);}
		else{	usbMIDI.sendNoteOff(pitch, velocity, chan);}
	}
	
	void sendRealTime(const midi::MidiType type, const uint8_t port){
		if(port == 0){return;}
		else if(port < numberOfOutPorts-1){	outPorts[port-1].sendRealTime(type);}
		else{	usbMIDI.sendRealTime(type);}
	}
	
	void sendCC(const uint8_t number, const uint8_t value, const uint8_t chan, const uint8_t port){
		if(port == 0){return;}
		else if(port < numberOfOutPorts-1){	outPorts[port-1].sendControlChange(number, value, chan);}
		else{	usbMIDI.sendControlChange(number, value, chan);}
	}
	
	void sendStart(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputs[i].sendStartStop){
				sendRealTime(midi::Start, i);
			}
		}
	}
	
	void sendContinue(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputs[i].sendStartStop){
				sendRealTime(midi::Continue, i);
			}
		}
	}
	
	void sendStop(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputs[i].sendStartStop){
				sendRealTime(midi::Stop, i);
			}
		}
	}
	
	void sendClock(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputs[i].sendClock){
				sendRealTime(midi::Clock, i);
			}
		}
	}
	
	void sendAllNotesOff(){
		for(int i = 0; i<numberOfOutPorts; i++){
			for(int chan = 0; chan<16; chan++){
				sendCC(midi::AllNotesOff, 0, chan, i);
			}
		}
	}
	
	void readMIDI(){
		usbMIDI.read();
		for(int i=0; i<numberOfInPorts-1; i++){
			MIDIports::inPorts[i].read();
		}
	}
	
	const char* getName(const int num){
		if(num == 0){ return "none";}
		return outputs[num-1].name;
	}
}
#endif