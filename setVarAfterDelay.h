#ifndef setVarAfterDelay_h
#define setVarAfterDelay_h
#include "scheduled.h"

namespace setVarAfterDelay{
	class setDecimal 
	{
		public:
			std::function<void(int)> variableSetter; // The setter to call to set the variable
			int minValue; // The minimum value that the variable can take
			int maxValue; // The maximum value that the variable can take
			static const long int lengthBeforeTimeout = 1000 ; //the time in millis it takes of no activity before the value is accepted 
			signed int value = 0 ; // The value to be set;
			signed int negativeValue = 1 ;//-1 or 1 representing whether it is negative or not - will be toggled by a key
			bool hasSet = false;
			template<typename F>
			setDecimal(const F c_variableSetter, const int c_minValue, const int c_maxValue) //Constructor
				{
				variableSetter = c_variableSetter;
				minValue = c_minValue;
				maxValue = c_maxValue;
				}			
			setDecimal(){
				variableSetter = nullIntFunc;
				minValue = 0;
				maxValue = 9;
			}
			void setConstraints(const int c_minValue, const int c_maxValue){
				//reset();
				//value = 0;
				minValue = c_minValue;
				maxValue = c_maxValue;
				}
			void setSetter(std::function<void(int)> func){
				variableSetter = func;
			}
			void reset(){ //Reset the object to how it was at start
				//value = 0 ;
				negativeValue = 1;
				scheduled::clearEvent(scheduled::lOE::setVarAfterDelay); //The timing event will have already deleted itself 
			}
			void setVariable(){
				value *= negativeValue; // Make the value negative if it has to be
				value = constrain(value,minValue,maxValue); // Constrain the number. Maybe not needed due to setter
				variableSetter(value);
				reset(); //Reset all the variables in the struct so the function can run from scratch
				hasSet = true;
				}
			signed int buttonPress(int button){ // What to do when the button is pressed;
				scheduled::clearEvent(scheduled::lOE::setVarAfterDelay);
				scheduled::newEvent(scheduled::lOE::setVarAfterDelay, [this](){this->setVariable();}, lengthBeforeTimeout);
				if(button >= 0 && button <= 9){ //If the input is a digit
					if(hasSet){
						hasSet = false;
						value = 0;
						}
					value = value*10 + button; // Append a digit to a number
					//If num >= ceiling(maxVal/10) then it is ready to be converted
					if (value >= ((maxValue + 9)/10)){
						scheduled::clearEvent(scheduled::lOE::setVarAfterDelay);
						setVariable();
						return value;
						}
					}
				else //If the parameter is not a digit, it should make the number negative:
					{
					if(minValue < 0){//If the value can be negative, try make number become negative, otherwise, don't bother
						negativeValue = -1 * negativeValue;
						if(hasSet){
							setVariable();
							}
						}	//Make the number negative
					}
				return negativeValue * value; //Returns in-progress value for display
				}
	};


	class setHex{
		public:
			std::function<void(int)> variableSetter; // The setter to call to set the variable
			static const long int lengthBeforeTimeout = 500 ; //the time in millis it takes of no activity before the value is accepted 
			signed int value = 0 ; // The value to be set;
			uint8_t maxPresses = 1;
			uint8_t numPresses = 0;
			template<typename F>
			
			setHex(F c_variableSetter, const int p_maxPresses){ //Constructor
				variableSetter = c_variableSetter;
				maxPresses = p_maxPresses;
			}
			
			void reset(){ //Reset the object to how it was at start
				value = 0 ;
				numPresses = 0;
				scheduled::clearEvent(scheduled::lOE::setVarAfterDelay); //The timing event will have already deleted itself 
			}
			
			void setVariable(){
				variableSetter(value);
				reset(); //Reset all the variables in the struct so the function can run from scratch
			}
			
			void buttonPress(uint8_t button){ // What to do when the button is pressed;
				//setVariableEvent.invalidate();
				scheduled::clearEvent(scheduled::lOE::setVarAfterDelay);
				scheduled::newEvent(scheduled::lOE::setVarAfterDelay, [this](){this->setVariable();}, lengthBeforeTimeout);
				numPresses++;
				value += utl::power(16, maxPresses - numPresses) * button;
				if(numPresses == maxPresses){
					scheduled::clearEvent(scheduled::lOE::setVarAfterDelay);
					setVariable();
					}
			}
	};
}//end namespace
#endif