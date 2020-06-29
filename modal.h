//Popups that override all the keys
#ifndef modal_h
#define modal_h
#include "Utility.h"
#include "screen.h"  

namespace modal{
	const int maxOptions = 16;
	std::function<void(void)> savedOptions[maxOptions]; //Option functions pointers can be saved here
	int numberOfUsedOptions = 0;
	//modes::mode* lastMode = &modes::editNotes;
	//SavedStrings ?
	void clearModalFunctionPointers(){
		for(int i = 0; i< maxOptions; i++){
			savedOptions[i] = nullFunc;
			}
		}
	void endModal(){
		clearModalFunctionPointers();
		modes::reset();
		}
	void doFunction(const int num){
		if (num < numberOfUsedOptions){//If the button pressed is within bounds
			savedOptions[num]();
			endModal();
			}
		}
	class option{
			public:
				static int getMaxStringLength(){
					return maxStringLength;
					}
				option(const char* c_string, const std::function<void(void)> c_callback) : 
					callback(c_callback),
					displayString(c_string){}
				std::function<void(void)> getFunction() const{
					return callback;
					}
				void getString (char* outString) const{
					strncpy(outString, displayString, maxStringLength);
					}
				const char* getStringPointer () const{
					return displayString;
					}
			private:
				const static int maxStringLength = 16;
				const std::function<void(void)> callback;
				const char* displayString; //I think this char references flash memory
		};
	void initiateModal(const char* choice, std::vector<option> options, modes::mode& returnTo){
		clearModalFunctionPointers();
		modes::switchToMode(modes::modal, false);
		//lastMode = &returnTo;
		scrn::write(5, 5, choice);
		const int numberOfOptions = options.size(); //Do a check to ensure this is less than maxOptions
		numberOfUsedOptions = numberOfOptions;
		std::vector<const char*> strings;
		for (int i = 0; i<numberOfOptions; i++){
			savedOptions[i] = options[i].getFunction();
			// char optionString[option::getMaxStringLength()+1];
			// options[i].getString(optionString);
			// int yline = 20*i;
			// scrn::write(5, 30 + yline, i);
			// scrn::write(30, 30 + yline, optionString);
			strings.push_back(options[i].getStringPointer());
			}		
		notifications::notification n = {choice};
		n.display(strings);
		};
	std::function<void(int)> modalNumFunction;
	void doNumFunction(const int num){
		modalNumFunction(num);
		modes::reset();
		}
	void initiateNumberModal(const char* choice, std::function<void(int)> theFunction){
		modes::switchToMode(modes::modalNum, false);
		//lastMode = &returnTo;
		modalNumFunction = theFunction;
		notifications::notification n = {choice};
		n.display();
		};
}
#endif