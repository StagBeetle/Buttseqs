//Sending data to LEDs
#ifndef LED_h
#define LED_h
#include "LED_external.h"
#include "Utility.h"
//#include "comms.h"
#include <bitset>
// LEDfeedback::updateLEDs();

namespace LEDfeedback{
	//HARDWARE setup:
	auto AUXSPIBUS = SPI1;
	const uint8_t LEDCS = 31;
	const uint8_t numberOfButtons = 81;
	SPISettings sk9822(2000000, MSBFIRST, SPI_MODE3); 
	uint8_t brightness = 2;
	
	// uint8_t brightnesses[4] = {0,2,40,255};

	uint8_t keyOffsets[7] = { 0, 16, 41, 43, 45, 61, 77};
	uint8_t   keySizes[7] = {16, 25,  2,  2, 16, 16,  4};
	
	uint8_t LEDValues[numberOfButtons][4] = {0};
	

	void begin(){
		// AUXSPIBUS.begin();
		pinMode(LEDCS, OUTPUT);
	}
	
	void transmitFixedFrame(uint8_t f){
		for(int i = 0; i<4; i++){
			AUXSPIBUS.transfer(f);
			// delay(10);
		}
	}

	void transmitFrame(uint8_t v, uint8_t r, uint8_t g, uint8_t b){
		v = v % 32;
		AUXSPIBUS.transfer((128+64+32) | v);
		// AUXSPIBUS.transfer(B11100000);
		// delay(10);
		AUXSPIBUS.transfer(b);
		// delay(10);
		AUXSPIBUS.transfer(g);
		// delay(10);
		AUXSPIBUS.transfer(r);
		// delay(10);
	}

	bool hasChanged = false;
	
	void scheduleChange(){
		hasChanged = true;
	}
		
	
	void sendLEDs(){
		if(hasChanged){
			AUXSPIBUS.beginTransaction(sk9822);
			digitalWrite(LEDCS,HIGH);
			transmitFixedFrame(0);
			for(int i = 0; i<numberOfButtons; i++){
				transmitFrame(LEDValues[i][0], LEDValues[i][1], LEDValues[i][2], LEDValues[i][3]);
			}
			transmitFixedFrame(255);
			digitalWrite(LEDCS,LOW);
			AUXSPIBUS.endTransaction();
			hasChanged = false;
		}
	}
	
	
	const uint8_t rainbowArray[7][3] = {
		{255, 5, 5},
		{250, 50, 15},
		{250, 128, 5},
		{20, 250, 20},
		{20, 20, 200},
		{220, 0, 255},
		{250, 50, 210},
	};
	const char* const hueColours[] = {"red", "orange", "yellow", "green", "blue", "indigo", "violet"};
	
	uint8_t activeHue = 0;
	void setActiveHue(const int newHue){
		if(newHue < 7){
			activeHue = newHue;
			// sendLEDs();
			scheduleChange();
		}
	}
	
	const char* getActiveHue(){
		return hueColours[activeHue];
	}

		class LEDSet{
			private:
				// static const uint8_t maxRequired = 25;
				const uint8_t sendNum;
				const uint8_t numberOfLEDs;
				// std::bitset<maxRequired*2> ledArray; //Waste of space but it doesn't really matter
				//int ledArray[maxRequired] = {0};
			public:
				LEDSet(const uint8_t c_sendNum, const uint8_t c_numberOfLEDs) :
				sendNum(c_sendNum),
				numberOfLEDs(c_numberOfLEDs) {}
				
				void send(){
					scheduleChange();
				}
				
				void clear(){
					// ledArray.reset();
					for (int i = keyOffsets[sendNum]; i < keyOffsets[sendNum] + keySizes[sendNum]; i++){
						LEDValues[i][0] = 0;
						LEDValues[i][1] = 0;
						LEDValues[i][2] = 0;
						LEDValues[i][3] = 0;
					}
					scheduleChange();
				}
				void sendSendNum(){
					lg(sendNum);
					lg(numberOfLEDs);
				}
				void clearAndSend(){
					clear();
					// send();
					scheduleChange();
				}
				void setSingle(const uint8_t num, const int val){
					if(num < numberOfLEDs){
						int LEDIndex = keyOffsets[sendNum] + num;
						LEDValues[LEDIndex][0] = val;
						LEDValues[LEDIndex][1] = rainbowArray[activeHue][0];
						LEDValues[LEDIndex][2] = rainbowArray[activeHue][1];
						LEDValues[LEDIndex][3] = rainbowArray[activeHue][2];
					}
					scheduleChange();
				}
				uint8_t getSingle(const uint8_t num){
					if(num < numberOfLEDs){
						int LEDIndex = keyOffsets[sendNum] + keySizes[sendNum] + num;
						return 
							LEDValues[LEDIndex][0];
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
						// int LEDIndex = keyOffsets[sendNum] + keySizes[sendNum] + i;
						setSingle(i,arr[i]);
					}
					//send();
					scheduleChange();
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
				LEDs.setSingle(i, !Sequencing::getTrack(i).getMuted() ? 3 : 0);
				}
			LEDs.send();
			}
			
		void showNoteMutes(LEDSet& LEDs){
			LEDs.clear();
			for (int i = 0; i<gc::keyNum::notes;i++){
				int theNote = i+(12*interface::keyboardOctave);
				bool muted = (theNote < 128) ? Sequencing::getActiveTrack().isNoteMuted(theNote) : true;
				LEDs.setSingle(i, !muted ? 3 : 0);
				}
			LEDs.send();
			}
			
		void showAvailableTrackBank(LEDSet& LEDs){
			LEDs.clear();
			for (int i = 0; i<gc::keyNum::datas; i++){
				bool isThereAPatternAtThatPosition = Sequencing::getActiveTrack().doesTrackBankHavePattern(i);
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
					int currentPatternStep = Sequencing::getActiveTrack().getCurrentStep();
					int length = Sequencing::getActiveTrack().getStepsPerBar();
					LEDs.setSingleIfBrighter(currentPatternStep, 3);
					if(interface::settings::useFancyLEDChaser){
						LEDs.setSingleIfBrighter((currentPatternStep-1) % length, 2);
						LEDs.setSingleIfBrighter((currentPatternStep-2) % length, 1);
						}
					// LEDs.setSingleIfBrighter(Sequencing::getActiveTrack().getCurrentBar(), 2);
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
			switch(Sequencing::getSequencerStatus()){
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
			for (int i = 0; i < numberOfButtons; i++){
				LEDValues[i][0] = 0;
				LEDValues[i][1] = 0;
				LEDValues[i][2] = 0;
				LEDValues[i][3] = 0;
			}
			// sendLEDs();
		}
		
		void updateLEDs(){
			modes::getActiveMode().updateLEDs();
			stepChaser(steps);
		}
};
#endif