
#include "Utility.h"
#include "screen.h"  
#include "setVarAfterDelay.h"  
#include "forwarddec.h"  
#include "list.h"  
//#include <typeinfo>

namespace list{
	
	
		
	std::function<void(int)> setterFunc = nullptr;
	setVarAfterDelay::setDecimal listSetter{nullIntFunc, 0, 0};
	list::listController*	activeList	= nullptr;
	list::liElem*	activeliElem	= nullptr;
	std::function<void(int)> heldListItem = nullptr;
	
	
	//Functions taking a number:
	template<> void sF<int>	::operator()(int n){(void)(n)	; pa();	}
	template<> void sF<volatile bool*>	::operator()(int n){*parameter = !*parameter;(void)(n)	; pa();	}
	template<> void sF<voidvoid>	::operator()(int n){parameter();(void)(n)	; pa();	}
	template<> void sF<voidint>	::operator()(int n){
	//This gap here due to ealstic tabstops
		if(minMax.size() == 0){
			parameter(n);
		} else {
			listSetter.setSetter(parameter);
			listSetter.setConstraints(minMax[0], minMax[1]);
			listSetter.buttonPress(n);
		}
		pa();
	}
	
	//Functions taking no number:
	template<> void sF<>	::operator()(){	}
	template<> void sF<volatile bool*>	::operator()(){*parameter = !*parameter;	pa();	}
	template<> void sF<voidvoid>	::operator()(){parameter();	pa();	}
	template<> void sF<voidint>	::operator()(){	}
	
	template<> void gF<charpvoid>::get(char* str) const{
		strcpy(str, func());
		}
	template<> void gF<intvoid>::get(char* str) const{
		int val = func();
		sprintf(str, "%d", val);
		}
	template<> void gF<boolvoid>::get(char* str) const{
		strcpy(str, values[func()]);
		}
	template<> void gF<volatile bool*>::get(char* str) const{
		strcpy(str, values[*func]);
		}
	template<> void gF<int>::get(char* str) const{
		}
		
		
		
		
		
		
		
		
		
		
		
	void deleteList(std::vector<list::liElem*>* theListVec){//Delete all the elements in a list and the list itself
		for(int16_t i = theListVec->size()-1; i>=0; i--){
			delete (*theListVec)[i];
			}
		delete theListVec;
	}
	
	
	
	
	
	
	
	void clearHeldSetter(){
		// lg("ClearHeldSetter");
		heldListItem = nullptr;
	}
	
	void setHeldSetter(std::function<void(int)> item){
		// lg("SetHeldSetter");
		heldListItem = item;
	}
	
	void useHeldSetter(const int button){
		// lgc("usHeSe:");
		// lg(button);
		heldListItem(button);
		//activeList->draw();
	}
	
	bool isHeldSetter(){
		bool isSetter = heldListItem != nullptr;
		// lg("isHeldSetter?");
		// lg(isSetter ? "yes" : "no");
		return isSetter;
	}
	
	//General list functions:
	void setActiveList(list::listController* newList){
		activeList = newList;
		if(activeList){
			activeList->draw();
			activeliElem = activeList->getActiveListItem();
		} else {
			activeliElem = nullptr;
		}
	}
	
	void jumpTo(const uint8_t button){//Absolute position
		if(activeList){
			activeliElem = activeList->moveTo(button);
		}
		setHeldSetter(activeliElem->getSetter());
	}
	
	void moveList(const int dir){//Up down
		if(activeList){
			activeliElem = activeList->listMove(dir);
		}
	}
	
	void performActionOnActiveListItem(const int button){
		if(activeliElem){
			(*activeliElem)(button);
			if(activeList){
				activeList->draw();
			}
		}
	}
	
	void drawList(){
		if(activeList){
			activeList->draw();
		}
	}

}//End namespace