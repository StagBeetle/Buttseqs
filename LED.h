//Sending data to LEDs
#ifndef LED_h
#define LED_h
#include "LED_external.h"
#include "Utility.h"
//#include "comms.h"
#include <bitset>
// LEDfeedback::updateLEDs();

uint8_t brightnesses[4] = {0,2,40,255};

uint8_t keyOffsets[7] = { 0, 16, 41, 43, 45, 61, 77};
uint8_t   keySizes[7] = {16, 25,  2,  2, 16, 16,  4};



namespace LEDfeedback{
	
		void LEDsOn(){
		Wire.beginTransmission(_i2caddr);
		Wire.write((byte)(0x00));
		for(int i = 0; i<18; i++){
			Wire.write(255);
		}
		Wire.endTransmission();
	}

	void setup(){
		Wire.begin();
		Wire.setClock(400000);
		writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00); // shutdown
		delay(10);
		writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01); // out of shutdown
		writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG, ISSI_REG_CONFIG_PICTUREMODE); // picture mode
		displayFrame(0);
		Wire.beginTransmission(_i2caddr);
		Wire.write((byte)ISSI_COMMANDREGISTER);
		Wire.write(0);
		Wire.endTransmission();
		LEDsOn();
	}

	void LED_PWM(const uint8_t reg, const uint8_t data) {
		Wire.beginTransmission(_i2caddr);
		Wire.write((byte)0x24 + reg);
		Wire.write((byte)data);
		Wire.endTransmission();
	}

	void setLEDs(const int ledset, const uint8_t* dataArray){
		Wire.beginTransmission(_i2caddr);
		Wire.write((byte)0x24 + keyOffsets[ledset]);
		for(int i = 0; i< keySizes[ledset]; i++){
			Wire.write(brightnesses[(*(dataArray+i))%4]); // 0 1 2 3 -- Mod 4 incase some shit data gets fed in
		}
		Wire.endTransmission();
	}

// void writeLED(const uint8_t* data){
	// const uint8_t* d = data+1;
	// setLEDs(data[0], d);
// }

		class LEDSet{
			private:
				static const uint8_t maxRequired = 25;
				const uint8_t sendNum;
				const uint8_t numberOfLEDs;
				std::bitset<maxRequired*2> ledArray; //Waste of space but it doesn't really matter
				//int ledArray[maxRequired] = {0};
			public:
				LEDSet(const uint8_t c_sendNum, const uint8_t c_numberOfLEDs) :
				sendNum(c_sendNum),
				numberOfLEDs(c_numberOfLEDs) {}
				
				void send(){
					uint8_t arr[numberOfLEDs];
					//arr[0] = sendNum;
					//arr[0] = sendNum + 48;
					for(unsigned int i = 0; i<numberOfLEDs; i++){
						uint8_t val = getSingle(i);
						arr[i] = val;
					}
					setLEDs(sendNum, arr);
					//utl::sendSerialArray(sendNum + 48, arr, numberOfLEDs);
					//comms::send(comms::pM::LED, 0, arr, numberOfLEDs+1);
				}
				
				void clear(){
					ledArray.reset();
				}
				void sendSendNum(){
					lg(sendNum);
					lg(numberOfLEDs);
				}
				void clearAndSend(){
					ledArray.reset();
					send();
				}
				void setSingle(const uint8_t num, const int val){
					if(num < numberOfLEDs){
						ledArray.set(2*num	, val / 2);
						ledArray.set(2*num+1	, val % 2);
					}
				}
				uint8_t getSingle(const uint8_t num){
					if(num < numberOfLEDs){
						return 
							ledArray.test(num*2) * 2 + 
							ledArray.test(num*2+1);
					} else {
						return 0;
					}
				}
				void setSingleIfBrighter(const uint8_t num, const int val){
					if(val > getSingle(num)){
						setSingle(num, val);
					}
				}
					//send();
					// template<typename T>void update(const std::array<uint8_t, T> arr){
						// for(int i = 0; i<arr.max_size && i<numberOfLEDs; i++){
							// ledArray.set(2*i	, arr[i] / 2);
							// ledArray.set(2*i+1	, arr[i] % 2);
						// }
						// send();
					// }
				template<typename T>void update(const T arr){
					for(unsigned int i = 0; i<arr.max_size() && i<numberOfLEDs; i++){
						ledArray.set(2*i	, arr[i] / 2);
						ledArray.set(2*i+1	, arr[i] % 2);
					}
					send();
				}
		};
		LEDSet
			steps{0,	gc::keyNum::steps	},
			notes{1,	gc::keyNum::notes	},
			verti{2,	gc::keyNum::vertical	},
			horiz{3,	gc::keyNum::horizontal	},
			datas{4,	gc::keyNum::datas	},
			modes{5,	gc::keyNum::modes	},
			mores{6,	gc::keyNum::extras	};

		LEDSet& getLEDSet(buttons::keySet::ks k){
			switch(k){
				case buttons::keySet::step:
					return steps;
				case buttons::keySet::note:
					return notes;
				case buttons::keySet::vertical:
					return verti;
				case buttons::keySet::horizontal:
					return horiz;
				case buttons::keySet::mode:
					return modes;
				case buttons::keySet::data:
					return datas;
				case buttons::keySet::extra:
					return mores;
				default:
					return mores;
			}
		}
		
		void showStepSelection(LEDSet& LEDs){
			std::array<int, gc::keyNum::steps> arr = {false};
			if(interface::settings::editSubstepsAlso){
				interface::forEachStepButtonWithSubstep([&arr](patMem::position pos, int step, int substep){
					arr[step] = 3;
				});
			} else {
				interface::forEachStepButton([&arr](patMem::position pos, int step){
					arr[step] = 3;
				});
			}
			LEDs.update(arr);
			}
			
		void showNoteSelection(LEDSet& LEDs){
			std::array<int, gc::keyNum::notes> arr = {0};
			interface::forEachPitch([&arr](int i){
				int LED = i - interface::keyboardOctave*12;
				if(LED >= 0 && LED < gc::keyNum::notes){
					arr[LED] = 3;
					}
				});
			LEDs.update(arr);
			}

		void showStepSelectionOrStepsContainingNote(LEDSet& LEDs){
			if (interface::settings::editPriority == interface::editPriorityModes::stepFirst){
				showStepSelection(LEDs);
				}
			else{//noteFirst
				bool isInSubstep = interface::inSubstep();
				std::array<int, gc::keyNum::steps> arr = {0};
				interface::forEachPitch([&arr, isInSubstep](int pitch){
					
					std::array<bool, gc::keyNum::steps> temp = Sequencing::getActivePattern().getStepLeds(pitch, interface::viewBar, interface::editStepForSubstep, interface::settings::editSubstepsAlso);
					utl::arrayOR(arr, temp);
					
				});
				for(int& j : arr){
					j = j ? 3 : 0;
				}
				LEDs.update(arr);
				}
			}
			//interface::settings::editSubstepsAlso
		void showNotesOnStep(LEDSet& LEDs){
			std::array<int, gc::keyNum::notes> arr = {0};
			bool isInSubstep = interface::inSubstep();
			interface::forEachStepButton([&arr, isInSubstep](patMem::position pos, int button){
				std::array<bool, gc::keyNum::notes> temp = Sequencing::getActivePattern().getNoteLeds(interface::keyboardOctave * 12, pos, isInSubstep, interface::settings::editSubstepsAlso);
				utl::arrayOR(arr,temp);
				});
			for(int& j : arr){
				j = j ? 3 : 0;
			}
			LEDs.update(arr);
		}
			
		void showNoteSelectionOrNotesOnStep(LEDSet& LEDs){
			if (interface::settings::editPriority == interface::editPriorityModes::stepFirst){
				showNotesOnStep(LEDs);
				}
			else{//noteFirst
				showNoteSelection(LEDs);
				}
			}
			
		void showStepsContainingNotes(LEDSet& LEDs){//Does not ever show substeps as this is for inside substep view
			std::array<int, gc::keyNum::steps> arr = {0};
			interface::forEachPitch([&arr](int pitch){
				std::array<bool, gc::keyNum::steps> temp = Sequencing::getActivePattern().getStepLeds(pitch, interface::viewBar, -1, false);
				utl::arrayOR(arr, temp);
				});
			for(int& j : arr){
				j = j ? 3 : 0;
			}
			LEDs.update(arr);
		}
			
		void showActiveTrack(LEDSet& LEDs){
			//LEDs.sendSendNum();
			LEDs.clear();
			LEDs.setSingle(interface::editTrack, 3);
			LEDs.send();
			}
			
		void showViewBar(LEDSet& LEDs){
			LEDs.clear();
			LEDs.setSingle(interface::viewBar, 3);
			LEDs.send();
			}
			
		void showSubstepStep(LEDSet& LEDs){
			LEDs.clear();
			LEDs.setSingle(interface::editStepForSubstep, 3);
			LEDs.send();
			}

		void showTrackMutes(LEDSet& LEDs){
			LEDs.clear();
			for (int i = 0; i<gc::keyNum::steps; i++){
				LEDs.setSingle(i, !Sequencing::trackArray[i].getMuted() ? 3 : 0);
				}
			LEDs.send();
			}
			
		void showNoteMutes(LEDSet& LEDs){
			LEDs.clear();
			for (int i = 0; i<gc::keyNum::notes;i++){
				int theNote = i+(12*interface::keyboardOctave);
				bool muted = (theNote < 128) ? Sequencing::trackArray[interface::editTrack].isNoteMuted(theNote) : true;
				LEDs.setSingle(i, !muted ? 3 : 0);
				}
			LEDs.send();
			}
			
		void showAvailableTrackBank(LEDSet& LEDs){
			LEDs.clear();
			for (int i = 0; i<gc::keyNum::datas; i++){
				bool isThereAPatternAtThatPosition = Sequencing::trackArray[interface::editTrack].doesTrackBankHavePattern(i);
				LEDs.setSingle(i, isThereAPatternAtThatPosition ? 3 : 0);
				}
			LEDs.send();
			}
			
		void showMode(LEDSet& LEDs){
			LEDs.clear();
			for(int i = 0; i< gc::keyNum::modes; i++){
				if(modes::buttons[i] == &modes::getActiveMode()){
					LEDs.setSingle(i, 3);
					break;
				}
			}
			LEDs.send();
			}
		
		void stepChaser(LEDSet& LEDs){
			if(modes::getActiveMode().allowChase()){
				modes::getActiveMode().updateLEDs(); // Clears the old lit steps
				if (Sequencing::isSeqPlaying()){
					int currentPatternStep = Sequencing::trackArray[interface::editTrack].getCurrentStep();
					int length = Sequencing::trackArray[interface::editTrack].getStepsPerBar();
					LEDs.setSingleIfBrighter(currentPatternStep, 3);
					if(interface::settings::useFancyLEDChaser){
						LEDs.setSingleIfBrighter((currentPatternStep-1) % length, 2);
						LEDs.setSingleIfBrighter((currentPatternStep-2) % length, 1);
						}
					LEDs.setSingleIfBrighter(Sequencing::trackArray[interface::editTrack].getCurrentBar(), 2);
					}
				LEDs.send();
			}
		}
		
		void showKeyboardOctave(LEDSet& LEDs){
			LEDs.clear();
			int offset = interface::keyboardOctave - 3;
			if(offset < 0){
				LEDs.setSingle(0, min(offset, 3));
			}
			else if (offset > 0){
				LEDs.setSingle(1, abs(offset));
			}
			LEDs.send();
		}
		
		void showExtras(LEDSet& LEDs){
			// lgc("showExtra:");
			// lg(Sequencing::seqSta[static_cast<int>(Sequencing::seqStatus)]);
			int playButtonIllumination = 0;
			int stopButtonIllumination = 0;
			switch(Sequencing::seqStatus){
				case Sequencing::sequencerStatus::stopped :
					playButtonIllumination = 0;
					stopButtonIllumination = 3;
					break;
				case Sequencing::sequencerStatus::playing :
					playButtonIllumination = 3;
					stopButtonIllumination = 0;
					break;
				case Sequencing::sequencerStatus::paused :
					playButtonIllumination = 2;
					stopButtonIllumination = 2;
					break;
			}
			
			LEDs.clear();
			LEDs.setSingle(0, playButtonIllumination	);
			LEDs.setSingle(1, stopButtonIllumination	);
			LEDs.setSingle(2, interface::isRecording()	? 3 : 0);
			LEDs.send();
		}
		
		//Removed because of different range methods
		// void showStepRange(LEDSet& LEDs){
			// LEDs.clear();
			// for(int i = interface::process::rangeStart.getStep(); i < interface::process::rangeEnd.getStep() + 1; i++){
				// LEDs.setSingle(i, 3);
			// }
			// LEDs.send();
		// }
		
		void showTranspose(LEDSet& LEDs){
			LEDs.clear();
			int activeTranspose = Sequencing::getActivePattern().getTranspose();
			int baseLine = 48;
			int firstLed = interface::keyboardOctave*12;
			LEDs.setSingle(baseLine - firstLed, 1);
			LEDs.setSingle((activeTranspose+48) - firstLed, 3);
			LEDs.send();
			// lg(activeTranspose);
			// lg(activeTranspose - firstLed);
		}
		
		void clearAll(){
			steps.clearAndSend();
			notes.clearAndSend();
			verti.clearAndSend();
			horiz.clearAndSend();
			datas.clearAndSend();
		}
		
		void updateLEDs(){
			modes::getActiveMode().updateLEDs();
			stepChaser(steps);
		}
};
#endif