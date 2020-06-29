#ifndef modes_h
#define modes_h
#include <bitset>
#include "Utility.h"

namespace modes{
	
enum class modeType : uint8_t{
	creation 	,
	live	,
	process	,
	admin	,
	unlisted	,
};

const char* const modeTypeNames[] = {
	"creation"	,
	"live"	,
	"process"	,
	"admin"	,
};

class buttonParam{ public:
	buttons::keySet::ks key;
	buttons::buttonEvent::be event;
	//std::function<void(const uint8_t)> function;
	voidint function;
	buttonParam(buttons::keySet::ks c_key, buttons::buttonEvent::be c_event, voidint c_function) :
		key(c_key),
		event(c_event),
		function(c_function){}
};

class LEDParam{ public:
	buttons::keySet::ks key;
	std::function<void(LEDfeedback::LEDSet&)> function;
	LEDParam(buttons::keySet::ks c_key, std::function<void(LEDfeedback::LEDSet&)> c_function) :
		key(c_key),
		function(c_function){}
};
const int numberOfModes = 40;
class mode;
mode* listOfModes[numberOfModes] = {nullptr};

class mode{
	private:
		const char*	name;
		modeType	type;
		std::function<void()>	screenDisplayFunc;
		voidint	actions[buttons::keySet::max][buttons::buttonEvent::max];
		//std::function<void(const uint8_t)>	actions[buttons::keySet::max][buttons::buttonEvent::max];
		std::function<void(LEDfeedback::LEDSet&)>	LEDDisplays[buttons::keySet::max];
		std::function<void(int)>	encoderFunctions[gc::numberOfEncoders] = {};
		list::listController*	activeList	= nullptr;
		const std::vector<buttons::keySet::ks> 	heldFuncSetterKeysets	= {};//First is where the setter will be called, second is what it will be set with
		const bool	usesHeldSetter	= false;
		const bool	enableChase	= false;	//Show activePattern chaser LEDs on steps
		const bool	doNotReturn	= false;	//Can this be used as last function
		const bool	clearScreen	= true;	//Clear screen on switch
		const bool	labelled	= true;	//Should a label be displayed on switch
		const bool	allButtonsOverride	= false;	//All buttons delegate to the step functions
		const int	ID;
	public:
		static uint8_t counter;
		mode(const char* c_name, const modeType c_type,
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
		mode& operator=(const mode&) = delete;
    //mode(const mode&) = delete;
		void doAction(const buttons::keySet::ks key, int number, const buttons::buttonEvent::be event, const bool heldFuncTriggered){
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
			//If the note/udown buttons should be taken over by recording:
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
		bool allowChase() const {
			return enableChase;
		}
		const char* getName() const{
			return name;
		}
		int getID() const{
			return ID;
		}
		modeType getType() const{
			return type;
		}
		bool getDoNotReturn(){
			return doNotReturn;
		}
		void updateDisplay(){
			screenDisplayFunc();
			list::setActiveList(activeList);
			updateLEDs();
		}
		void updateLEDs(){
			LEDfeedback::clearAll();
			if(LEDDisplays[buttons::keySet::step	]){LEDDisplays[buttons::keySet::step	](LEDfeedback::getLEDSet(buttons::keySet::step	));}
			if(LEDDisplays[buttons::keySet::note	]){LEDDisplays[buttons::keySet::note	](LEDfeedback::getLEDSet(buttons::keySet::note	));}
			if(LEDDisplays[buttons::keySet::vertical	]){LEDDisplays[buttons::keySet::vertical	](LEDfeedback::getLEDSet(buttons::keySet::vertical	));}
			if(LEDDisplays[buttons::keySet::horizontal	]){LEDDisplays[buttons::keySet::horizontal	](LEDfeedback::getLEDSet(buttons::keySet::horizontal	));}
			if(LEDDisplays[buttons::keySet::data	]){LEDDisplays[buttons::keySet::data	](LEDfeedback::getLEDSet(buttons::keySet::data	));}
		}
		void switchTo(){
			if(clearScreen)	{scrn::blankScreen();}
			if(labelled)	{draw::drawMode();}
			updateDisplay();
		}
		
		void forEachFunction(std::function<void(buttons::keySet::ks, buttons::buttonEvent::be, voidint)> f ){
			for(int b=0; b<static_cast<int>(buttons::keySet::max); b++){
				for(int e=0; e<static_cast<int>(buttons::buttonEvent::max); e++){
					if(actions[b][e] != nullptr){
						f(static_cast<buttons::keySet::ks>(b), static_cast<buttons::buttonEvent::be>(e), actions[b][e]);
					}
				}
			}
		}
		
		void doEncoderFunction(const int number, const int change){
			if(encoderFunctions[number]){
				encoderFunctions[number](change);
			}
		}
		
};
bool operator == (const mode& one, const mode& other){
	// lg("{");
	// lgc(" ");
	// lg(one.getID());
	// lgc(" ");
	// lg(other.getID());
	// lg("}");
	return one.getID() == other.getID();
	
}
bool operator != (const mode& one, const mode& other){
	return !(one == other);
}
uint8_t mode::counter = 0;
extern mode editNotes, editSteps, editPattern, editCC, patternProcess, mute, transpose, settings, chain, performance, patternUtils, patternSwitch, rename, cardView, modal, modalNum, error, copy, themeEdit, arrange, quantise, MIDIRouting, shiftNotes, debug, memoryInspect, selection, modeSelect, LFO, process;

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

mode* activeMode = &settings;
mode* lastMode = &settings;

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
	scrn::setScreenLock(false);
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

class modeField{
	private:
		std::bitset<numberOfModes> m_modeField;
	public:
		modeField(const std::vector<mode> c_modeField){
			for (const mode& m: c_modeField){
				m_modeField.set(m.getID());
				}
			}
		modeField(const bool a){
			m_modeField.set();
		}
		bool isInField(const mode& m) const{
			return m_modeField.test(m.getID());
			}
};

void encoderAllocator(const int number, const int change){
	getActiveMode().doEncoderFunction(number, change);
}

}//end namespace
#endif