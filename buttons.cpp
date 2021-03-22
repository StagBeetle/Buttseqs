//Reading the input from the buttons in the matrices and attached shift registers and then sending them
#include <vector>
#include <functional>
#include "Utility.h"
#include "forwarddec.h"
#include "Arduino.h"
#include "mode.h"
#include "buttons.h"

#include <SPI.h>

namespace buttons{
	//press release hold
	extern std::function<void()> allButtons[gc::keyNum::extras][buttons::buttonEvent::max]; //Defined in mode definitions.
	
	void switchToMode(const int num, const buttons::buttonEvent::be event){
		static long long lastRelease = millis();
		//lg(" ");
		if(event == buttons::buttonEvent::press){
			//lg("press");
			modes::switchToMode(num);
			scheduled::clearEvent(scheduled::lOE::mode);
		}
		
		if(event == buttons::buttonEvent::release){
			//lg("release");
			if(millis() - lastRelease > interface::settings::doubleClickSpeed){
				//lg("slow");
				scheduled::newEvent(scheduled::lOE::mode, modes::reset, interface::settings::doubleClickSpeed);
			} else {
				scheduled::clearEvent(scheduled::lOE::mode);
				//lg("fast");
			}
			lastRelease = millis();
		}
		
		scrn::setScreenLock(false);
	}
	
	void buttonAllocator(const int buttonNum, const buttons::buttonEvent::be event, const bool heldTriggered){//Maps each of the physical buttons to their functions
		buttons::keySet::ks keys = buttons::keySet::max;
		int shiftedNum = 0;
		//Convert the global button number (0-80) to the button number within each set:
		//Setting up the button parameters for each mode:
		if (buttonNum < gc::keyPos::notes){//Steps
			keys = buttons::keySet::step;
			shiftedNum = buttonNum - 0;
		} 
		else if (buttonNum < gc::keyPos::vertical){//Notes
			keys = buttons::keySet::note;
			shiftedNum = buttonNum - gc::keyPos::notes;
		} 
		else if (buttonNum < gc::keyPos::horizontal){//Up down
			keys = buttons::keySet::vertical;
			shiftedNum = buttonNum - gc::keyPos::vertical;
		} 
		else if (buttonNum < gc::keyPos::datas){//Left rights
			keys = buttons::keySet::horizontal;
			shiftedNum = buttonNum - gc::keyPos::horizontal;
		} 
		else if (buttonNum < gc::keyPos::modes){//data
			keys = buttons::keySet::data;
			shiftedNum = buttonNum - gc::keyPos::datas;
		} 
		//Switching the mode:
		else if (buttonNum < gc::keyPos::extras){//mode
			shiftedNum = buttonNum - gc::keyPos::modes;
			switchToMode(shiftedNum, event);
			return;
		} 
		//The same buttons in all modes:
		else {//other
			shiftedNum = buttonNum - gc::keyPos::extras;
			if(allButtons[shiftedNum][event]){
				allButtons[shiftedNum][event]();
			}
			return;
		}
		//Actually do the function from each mode:
		if (buttonNum < gc::keyPos::modes){//If specific to each mode
			modes::getActiveMode().doAction(keys, shiftedNum, event, heldTriggered);
		}
	}
	
	std::function<void(void)> onButtonEvent = nullFunc;
	const int numberOfButtons = gc::keyNum::total;
	
	class button{
		private:
			bool m_isPressed = false; //If the button is currently held down
			bool m_wasPressed = false; //If the button was held down last time it was checked
			bool m_heldFuncTriggered = false; //If the held function has triggered
			unsigned long long m_timeSincePressed = 0;//millis()
			static const uint8_t debounceTime = 12;
			uint8_t buttonNumber = 0;
		public:
			static int buttonCounter;
			button (){
				buttonNumber = buttonCounter;
				buttonCounter++;
			}
			void m_checkPressed(bool isPressed)
				{//Triggered each cycle
				m_isPressed = isPressed;
				if(!m_wasPressed && m_isPressed && millis() - m_timeSincePressed > debounceTime)
					{ //If the button was not pressed, but it has now been pressed but not pressed recently
					// lgc("pressed:");lg(buttonNumber);
					buttonAllocator(buttonNumber, buttons::buttonEvent::press, false);
					m_wasPressed = true; //Set the button to pressed
					m_timeSincePressed = millis(); //reset the timer
					onButtonEvent();
					}
				if (m_isPressed && millis() - m_timeSincePressed > interface::settings::buttonHoldDelay && !m_heldFuncTriggered)//If it has been held
					{
					//m_function(BA::Hold);
					buttonAllocator(buttonNumber, buttons::buttonEvent::hold, false);
					m_heldFuncTriggered = true;
					onButtonEvent();
					}
				if(m_wasPressed && !m_isPressed && millis() - m_timeSincePressed > debounceTime) //If it was pressed last cycle but not this time
					{ //If it had been pressed but wasn't last loop
					//m_function(BA::Release);
					//lgc("released:");lg(buttonNumber);
					buttonAllocator(buttonNumber, buttons::buttonEvent::release, m_heldFuncTriggered);
					m_wasPressed = false ; //Say the button is not pressed
					m_heldFuncTriggered = false;
					m_timeSincePressed = millis(); //reset the timer
					onButtonEvent();
					}
				}
	};
	int button::buttonCounter = 0;
	
	button buttonArray	[numberOfButtons];
	
	//Timing debugging:
	long long timeAtLastButtonCheck = 0;
	long long longestTimeSinceCheck = 0;
	
	long getAndResetLongestTimeSinceCheck(){
		long long longestTimeSinceCheck_temp = longestTimeSinceCheck;
		longestTimeSinceCheck = 0;
		return longestTimeSinceCheck_temp;
	}
	
	//For debug: control
	volatile bool buttonsDEBUG[numberOfButtons] = {0};//Set by serial
	void setButton(const int num, const int status){
		buttonsDEBUG[num] = status;
	}
	
	//HARDWARE setup:
	auto AUXSPIBUS = SPI1;
	SPISettings shiftreg(2000000, LSBFIRST, SPI_MODE3); 
	const uint8_t BUTTONCS = 2;
	
	const int numberOfShiftRegisters = (numberOfButtons + 7) / 8;
	uint8_t buttonShiftArray[numberOfShiftRegisters] = {0};
	
	PROGMEM void begin(const std::function<void(void)> callback){
		onButtonEvent = callback;
		AUXSPIBUS.begin();
		AUXSPIBUS.setMISO(39);
		pinMode(BUTTONCS, OUTPUT);
		digitalWrite(BUTTONCS, 0);
		}
		 
		 
	//HARDWARE loop
	void loop(const bool recordModeNotesOnly){//recordModeNotesOnly does nothing. DELET THIS
		
		//For timing checks:
		// longestTimeSinceCheck = max(longestTimeSinceCheck, micros() - timeAtLastButtonCheck);
		// timeAtLastButtonCheck = micros();
	
		//SPI the data:
		digitalWrite(BUTTONCS, 1);
		AUXSPIBUS.beginTransaction(shiftreg);
		for(int i = 0; i < numberOfShiftRegisters; i++){
			buttonShiftArray[i] = AUXSPIBUS.transfer(0);
		}
		AUXSPIBUS.endTransaction();
		digitalWrite(BUTTONCS, 0);

		//Process the bytes into a neat little array
		for (int b = 0; b<numberOfButtons; b++){//For each button
			const int index = b/8;
			const int position = b%8;
			const bool isPressed = !bitRead(buttonShiftArray[index], position);
		
			buttonArray[b].m_checkPressed(/*buttonsDEBUG[b] || */isPressed); 
			// if(isPressed)
				// {Serial.println(b);}
		}
	}
		
}//end namespace