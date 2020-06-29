//Reading the input from the buttons in the matrices and attached shift registers and then sending them
#ifndef buttons_h
#define buttons_h

#include <functional>

namespace gc{//global constants
	struct keyNum{
		static const int steps	= 16;
		static const int notes	= 25;
		static const int vertical	= 2;
		static const int horizontal	= 2;
		static const int datas	= 16;
		static const int modes	= 16;
		static const int extras	= 4;
		static const int total	= steps + notes + vertical + horizontal + datas + modes + extras;
	};
	
	struct keyPos{
		static const int steps	= 0;
		static const int notes	= steps	+	keyNum::steps;
		static const int vertical	= notes 	+	keyNum::notes;
		static const int horizontal	= vertical 	+	keyNum::vertical;
		static const int datas	= horizontal	+	keyNum::horizontal;
		static const int modes	= datas	+	keyNum::datas;
		static const int extras	= modes	+	keyNum::modes;
		static const int max	= extras	+	keyNum::extras;
	};
}

namespace buttonsSimple{
	class buttonEvent {public: enum be : uint8_t{
		press	= 0,
		release	= 1,
		hold	= 2,
		max	= 3,
	};};
	class keySet {public: enum ks : uint8_t{
		step	= 0,
		note	= 1,
		vertical	= 2,
		horizontal	= 3,
		data	= 4,
		max	= 5, //max different in each mode
		mode	= 6,
		extra	= 7,
		none	= 8,
	};};
	const char* const buttonEventNames [] = {
		"press"	,
		"release"	,
		"hold"	,
		"max"	,
	};
	const char* const keySetNames [] = {
		"step"	,
		"note"	,
		"vertical"	,
		"horizontal"	,
		"data"	,
		"max"	, //max different in each mode
		"mode"	,
		"extra"	,
		"none"	,
	};
	
	
	const uint8_t shiftInSerPar = 2;
	const uint8_t shiftInClock  = 3;
	const uint8_t shiftInData   = 4;
	
	const int numberOfButtons = 81;
	
	int holdDelayTime = 250;

	void setup(){
		pinMode(shiftInClock,OUTPUT);
		pinMode(shiftInSerPar,OUTPUT);
		pinMode(shiftInData,INPUT_PULLUP);
		digitalWriteFast(shiftInSerPar,LOW); //Set the register read the buttons
		digitalWriteFast(shiftInClock,LOW); //Set it low ready for clock. Should be low anyway
		}
		 
	const byte shiftDelay = 1;
	
	volatile bool buttonsDEBUG[numberOfButtons] = {0};
	volatile long long buttonsBounce[numberOfButtons] = {0};
	
	bool getButtonHeld(const int button){
		return buttonsDEBUG[button];
	}
	
	std::function<void(int)> buttonFunctions [8][3];
	
	void buttonAllocator(const int buttonNum, const buttonEvent::be event, const bool heldTriggered){//Maps each of the physical buttons to their functions
		keySet::ks keys = keySet::max;
		int shiftedNum = 0;
		//Convert the global button number (0-80) to the button number within each set:
		
		
		//Setting up the button parameters for each mode:
		if (buttonNum < gc::keyPos::notes){//Steps
			keys = keySet::step;
			shiftedNum = buttonNum - 0;
		} 
		else if (buttonNum < gc::keyPos::vertical){//Notes
			keys = keySet::note;
			shiftedNum = buttonNum - gc::keyPos::notes;
		} 
		else if (buttonNum < gc::keyPos::horizontal){//Up down
			keys = keySet::vertical;
			shiftedNum = buttonNum - gc::keyPos::vertical;
		} 
		else if (buttonNum < gc::keyPos::datas){//Left rights
			keys = keySet::horizontal;
			shiftedNum = buttonNum - gc::keyPos::horizontal;
		} 
		else if (buttonNum < gc::keyPos::modes){//data
			keys = keySet::data;
			shiftedNum = buttonNum - gc::keyPos::datas;
		} 
		//Switching the mode:
		else if (buttonNum < gc::keyPos::extras){//mode
			keys = keySet::mode;
			shiftedNum = buttonNum - gc::keyPos::modes;
		} 
		else {//other
			keys = keySet::extra;
			shiftedNum = buttonNum - gc::keyPos::extras;
		}
		
		//Do the function:
		if(buttonFunctions[keys][event]){
			Serial.println(keySetNames[keys]);
			Serial.println(" ");
			Serial.println(buttonEventNames[event]);
			Serial.println(" ");
			Serial.println(shiftedNum);
			
			buttonFunctions[keys][event](shiftedNum);
		}
	}
	
		
	class button{
		private:
			bool m_isPressed = false; //If the button is currently held down
			bool m_wasPressed = false; //If the button was held down last time it was checked
			bool m_heldFuncTriggered = false; //If the held function has triggered
			unsigned long long m_timeSincePressed = 0;//millis()
			static const byte debounceTime = 12;
			int buttonNumber = 0;
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
					Serial.print("pressed:");Serial.println(buttonNumber);
					buttonAllocator(buttonNumber, buttonEvent::press, false);
					m_wasPressed = true; //Set the button to pressed
					m_timeSincePressed = millis(); //reset the timer
					//onButtonPress();
					}
				if (m_isPressed && millis() - m_timeSincePressed > holdDelayTime && !m_heldFuncTriggered)//If it has been held
					{
					//m_function(BA::Hold);
					buttonAllocator(buttonNumber, buttonEvent::hold, false);
					m_heldFuncTriggered = true;
					//onButtonPress();
					}
				if(m_wasPressed && !m_isPressed && millis() - m_timeSincePressed > debounceTime) //If it was pressed last cycle but not this time
					{ //If it had been pressed but wasn't last loop
					//m_function(BA::Release);
					Serial.print("released:");Serial.println(buttonNumber);
					buttonAllocator(buttonNumber, buttonEvent::release, m_heldFuncTriggered);
					m_wasPressed = false ; //Say the button is not pressed
					m_heldFuncTriggered = false;
					m_timeSincePressed = millis(); //reset the timer
					//onButtonPress();
					}
				}
	};
	int button::buttonCounter = 0;
	
	button buttonArray	[numberOfButtons];

	
	void loop(const bool recordModeNotesOnly = false){//recordModeNotesOnly is a special thing for record mode to stop jitter (I hope)
		digitalWrite(shiftInSerPar, HIGH); //Set the register to serial read mode
		delayMicroseconds(shiftDelay); //Wait a bit - may not be necessary
		for (int i = 0; i < numberOfButtons; i++){//For each button
			const bool isPressed = !digitalRead(shiftInData);
			//buttonsDEBUG[i] = isPressed;
			buttonArray[i].m_checkPressed(isPressed); //If the button is held down, level is low
				//if(isPressed){Serial.write(i);}
			digitalWrite(shiftInClock, LOW); // clock the chip
			delayMicroseconds(shiftDelay); ////Wait a bit - may not be necessary
			digitalWrite(shiftInClock, HIGH); //Clock the chip
			delayMicroseconds(shiftDelay); //Wait a bit - may not be necessary
			}
		digitalWrite(shiftInClock, LOW); //Set the chip low
		digitalWrite(shiftInSerPar,LOW); //Set the register back to parallel mode
	}
		
}//end namespace


#endif