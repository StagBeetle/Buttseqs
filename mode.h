#ifndef modes_h
#define modes_h
#include <bitset>
#include <vector>
#include "Utility.h"
#include "forwarddec.h"

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
		);
		mode& operator=(const mode&) = delete;
    //mode(const mode&) = delete;
		void doAction(const buttons::keySet::ks key, int number, const buttons::buttonEvent::be event, const bool heldFuncTriggered);
		bool allowChase() const ;
		const char* getName() const;
		int getID() const;
		modeType getType() const;
		bool getDoNotReturn();
		void updateDisplay();
		void updateLEDs();
		void switchTo();
		
		void forEachFunction(std::function<void(buttons::keySet::ks, buttons::buttonEvent::be, voidint)> f );
		
		void doEncoderFunction(const int number, const int change);
		
};
extern bool operator == (const mode& one, const mode& other);
extern bool operator != (const mode& one, const mode& other);
extern mode editNotes, editSteps, editPattern, editCC, patternProcess, mute, transpose, settings, chain, performance, patternUtils, patternSwitch, rename, cardView/*, modal, modalNum, error*/, copy, themeEdit, arrange, quantise, MIDIRouting, shiftNotes, debug, memoryInspect, selection, modeSelect, LFO, process, sentinel;

extern mode* buttons[gc::keyNum::modes];

extern mode& getActiveMode();
extern bool checkActive(const mode& m);
extern bool checkActive (std::vector<mode> modesToCheck);
extern mode& getLastMode();
extern void switchToMode(mode& newMode, const bool setLast = false);
extern void switchToMode(const int pos, const bool fixed = false);
extern void reset();
extern const char* getModeString();
extern void encoderAllocator(const int number, const int change);

// extern listOfModes* getListOfModes();
// extern int getnumberOfModes();
extern void forEachMode(const std::function<void(mode&)> func);
extern mode& getModeByID(const int ID);
extern bool isValid(mode& m);

extern void setDialog(const dialogType type);//Has an error or a modal popped up
extern void clearDialog();//Has an error or a modal popped up

const static int numberOfModes = 40;

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

}//end namespace
#endif