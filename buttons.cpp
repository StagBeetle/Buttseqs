//Reading the input from the buttons in the matrices and attached shift registers and then sending them
#include <vector>
#include <functional>
#include "Utility.h"
#include "forwarddec.h"
#include "Arduino.h"
#include "mode.h"
#include "buttons.h"

namespace buttons{
	//press release hold
	extern std::function<void()> allButtons[gc::keyNum::extras][buttons::buttonEvent::max]; //Defined in mode definitions.
	
	void switchToMode(const int num, const buttons::buttonEvent::be event){
		static long long lastRelease = millis();
		lg(" ");
		if(event == buttons::buttonEvent::press){
			lg("press");
			modes::switchToMode(num);
			scheduled::clearEvent(scheduled::lOE::mode);
		}
		
		if(event == buttons::buttonEvent::release){
			lg("release");
			if(millis() - lastRelease > interface::settings::doubleClickSpeed){
				lg("slow");
				scheduled::newEvent(scheduled::lOE::mode, modes::reset, interface::settings::doubleClickSpeed);
			} else {
				scheduled::clearEvent(scheduled::lOE::mode);
				lg("fast");
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
	
	const pin shiftInSerPar = 2;
	const pin shiftInClock  = 3;
	const pin shiftInData   = 4;
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
					//lgc("pressed:");lg(buttonNumber);
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
	// button dataButtonArray	[matrix::numberOfButtonsPerMatrix];
	// button modeButtonArray	[matrix::numberOfButtonsPerMatrix];
	// button extraButtonArray	[gc::keyNum::extras];
	
	void setup(const std::function<void(void)> callback){
		// shiftInSerPar = serPar;
		// shiftInClock  = clock;
		// shiftInData   = data;
		onButtonEvent = callback;
		pinMode(shiftInClock,OUTPUT);
		pinMode(shiftInSerPar,OUTPUT);
		pinMode(shiftInData,INPUT_PULLUP);
		digitalWrite(shiftInSerPar,LOW); //Set the register read the buttons
		digitalWrite(shiftInClock,LOW); //Set it low ready for clock. Should be low anyway
		}
		 
	const byte shiftDelay = 1;
	
	long long timeAtLastButtonCheck = 0;
	long long longestTimeSinceCheck = 0;
	
	long getAndResetLongestTimeSinceCheck(){
		long long longestTimeSinceCheck_temp = longestTimeSinceCheck;
		longestTimeSinceCheck = 0;
		return longestTimeSinceCheck_temp;
	}
	
	volatile bool buttonsDEBUG[numberOfButtons] = {0};//Set by serial
	
	volatile bool buttonsOutput[numberOfButtons] = {0};//Printed to Serial
	
	void loop(const bool recordModeNotesOnly){//recordModeNotesOnly is a special thing for record mode to stop jitter (I hope)

		longestTimeSinceCheck = max(longestTimeSinceCheck, micros() - timeAtLastButtonCheck);
		timeAtLastButtonCheck = micros();
		digitalWrite(shiftInSerPar, HIGH); //Set the register to serial read mode
		delayMicroseconds(shiftDelay); //Wait a bit - may not be necessary
		
		for (int b = 0; b<numberOfButtons; b++){//For each button
			//if((b >= gc::keyPos::notes && b < gc::keyPos::horizontal) || !recordModeNotesOnly){//If a note button or just a normal mode:
			volatile bool isPressed = !digitalRead(shiftInData);
				
				//b.m_checkPressed(isPressed);// || pythonButtons::pythonButtons[i]); //If the button is held down, level is low
			buttonArray[b].m_checkPressed(buttonsDEBUG[b] || isPressed); 
			//buttonsOutput[b] = isPressed;
			//if(isPressed){lg(b);}
			//}
			digitalWrite(shiftInClock, LOW); // clock the chip
			delayMicroseconds(shiftDelay); ////Wait a bit - may not be necessary
			digitalWrite(shiftInClock, HIGH); //Clock the chip
			delayMicroseconds(shiftDelay); //Wait a bit - may not be necessary
			}
		digitalWrite(shiftInClock,  LOW); //Set the chip low
		digitalWrite(shiftInSerPar, LOW); //Set the register back to parallel mode
		
	}
		
}//end namespace