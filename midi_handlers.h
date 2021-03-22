#ifndef MIDI_handlers
#define MIDI_handlers

//What happens on MIDI in events

namespace MIDIhandlers{
	using namespace MIDIports;
	
	void myNoteOn(byte channel, byte note, byte velocity, const uint8_t port){//Absolute mess:
		// lg("myNoteOn");
		midiInputSettings& m = getMIDIInputSettings(port);//Get the port
		routingType route = m.getRoutingType();
		if(route == routingType::none)
			{return;}
		recordType record = m.getRecordType();
		notePreviewType preview = m.getPreviewType();
		
		bool isInRecordMode = interface::isRecording();
		
		if(route == routingType::buttons){
			//doActiveNoteFunction
		}
		else if(route == routingType::port){
			//Echo
		}
		bool recording = false;
		uint8_t relevantTrack = 15;
		if(
			(record == recordType::all 	)||
			(record == recordType::invert && !isInRecordMode	)||
			(record == recordType::record && isInRecordMode	)
			) {
			recording = true;
				switch(route){
					case routingType::activePattern :
						relevantTrack = interface::editTrack;
					case routingType::track :
						relevantTrack = m.getRoutingDestination();
					case routingType::trackFromChannel :
						relevantTrack = channel-1;
					default:
						break;
				}
			//record note on relevantTrack
		}
		
		switch(preview){
			case notePreviewType::none:
				return;
			case notePreviewType::noteOnly:
				if(route == routingType::buttons){return;}
				break;
			case notePreviewType::recordOnly:
				if(!recording){return;}
				break;
			case notePreviewType::stoppedOnly:
				if(Sequencing::getSequencerStatus() != Sequencing::sequencerStatus::stopped){return;}
				break;
			case notePreviewType::priority:
				interface::mute::muteTrack(relevantTrack);
				scheduled::newEvent(
					scheduled::lOE::unmute	,
					[relevantTrack]{interface::mute::unmuteTrack(relevantTrack);} ,
					interface::settings::recordPriorityCutoff
				);
				lg("this is bad and should use silenceNotesWhenRecording");
				break;
			case notePreviewType::all:
			default:
				break;
		}
		
		int outputPort = 0;
		
		switch(route){
			case routingType::activePattern :
			case routingType::track :
			case routingType::trackFromChannel :
				outputPort = Sequencing::getTrack(relevantTrack).getMIDIPortNumber() - 1;
				break;
			case routingType::buttons:
				outputPort = Sequencing::getActiveTrack().getMIDIPortNumber() - 1;
				break;
			case routingType::port:
				outputPort = m.getRoutingDestination();
				break;
			default:
				break;
		}
		if(outputPort >= 0){
			lg("MIDIhandlers::switch");
			sendNoteOn(note, velocity, channel, static_cast<MIDIPort>(outputPort));
		}
	}
	
  void myNoteOff(byte channel, byte note, byte velocity, const uint8_t port){
		//interface::record::endNote(note - 36);
	}
  void myAfterTouchPoly(byte channel, byte note, byte velocity, const uint8_t port){}
  void myControlChange(byte channel, byte control, byte value, const uint8_t port){
		switch(control){
			case 123:
				if(value == 0){
					// lg("allnotesoff");
				}
				break;
			default:
				break;
		}
	}
  void myProgramChange(byte channel, byte program, const uint8_t port){}
  void myAfterTouch(byte channel, byte pressure, const uint8_t port){}
  void myPitchChange(byte channel, int pitch, const uint8_t port){}
  void mySystemExclusiveChunk(const byte *data, uint16_t length, bool last, const uint8_t port){}
  //void mySystemExclusive(byte *data, unsigned int length){}
  void myTimeCodeQuarterFrame(byte data, const uint8_t port){}
  void mySongPosition(uint16_t beats, const uint8_t port){}
  void mySongSelect(byte songNumber, const uint8_t port){}
  void myTuneRequest(const uint8_t port){}
	
  void myClock(const uint8_t port){
		if(getMIDIInputSettings(port).receiveClock){
			//clockReceive::onClock();
		}
	}
	
  void myStart(const uint8_t port){
		if(getMIDIInputSettings(port).receiveStartStop){
			Sequencing::startSequencer();
		}
	}
  void myContinue(const uint8_t port){}
	
  void myStop(const uint8_t port){
		if(getMIDIInputSettings(port).receiveStartStop){
			Sequencing::stopSequencer();
		}
	}
	
  void myActiveSensing(const uint8_t port){}
  void mySystemReset(const uint8_t port){}
  void myRealTimeSystem(byte realtimebyte, const uint8_t port){}
	
	void begin(){
		// using namespace std::placeholders;
		
		// for(int i = 0; i<gc::numberOfMIDIIns-1; i++){
			// inPorts[i].setHandleNoteOff(	std::bind(&myNoteOff	,_1, _2, _3, i	) );
			// inPorts[i].setHandleNoteOn(	std::bind(&myNoteOn	,_1, _2, _3, i	) );
			// inPorts[i].setHandleAfterTouchPoly(	std::bind(&myAfterTouchPoly	,_1, _2, _3, i	) );
			// inPorts[i].setHandleControlChange(	std::bind(&myControlChange	,_1, _2, _3, i	) );
			// inPorts[i].setHandleProgramChange(	std::bind(&myProgramChange	,_1, _2, i	) );
			// inPorts[i].setHandleTimeCodeQuarterFrame(	std::bind(&myTimeCodeQuarterFrame	,_1, i	) );
			// inPorts[i].setHandleSongSelect(	std::bind(&mySongSelect	,_1, i	) );
			// inPorts[i].setHandleTuneRequest(	std::bind(&myTuneRequest	,i	) );
			// inPorts[i].setHandleClock(	std::bind(&myClock	,i	) );
			// inPorts[i].setHandleStart(	std::bind(&myStart	,i	) );
			// inPorts[i].setHandleContinue(	std::bind(&myContinue	,i	) );
			// inPorts[i].setHandleStop(	std::bind(&myStop	,i	) );
			// inPorts[i].setHandleActiveSensing(	std::bind(&myActiveSensing	,i	) );
			// inPorts[i].setHandleSystemReset(	std::bind(&mySystemReset	,i	) );
		// }
		
		getMIDIInputHardwarePort(0).setHandleNoteOff(	[](	byte chan, byte pitch, byte velocity	){myNoteOff	(chan, pitch, velocity, 0	); });
		getMIDIInputHardwarePort(0).setHandleNoteOn(	[](	byte chan, byte pitch, byte velocity	){myNoteOn	(chan, pitch, velocity, 0	); });
		getMIDIInputHardwarePort(0).setHandleAfterTouchPoly(	[](	byte chan, byte pitch, byte velocity	){myAfterTouchPoly	(chan, pitch, velocity, 0	); });
		getMIDIInputHardwarePort(0).setHandleControlChange(	[](	byte chan, byte control, byte value	){myControlChange	(chan, control, value, 0	); });
		getMIDIInputHardwarePort(0).setHandleProgramChange(	[](	byte channel, byte program	){myProgramChange	(channel, program, 0	); });
		getMIDIInputHardwarePort(0).setHandleTimeCodeQuarterFrame(	[](	byte data	){myTimeCodeQuarterFrame	(data, 0	); });
		getMIDIInputHardwarePort(0).setHandleSongSelect(	[](	byte songNumber	){mySongSelect	(songNumber, 0	); });
		getMIDIInputHardwarePort(0).setHandleTuneRequest(	[](		){myTuneRequest	(0	); });
		getMIDIInputHardwarePort(0).setHandleClock(	[](		){myClock	(0	); });
		getMIDIInputHardwarePort(0).setHandleStart(	[](		){myStart	(0	); });
		getMIDIInputHardwarePort(0).setHandleContinue(	[](		){myContinue	(0	); });
		getMIDIInputHardwarePort(0).setHandleStop(	[](		){myStop	(0	); });
		getMIDIInputHardwarePort(0).setHandleActiveSensing(	[](		){myActiveSensing	(0	); });
		getMIDIInputHardwarePort(0).setHandleSystemReset(	[](		){mySystemReset	(0	); });
		
		getMIDIInputHardwarePort(1).setHandleNoteOff(	[](	byte chan, byte pitch, byte velocity	){myNoteOff	(chan, pitch, velocity, 1	); });
		getMIDIInputHardwarePort(1).setHandleNoteOn(	[](	byte chan, byte pitch, byte velocity	){myNoteOn	(chan, pitch, velocity, 1	); });
		getMIDIInputHardwarePort(1).setHandleAfterTouchPoly(	[](	byte chan, byte pitch, byte velocity	){myAfterTouchPoly	(chan, pitch, velocity, 1	); });
		getMIDIInputHardwarePort(1).setHandleControlChange(	[](	byte chan, byte control, byte value	){myControlChange	(chan, control, value, 1	); });
		getMIDIInputHardwarePort(1).setHandleProgramChange(	[](	byte channel, byte program	){myProgramChange	(channel, program, 1	); });
		getMIDIInputHardwarePort(1).setHandleTimeCodeQuarterFrame(	[](	byte data	){myTimeCodeQuarterFrame	(data, 1	); });
		getMIDIInputHardwarePort(1).setHandleSongSelect(	[](	byte songNumber	){mySongSelect	(songNumber, 1	); });
		getMIDIInputHardwarePort(1).setHandleTuneRequest(	[](		){myTuneRequest	(1	); });
		getMIDIInputHardwarePort(1).setHandleClock(	[](		){myClock	(1	); });
		getMIDIInputHardwarePort(1).setHandleStart(	[](		){myStart	(1	); });
		getMIDIInputHardwarePort(1).setHandleContinue(	[](		){myContinue	(1	); });
		getMIDIInputHardwarePort(1).setHandleStop(	[](		){myStop	(1	); });
		getMIDIInputHardwarePort(1).setHandleActiveSensing(	[](		){myActiveSensing	(1	); });
		getMIDIInputHardwarePort(1).setHandleSystemReset(	[](		){mySystemReset	(1	); });
		
		getMIDIInputHardwarePort(2).setHandleNoteOff(	[](	byte chan, byte pitch, byte velocity	){myNoteOff	(chan, pitch, velocity, 2	); });
		getMIDIInputHardwarePort(2).setHandleNoteOn(	[](	byte chan, byte pitch, byte velocity	){myNoteOn	(chan, pitch, velocity, 2	); });
		getMIDIInputHardwarePort(2).setHandleAfterTouchPoly(	[](	byte chan, byte pitch, byte velocity	){myAfterTouchPoly	(chan, pitch, velocity, 2	); });
		getMIDIInputHardwarePort(2).setHandleControlChange(	[](	byte chan, byte control, byte value	){myControlChange	(chan, control, value, 2	); });
		getMIDIInputHardwarePort(2).setHandleProgramChange(	[](	byte channel, byte program	){myProgramChange	(channel, program, 2	); });
		getMIDIInputHardwarePort(2).setHandleTimeCodeQuarterFrame(	[](	byte data	){myTimeCodeQuarterFrame	(data, 2	); });
		getMIDIInputHardwarePort(2).setHandleSongSelect(	[](	byte songNumber	){mySongSelect	(songNumber, 2	); });
		getMIDIInputHardwarePort(2).setHandleTuneRequest(	[](		){myTuneRequest	(2	); });
		getMIDIInputHardwarePort(2).setHandleClock(	[](		){myClock	(2	); });
		getMIDIInputHardwarePort(2).setHandleStart(	[](		){myStart	(2	); });
		getMIDIInputHardwarePort(2).setHandleContinue(	[](		){myContinue	(2	); });
		getMIDIInputHardwarePort(2).setHandleStop(	[](		){myStop	(2	); });
		getMIDIInputHardwarePort(2).setHandleActiveSensing(	[](		){myActiveSensing	(2	); });
		getMIDIInputHardwarePort(2).setHandleSystemReset(	[](		){mySystemReset	(2	); });
		
		// getMIDIInputHardwarePort(3).setHandleNoteOff(	[](	byte chan, byte pitch, byte velocity	){myNoteOff	(chan, pitch, velocity, 3	); });
		// getMIDIInputHardwarePort(3).setHandleNoteOn(	[](	byte chan, byte pitch, byte velocity	){myNoteOn	(chan, pitch, velocity, 3	); });
		// getMIDIInputHardwarePort(3).setHandleAfterTouchPoly(	[](	byte chan, byte pitch, byte velocity	){myAfterTouchPoly	(chan, pitch, velocity, 3	); });
		// getMIDIInputHardwarePort(3).setHandleControlChange(	[](	byte chan, byte control, byte value	){myControlChange	(chan, control, value, 3	); });
		// getMIDIInputHardwarePort(3).setHandleProgramChange(	[](	byte channel, byte program	){myProgramChange	(channel, program, 3	); });
		// getMIDIInputHardwarePort(3).setHandleTimeCodeQuarterFrame(	[](	byte data	){myTimeCodeQuarterFrame	(data, 3	); });
		// getMIDIInputHardwarePort(3).setHandleSongSelect(	[](	byte songNumber	){mySongSelect	(songNumber, 3	); });
		// getMIDIInputHardwarePort(3).setHandleTuneRequest(	[](		){myTuneRequest	(3	); });
		// getMIDIInputHardwarePort(3).setHandleClock(	[](		){myClock	(3	); });
		// getMIDIInputHardwarePort(3).setHandleStart(	[](		){myStart	(3	); });
		// getMIDIInputHardwarePort(3).setHandleContinue(	[](		){myContinue	(3	); });
		// getMIDIInputHardwarePort(3).setHandleStop(	[](		){myStop	(3	); });
		// getMIDIInputHardwarePort(3).setHandleActiveSensing(	[](		){myActiveSensing	(3	); });
		// getMIDIInputHardwarePort(3).setHandleSystemReset(	[](		){mySystemReset	(3	); });
			
		usbMIDI.setHandleNoteOff(	[](	byte chan, byte pitch, byte velocity	){myNoteOff	(chan, pitch, velocity, 4	); });
		usbMIDI.setHandleNoteOn(	[](	byte chan, byte pitch, byte velocity	){myNoteOn	(chan, pitch, velocity, 4	); });
		usbMIDI.setHandleAfterTouchPoly(	[](	byte chan, byte pitch, byte velocity	){myAfterTouchPoly	(chan, pitch, velocity, 4	); });
		usbMIDI.setHandleControlChange(	[](	byte chan, byte control, byte value	){myControlChange	(chan, control, value, 4	); });
		usbMIDI.setHandleProgramChange(	[](	byte channel, byte program	){myProgramChange	(channel, program, 4	); });
		usbMIDI.setHandleTimeCodeQuarterFrame(	[](	byte data	){myTimeCodeQuarterFrame	(data, 4	); });
		usbMIDI.setHandleSongSelect(	[](	byte songNumber	){mySongSelect	(songNumber, 4	); });
		usbMIDI.setHandleTuneRequest(	[](		){myTuneRequest	(4	); });
		usbMIDI.setHandleClock(	[](		){myClock	(4	); });
		usbMIDI.setHandleStart(	[](		){myStart	(4	); });
		usbMIDI.setHandleContinue(	[](		){myContinue	(4	); });
		usbMIDI.setHandleStop(	[](		){myStop	(4	); });
		usbMIDI.setHandleActiveSensing(	[](		){myActiveSensing	(4	); });
		usbMIDI.setHandleSystemReset(	[](		){mySystemReset	(4	); });
		
		//
		// usbMIDI.setHandleAfterTouch(	myAfterTouch	);
		// usbMIDI.setHandlePitchChange(	myPitchChange	);
		// usbMIDI.setHandleSystemExclusive(	mySystemExclusiveChunk	);
		// usbMIDI.setHandleRealTimeSystem(	myRealTimeSystem	);
		
		//inPorts[i].setHandleAfterTouch(	myAfterTouch	);
		//inPorts[i].setHandlePitchChange(	myPitchChange	);
		//inPorts[i].setHandleSystemExclusive(	mySystemExclusiveChunk	);
		//inPorts[i].setHandleRealTimeSystem(	myRealTimeSystem	);
		//inPorts[i].setHandleSongPosition(	[i](	){mySongPosition	});

	}
}
#endif