#include <bitset>
#include "Utility.h"
#include "buttons.h"
#include "mode.h"

#include <stdint.h>
#include <stdio.h>
#include "Arduino.h"
#include "utility.h"
#include "forwardDec.h"

namespace modes{
	
//const int numberOfModes = 40;
//class mode;
dialogType currentDialog = dialogType::none;

mode* listOfModes[numberOfModes] = {nullptr};
	
		mode::mode(const char* c_name, const modeType c_type,
		const bool c_enableChase, const bool c_doNotReturn, const bool c_clearScreen, const bool c_labelled,  const bool c_allButtonsOverride,
		list::listController* c_activeList, 
		const std::vector<buttons::keySet::ks> c_heldFuncSetter,
		const std::function<void()> c_screenDisplayFunc, 
		const std::vector<LEDParam> LEDs,
		const std::vector<buttonParam> buttons,
		const std::vector<std::function<void(int)>> encoders
		) :
			name(c_name),
			type(c_type),
			screenDisplayFunc(c_screenDisplayFunc),
			activeList(c_activeList),
			heldFuncSetterKeysets(c_heldFuncSetter),
			usesHeldSetter(heldFuncSetterKeysets.size()),
			enableChase(c_enableChase),
			doNotReturn(c_doNotReturn),
			clearScreen(c_clearScreen),
			labelled(c_labelled),
			allButtonsOverride(c_allButtonsOverride),
			ID(counter){
				for(const auto b : buttons){
					actions[b.key][b.event] = b.function;
				}
				for(const auto l : LEDs){
					LEDDisplays[l.key] = l.function;
				}
				for(unsigned int i = 0; i < encoders.size(); i++){
					encoderFunctions[i] = encoders[i];
				}
				listOfModes[ID] = this;
				counter++;
			}

		void mode::doAction(const buttons::keySet::ks key, int number, const buttons::buttonEvent::be event, const bool heldFuncTriggered){
			// if(!external){
				// switch(key){
					// case buttons::keySet::ks::note:
						// number = utl::gPFK(number);
						// break;
					// default:
						// break;
				// }
			// }
			//Reset the heldSetter in most cases:
			if(event == buttons::buttonEvent::release){//Reset held setter:
				if(!usesHeldSetter || key != heldFuncSetterKeysets[1]){
					list::clearHeldSetter();
					// lg("clear");
				}
			} 
			if(currentDialog != dialogType::none){
				if( //If it is a data, step or note key
					key == buttons::keySet::step ||
					key == buttons::keySet::note ||
					key == buttons::keySet::data
				){
					switch(currentDialog){
						case dialogType::error:
							interface::all::exitError(number);
							break;
						case dialogType::modal:
							interface::modals::doModalFunction(number);
							break;
						case dialogType::modalNum:
							interface::modals::doModalNumFunction(number);
							break;
						default:
							break;
					}
				}
				return;
			}
			//If the note +  up down buttons should be taken over by recording:
			if(interface::isRecording() && !interface::settings::recordOnlyFromExternal){
				if(key == buttons::keySet::ks::note 	&& event == buttons::buttonEvent::be::press	){	interface::record::startNote(number);}
				if(key == buttons::keySet::ks::note 	&& event == buttons::buttonEvent::be::release	){	interface::record::endNote(number);}
				if(key == buttons::keySet::ks::vertical 	&& event == buttons::buttonEvent::be::press	){	interface::all::keyboardOctShift(number);}
				return;
			}
			if(!allButtonsOverride){
				// lgc("heldSetter:");
				// lg(usesHeldSetter);
				// if(usesHeldSetter){
					// lgc("keySet1:");
					// lgc(static_cast<int>(heldFuncSetterKeysets[1]));
				// }
				
				//Activate the held setter function if a heldsetter is pressed
				if (usesHeldSetter && key == heldFuncSetterKeysets[1] && list::isHeldSetter() && event == buttons::buttonEvent::press){//Use held setter
					// lg("UseHeldSetter");
					list::useHeldSetter(number);
				}
				//Do the normal function
				else if (actions[key][event]){//If there is a function there
					if(event == buttons::buttonEvent::release && actions[key][buttons::buttonEvent::hold] && heldFuncTriggered){
						return; //Do not do release function if there is a hold function and it has triggered - this is for editNotes change bar and add note
					}
					actions[key][event](number);
				}
			} else {//All buttons override:
				if(key == buttons::keySet::ks::vertical || key == buttons::keySet::ks::horizontal){
					if (actions[key][event]){
						actions[key][event](number);
					}
				}
				else if (actions[buttons::keySet::step][event]){//If there is a step function there when using all buttons override
					int newNum;
					switch (key){
						case buttons::keySet::step:
							newNum = number;
							break;
						case buttons::keySet::note:
							newNum = number + gc::keyPos::notes;
							break;
						case buttons::keySet::data:
							newNum = number + gc::keyPos::vertical;
							break;
						default:
							newNum = number;
					}
					// lg(newNum);
					actions[buttons::keySet::step][event](newNum);
				}
			}
		}
		bool mode::allowChase() const {
			return enableChase;
		}
		const char* mode::getName() const{
			return name;
		}
		int mode::getID() const{
			return ID;
		}
		modeType mode::getType() const{
			return type;
		}
		bool mode::getDoNotReturn(){
			return doNotReturn;
		}
		void mode::updateDisplay(){
			screenDisplayFunc();
			list::setActiveList(activeList);
			updateLEDs();
		}
		void mode::updateLEDs(){
			LEDfeedback::clearAll();
			if(LEDDisplays[buttons::keySet::step	]){LEDDisplays[buttons::keySet::step	](LEDfeedback::getLEDSet(buttons::keySet::step	));}
			if(LEDDisplays[buttons::keySet::note	]){LEDDisplays[buttons::keySet::note	](LEDfeedback::getLEDSet(buttons::keySet::note	));}
			if(LEDDisplays[buttons::keySet::vertical	]){LEDDisplays[buttons::keySet::vertical	](LEDfeedback::getLEDSet(buttons::keySet::vertical	));}
			if(LEDDisplays[buttons::keySet::horizontal	]){LEDDisplays[buttons::keySet::horizontal	](LEDfeedback::getLEDSet(buttons::keySet::horizontal	));}
			if(LEDDisplays[buttons::keySet::data	]){LEDDisplays[buttons::keySet::data	](LEDfeedback::getLEDSet(buttons::keySet::data	));}
		}
		void mode::switchTo(){
			if(clearScreen)	{scrn::blankScreen();}
			if(labelled)	{draw::drawMode();}
			updateDisplay();
		}
		
		void mode::forEachFunction(std::function<void(buttons::keySet::ks, buttons::buttonEvent::be, voidint)> f ){
			for(int b=0; b<static_cast<int>(buttons::keySet::max); b++){
				for(int e=0; e<static_cast<int>(buttons::buttonEvent::max); e++){
					if(actions[b][e] != nullptr){
						f(static_cast<buttons::keySet::ks>(b), static_cast<buttons::buttonEvent::be>(e), actions[b][e]);
					}
				}
			}
		}
		
		void mode::doEncoderFunction(const int number, const int change){
			if(encoderFunctions[number]){
				encoderFunctions[number](change);
			}
		}
		
		
mode* activeMode = &settings;
mode* lastMode = &settings;

uint8_t mode::counter = 0;
		
bool operator == (const mode& one, const mode& other){
	return one.getID() == other.getID();
	
}
bool operator != (const mode& one, const mode& other){
	return !(one == other);
}

mode* buttons[gc::keyNum::modes] = {
	&editNotes	,
	&editSteps	,
	&editPattern	,
	&editCC	,
	&patternProcess	,
	&mute	,
	&transpose	,
	&settings	,
	&copy	,
	&process	,
	&patternUtils	,
	&patternSwitch	,
	&arrange	,
	&MIDIRouting	,
	&themeEdit	,
	&modeSelect	,
};

void forEachMode(const std::function<void(mode&)> func){
	for (auto& m : listOfModes){
		if(!m){return;}
		func(*m);
	}
}

mode& getModeByID(const int ID){
	mode* m = listOfModes[ID];
	return m ? *m : sentinel;
}

bool isValid(mode& m){
	return m != sentinel;
}

mode& getActiveMode(){
	return *activeMode;
}

bool checkActive(const mode& m){
	return m == *activeMode;
}

/*template<typename T>*/bool checkActive (std::vector<mode> modesToCheck){
	for(const auto& m: modesToCheck){
		// lgc(m.getName());
		// lgc(m.getID());
		// lgc("?=");
		// lgc(activeMode->getName());
		// lg(activeMode->getID());
		if(m == *activeMode){/*lg("true"); */return true;}
	}
	// lg("false"); 
	return false;
}

mode& getLastMode(){
	return *lastMode;
}

void switchToMode(mode& newMode, const bool setLast){
	lastMode = activeMode;
	activeMode = &newMode;
	modal::clearModalFunctionPointers();
	newMode.switchTo();
	LEDfeedback::showMode(LEDfeedback::getLEDSet(buttons::keySet::mode));
	LEDfeedback::showExtras(LEDfeedback::getLEDSet(buttons::keySet::extra));
	if(setLast){
		lastMode = &newMode;
	}
}

void switchToMode(const int pos, const bool fixed){
	switchToMode(*buttons[pos], fixed);
}

void reset(){
	if(lastMode != activeMode && !(*lastMode).getDoNotReturn()){
		switchToMode(*lastMode, true);
	}
	lastMode = activeMode;
}

const char* getModeString(){
	return getActiveMode().getName();
}

void setDialog(const dialogType type){//Has an error or a modal popped up
	currentDialog = type;
}
void clearDialog(){
	currentDialog = dialogType::none;
}

void encoderAllocator(const int number, const int change){
	getActiveMode().doEncoderFunction(number, change);
}

}//end namespace