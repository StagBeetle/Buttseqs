#include <MIDI.h>
#include "MIDIInputOutput.h"
#include "forwarddec.h"

namespace MIDIports{
	
//Includes USB:
const int numberOfInPorts = gc::numberOfMIDIIns; //4
const int numberOfOutPorts = gc::numberOfMIDIOuts; //6
	

HardwareSerial& serialPort0 = {Serial1}; //bi
HardwareSerial& serialPort1 = {Serial7}; //bi
HardwareSerial& serialPort2 = {Serial8}; //bi
HardwareSerial& serialPort3 = {Serial4}; //out
HardwareSerial& serialPort4 = {Serial3}; //out

	
MIDI_CREATE_INSTANCE(HardwareSerial, serialPort0, port0);
MIDI_CREATE_INSTANCE(HardwareSerial, serialPort1, port1);
MIDI_CREATE_INSTANCE(HardwareSerial, serialPort2, port2);
MIDI_CREATE_INSTANCE(HardwareSerial, serialPort3, port3);
MIDI_CREATE_INSTANCE(HardwareSerial, serialPort4, port4);

//midi::MidiInterface<midi::SerialMIDI<HardwareSerial> > test = {serialPort0};

decltype(port0)* outHardwarePorts[] = {&port0, &port1, &port2, &port3, &port4};
decltype(port0)* inHardwarePorts[] = {&port0, &port1, &port2};

decltype(port0)& getMIDIInputHardwarePort(const int port){
	//if(port > gc::numberOfMIDIIns -1)
	return *inHardwarePorts[port];
}
	
	PROGMEM void begin(){
		for(auto p : outHardwarePorts){
			p->begin();
		}
	}

//midiInputSettings
		midiInputSettings::midiInputSettings(const char* c_name) : 
			name(c_name)
		{}
		
		const char* midiInputSettings::getPreviewTypeString(){
			return notePreviewNames[static_cast<int>(notePreview)];
		}
		
		notePreviewType midiInputSettings::getPreviewType(){
			return notePreview;
		}
		
		const char* midiInputSettings::getRecordTypeString(){
			return recordNames[static_cast<int>(recordMode)];
		}
		
		routingType midiInputSettings::getRoutingType(){
			return routings[0];
		}
		
		recordType midiInputSettings::getRecordType(){
			return recordMode;
		}
		
		const char* midiInputSettings::getRoutingTypeString(){
			return routingTypeNames[static_cast<int>(routings[0])];
		}
		
		bool midiInputSettings::isRoutingDestinationRelevant(){
			routingType r = routings[0];
			return r == routingType::track || r == routingType::port;
		}
		
		uint8_t midiInputSettings::getRoutingDestination(){
			int r = routingDestinatons[0];
			return (routings[0] == routingType::port) ? constrain(r, 0, numberOfOutPorts-1): r ;
		}
		
		void midiInputSettings::toggleReceiveClock	(){receiveClock =	!receiveClock;	}
		void midiInputSettings::toggleReceiveStartStop	(){receiveStartStop =	!receiveStartStop;	}
		void midiInputSettings::toggleReceiveSeqControl	(){receiveSeqControl =	!receiveSeqControl;	}
		void midiInputSettings::toggleReceiveSSP	(){receiveSSP =	!receiveSSP;	}
		
		void midiInputSettings::setRecordType	(const int val){if(val<static_cast<int>(recordType::max	)){recordMode	= static_cast<recordType	>(val);} }
		void midiInputSettings::setPreviewType	(const int val){if(val<static_cast<int>(notePreviewType::max	)){notePreview	= static_cast<notePreviewType	>(val);} }
		void midiInputSettings::setRoutingType	(const int val){if(val<static_cast<int>(routingType::max	)){routings[0]	= static_cast<routingType	>(val);} }
		void midiInputSettings::setRoutingDestination	(const int val){if(val<static_cast<int>(16/*so it lines up*/	)){routingDestinatons[0]	= static_cast<int	>(val);} }
	
//midiOutputSettings
		midiOutputSettings::midiOutputSettings(const char* c_name) : 
			name(c_name)
		{}

		void midiOutputSettings::toggleSendClock	(){sendClock =	!sendClock;	}
		void midiOutputSettings::toggleSendStartStop	(){sendStartStop =	!sendStartStop;	}
		void midiOutputSettings::toggleSendSeqControl	(){sendSeqControl =	!sendSeqControl;	}
		void midiOutputSettings::toggleSendSSP	(){sendSSP =	!sendSSP;	}
	
	midiOutputSettings outputSettings[numberOfOutPorts] = {
		{"USB"	},
		{"Prt1"	}, 
		{"Prt2"	}, 
		{"Prt3"	}, 
		{"Prt4"	}, 
		{"Prt5"	}, 
	};
		
	midiInputSettings inputSettings[numberOfInPorts] = {
		{"USB"	},
		{"Prt1"	}, 
		{"Prt2"	}, 
		{"Prt3"	}, 
	};
	
////
////FUNCTIONS
	
	uint8_t getMIDIHardwarePortNumberFromPort(const MIDIPort port){
		return static_cast<int>(port)-2;
	}
	
	bool isPortNumberInOutputRange(const uint8_t portNumber){
		return (portNumber >= 0 && portNumber < numberOfOutPorts -1);
	}
	
	const char* getPortName(const int num){//assumes num=0 is USB
		if(num < numberOfOutPorts){
			return MIDIPortNames[num+1];
		}	
		return "nothing";
	}
	
////---------------------------------------////
////---------------------------------------////
	void sendNoteOn(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const MIDIPort port){
		// lgc("MIDIIO::sendNoteOn:");lg(port);
		const uint8_t portNumber = getMIDIHardwarePortNumberFromPort(port);
		if(port == MIDIPort::none){return;} 
		else if(port == MIDIPort::usb){usbMIDI.sendNoteOn(pitch, velocity, chan);}
		else if(isPortNumberInOutputRange(portNumber)){
			outHardwarePorts[portNumber]->sendNoteOn(pitch, velocity, chan);
			// Serial.print("note on port ");
			// Serial.println(portNumber);

		}	
		//utl::LEDflash();
	}

	void sendNoteOff(const uint8_t pitch, const uint8_t velocity, const uint8_t chan, const MIDIPort port){
		// lgc("MIDIIO::sendNoteOff:");lg(port);
		//lg("noteoff");
		const uint8_t portNumber = getMIDIHardwarePortNumberFromPort(port);
		if(port == MIDIPort::none){return;} 
		else if(port == MIDIPort::usb){usbMIDI.sendNoteOff(pitch, velocity, chan);}
		else if(isPortNumberInOutputRange(portNumber)){
			// Serial.println("error?");
			outHardwarePorts[portNumber]->sendNoteOff(pitch, velocity, chan);
			// Serial.print("note off port ");
			// Serial.println(portNumber);
			// outHardwarePorts[static_cast<int>(port)-2]->sendNoteOn(pitch, 0, chan);
			
			// Serial.println("NOTE OFF");
			// Serial.println(portNumber);
			// Serial.println(pitch);
			// Serial.println(velocity);
			// Serial.println(chan);
		}
	}
	
	void sendRealTime(const midi::MidiType type, const MIDIPort port){
		const uint8_t portNumber = getMIDIHardwarePortNumberFromPort(port);
		if(port == MIDIPort::none){return;} 
		else if(port == MIDIPort::usb){usbMIDI.sendRealTime(type);}
		else if(isPortNumberInOutputRange(portNumber)){
			outHardwarePorts[portNumber]->sendRealTime(type);
			// Serial.println("error?");
			// Serial.print("sendRealTime on port ");
			// Serial.println(portNumber);
		}
	}
	
	void sendCC(const uint8_t number, const uint8_t value, const uint8_t chan, const MIDIPort port){
		const uint8_t portNumber = getMIDIHardwarePortNumberFromPort(port);
		if(port == MIDIPort::none){return;} 
		else if(port == MIDIPort::usb){usbMIDI.sendControlChange(number, value, chan);}
		else if(isPortNumberInOutputRange(portNumber)){
			outHardwarePorts[portNumber]->sendControlChange(number, value, chan);
			// Serial.println("error?");
			// Serial.print("sendCC on port ");
			// Serial.println(portNumber);
		}
	}
////---------------------------------------////
////---------------------------------------////
	void sendStart(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputSettings[i].sendStartStop){
				sendRealTime(midi::Start, static_cast<MIDIPort>(i+1));
			}
		}
	}
	
	void sendContinue(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputSettings[i].sendStartStop){
				sendRealTime(midi::Continue, static_cast<MIDIPort>(i+1));
			}
		}
	}
	
	void sendStop(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputSettings[i].sendStartStop){
				sendRealTime(midi::Stop, static_cast<MIDIPort>(i+1));
			}
		}
	}
	
	void sendClock(){
		for(int i = 0; i<numberOfOutPorts; i++){
			if(outputSettings[i].sendClock){
				sendRealTime(midi::Clock, static_cast<MIDIPort>(i+1));
			}
		}
	}
	
	void sendAllNotesOff(){
		for(int i = 0; i<numberOfOutPorts; i++){
			for(int chan = 0; chan<16; chan++){
				sendCC(midi::AllNotesOff, 0, chan, static_cast<MIDIPort>(i+1));
			}
		}
	}
	
	void readMIDI(){
		usbMIDI.read();
		for(int i=0; i<numberOfInPorts-1; i++){
			inHardwarePorts[i]->read();
		}
	}

////---------------------------------------////0
////---------------------------------------////
	
	const char* getName(const unsigned int num){
		if(num >= sizeof(MIDIPortNames)/sizeof(MIDIPortNames[0])){return "error";}
		return MIDIPortNames[num];
	}
	
	midiOutputSettings& getMIDIOutputSettings(const int port){
		return outputSettings[port];
	}
	midiInputSettings& getMIDIInputSettings(const int port){
		return inputSettings[port];
	}
	
	// //Outputs
	// void toggleSendClock(const int port){
		// outputs[port].toggleSendClock();
	// }
	// void toggleSendStartStop(const int port){
		// outputs[port].toggleSendStartStop();
	// }
	// void toggleSendSeqControl(const int port){
		// outputs[port].toggleSendSeqControl();
	// }
	// void toggleSendSSP(const int port){
		// outputs[port].toggleSendSSP();
	// }
	
	// //Inputs
	// void toggleReceiveClock(const int port){
		// inputs[port].toggleReceiveClock();
	// }
	// void toggleReceiveStartStop(const int port){
		// inputs[port].toggleReceiveStartStop();
	// }
	// void toggleReceiveSeqControl(const int port){
		// inputs[port].toggleReceiveSeqControl();
	// }
	// void toggleReceiveSSP(const int port){
		// inputs[port].toggleReceiveSSP();
	// }
	
	
	// void setRecordType(const int port, const int button){
		// inputs[port].setRecordType(button);
	// }
	// void setPreviewType(const int port, const int button){
		// inputs[port].setPreviewType(button);
	// }
	// void setRoutingType(const int port, const int button){
		// inputs[port].setRoutingType(button);
	// }
	// void setRoutingDestination(const int port, const int button){
		// inputs[port].setRoutingDestination(button);
	// }
}