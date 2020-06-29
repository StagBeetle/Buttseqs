//Lists that show on screen
#ifndef list_h
#define list_h
#include "Utility.h"
#include "screen.h"  
#include "setVarAfterDelay.h"  
//#include <typeinfo>

namespace list{
	enum class listType{
		numbered, 
		normal,
		fullkeyed,//Shows the keys needed to switch to this thing e.g. 1, 5.. Intended to be for combos but combos are a bad idea
		modal
		};
	//const int dataXCoord = 300;
	
	std::function<void(int)> setterFunc = nullptr;
	setVarAfterDelay::setDecimal listSetter{nullIntFunc, 0, 0};
	
	enum class liElemType : uint8_t{
		plain	,
		dynamic	,
		function	,
		dataChar	,
		dataInt	,
		dataBool	,
	};
	
	//When setting a variable in a list, what mode to use
	enum class setModes : uint8_t{
		sixteen,
		decimal,
		enumerated,
		binary,
		};
		
	// template<class f = int>class sF;
	// sF<>* heldSetter = nullptr;
		
	template<class f = int>class sF{//Initialise with an int for no variable
		public:
			sF(volatile f c_parameter = 0, std::function<void()> c_postAction = nullptr, std::vector<int> c_minMax = {}) : 
				parameter(c_parameter),
				postAction(c_postAction),
				minMax(c_minMax){}
			sF() = default;
			void operator()(int n);
			void operator()();
			void pa(){
				//heldSetter = this;
				if(postAction){postAction();}
			}
		private:
			const f parameter;
			const std::function<void()> postAction;
			const std::vector<int> minMax;
	};
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
	
	template<class f = int>class gF{//Initialise with an int for no variable
		public:
			gF(volatile f c_func = 0, std::vector<const char*> c_values = {}) : 
				func(c_func),
				values(c_values){}
			void get(char*) const;
		private:
			const f func;
			const std::vector<const char*> values;
	};
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

	
	/*
	template<class f, class g>class gs{
		public:
			//Boolean function setter:
			template<class f, class g> gs(f c_setter, g c_getter, std::vector<char*> c_values = {}, std::function<void()> c_postAction = nullptr) : 
				setter(c_setter, c_postAction),
				getter(c_getter, c_values)
				{}
			//Single functions:
			template<class f> gs(f c_setter) : 
				setter(c_setter),
				getter(0)
				{}
			//Int set variables:
			template<class f, class g> gs(f c_setter, g c_getter, std::vector<int>c_minMax) : 
				setter(c_setter, nullptr, c_minMax),
				getter(c_getter)
				{}
			//Boolean variables:
			gs(bool& var, std::vector<char*> c_values = {}, std::function<void()> c_postAction = nullptr) : 
				setter(&var, c_postAction),
				getter(&var, c_values)
				{}
			//PatMem & card
			gs() : 
				setter(0), 
				getter(0) 
				{}
			
			void operator()(int n){
				setter(n);
			}
			void operator()(){
				setter();
			}
			void get(char* str){
				getter.get(str);
			}
		private:
			sF<f> setter;
			gF<g> getter;
	};
	*/
	
	class liElem{
		protected:
			const static int maxStringLength = 24;
			const static int maxDataLength = 12;
		public:
			constexpr static int getMaxStringLength(){return maxStringLength;}
			constexpr static int getMaxDataLength(){return maxDataLength;}
			virtual ~liElem(){}
			virtual uint16_t getNumber() const = 0;
			virtual void getName(char* outString) const = 0;
			virtual void getData(char* outString) const = 0;
			const char* getNamePointer() const{return nullptr;}//Override by dynamic elements
			virtual void operator()(int n) = 0;
			virtual void operator()() = 0;
			virtual std::function<void(int)> getSetter() const{
				return nullptr;
			}
	};
	
	
	template<class s0, class g0> class liElemF : public liElem{
		private:
			const char*	name;
			s0 setter;
			g0	getter;
		public:
			//Boolean variables:
			template<class b>
			liElemF(const char* c_name, volatile b& var, std::vector<const char*> c_values, std::function<void()> c_postAction = nullptr) : 
				name(c_name),
				setter(reinterpret_cast<volatile bool*>(&var), c_postAction),//So it can change bools and binary enums the same way
				getter(reinterpret_cast<volatile bool*>(&var), c_values)
				{}
			//Single functions:
			template<class s>
			liElemF(const char* c_name, s c_setter, std::function<void()> c_postAction = nullptr, const bool lazyProgramming = true) : //I think lazyProgramming is so this constructor is different from the above constructor
				name(c_name),
				setter(c_setter, c_postAction),
				getter()
				{}
			//Boolean function setter:
			// template<class s, class g> liElemF(char* c_name, s c_setterFunc, g c_getterFunc, std::vector<char*> c_values = {}, std::function<void()> c_postAction = nullptr) : 
				// setter(c_setter, c_postAction),
				// getter(c_getter, c_values)
				// {}
			//Int set variables:
			template<class s, class g> 
			liElemF(const char* c_name, s c_setter, g c_getter, std::vector<int>c_minMax = {}) : 
				name(c_name),
				setter(c_setter, nullptr, c_minMax),
				getter(c_getter)
				{}
			
			~liElemF() = default;
			
			uint16_t getNumber() const{return 0;}//unused
			void getName(char* outString) const{
				strcpy(outString, name);
			}
			void getData(char* outString) const{
				getter.get(outString);
			}
			void operator()(int n){
				setter(n);
			}
			void operator()(){
				setter();
			}
			std::function<void(int)> getSetter() const{
				return setter;
			}
	};
	
	class liElemD : public liElem{
		protected:
			const int num;
			char name[maxStringLength+1];
		public:
			liElemD(const char* c_name, const int c_num = 0) :
				num(c_num){
				strncpy(name, c_name, maxStringLength);
				}
			~liElemD() = default;
			uint16_t getNumber() const{return num;}
			void getName(char* outString) const{
				strcpy(outString, name);
			}
			const char* getNamePointer() const{
				return name;
			}
			void getData(char* outString) const{}//Unused 
			void operator()(int n){}//Unused 
			void operator()(){}//Unused 
	};
	
	
	
	void deleteList(std::vector<list::liElem*>* theListVec){//Delete all the elements in a list and the list itself
		for(int16_t i = theListVec->size()-1; i>=0; i--){
			delete (*theListVec)[i];
			}
		delete theListVec;
	}
	
	class listController{
		protected:
			uint16_t position = 0;
			std::vector<list::liElem*>* liElems; //Be careful with pointers
			uint16_t length; 
			const listType type; //How to number the list
			uint16_t yOffset; //How far down the screen it displays
			uint16_t xOffset; //How far along the screen it displays
			uint16_t clearWidth; //Width to clear
			const bool allowScroll = false; //Should it scroll?
			uint8_t maxLines = 8;
			const uint16_t dataXCoord = 240;
			const bool showActive = true;
		public:
			listController(std::vector<list::liElem*>* c_liElems, const listType c_type, const int c_yOffset, const int c_xOffset, const int c_clearWidth, const bool c_allowScroll, const int c_maxLines, const int c_dataXCoord, const bool c_showActive) : 
				liElems(c_liElems),
				length(c_liElems == nullptr ? 0 : c_liElems->size()),
				type(c_type),
				yOffset(c_yOffset),
				xOffset(c_xOffset),
				clearWidth(c_clearWidth),
				allowScroll(c_allowScroll),
				maxLines(c_maxLines),
				dataXCoord(c_dataXCoord),
				showActive(c_showActive)
				{}
			
			virtual void draw(){
				scrn::writeFillRect(xOffset, scrn::topOffset + yOffset, clearWidth, maxLines * scrn::font::getTextHeight(), scrn::getThemeColour(scrn::td::bg));
				scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
				int startPoint = allowScroll ? position : 0;
				int listEnd = min(startPoint + maxLines, length);
				int lineNumber = 0;
				
				for (int i = startPoint; i<listEnd; i++){
					char listString[liElem::getMaxStringLength()+1] = {0};
					(*liElems)[i]->getName(listString);
					int yline = scrn::topOffset + yOffset + (scrn::font::getTextHeight()*lineNumber);
					if(showActive && i == position){//Show a bar next to the active element
						scrn::writeFillRect(xOffset, yline, 5, scrn::font::getTextHeight(), scrn::getThemeColour(scrn::td::highlight));
					}
					if(type == listType::numbered){//Add numbers:
						scrn::write(xOffset + 5, yline, i);
						scrn::write(xOffset + 25, yline, listString);
					} else if(type == listType::fullkeyed){
						scrn::write(xOffset + 5 , yline, (*liElems)[i]->getNumber()/16);
						scrn::write(xOffset + 25, yline, (*liElems)[i]->getNumber()%16);
						scrn::write(xOffset + 50, yline, listString);
					} else {//Regular draw:
						scrn::write(xOffset + 5, yline, listString);
					};
					//Show data:
					char listData[liElem::getMaxDataLength()+1] = {0};
					(*liElems)[i]->getData(listData);
					scrn::write(xOffset + dataXCoord, yline, listData);
					lineNumber++;
				}
			}
			
			int getOffset() const{
				return yOffset;
			}
			
			void doFunc(const int button){
				if(button < length){
					(*((*liElems)[button]))();
					}
			}
			
			bool getAllowScroll(){
				return allowScroll;
			}
			
			int getLength() const{
				return length;
			}	
			
			liElem* getActiveListItem(){
				return liElems ? (*liElems)[position] : nullptr;
			}
			
			void setPosition(const int newPos, const bool redraw = false){
				position = newPos;
				if(redraw){draw();}
			}
			
			int getPosition() const{
				return position;
			}
			
			virtual void listDown()	= 0;
			virtual void listUp()	= 0;
			virtual liElem* listMove(const int dir)	= 0;
			virtual liElem* moveTo(const int position)	= 0;
	};
	
	class listControllerFixed : public listController{
		public:
			listControllerFixed(std::vector<list::liElem*>* c_liElems, const listType c_type, const int c_yOffset, const int c_xOffset, const int c_clearWidth, const bool c_allowScroll, const int c_maxLines, const int c_dataXCoord, const bool c_showActive) : 
				listController(c_liElems, c_type, c_yOffset, c_xOffset, c_clearWidth, c_allowScroll, c_maxLines, c_dataXCoord, c_showActive){}

			void listDown(){
				if (position < length-1){
					position++;
					draw();
					}
				}
				
			void listUp(){
				if (position > 0){
					position--;
					draw();
					} 
				}

			liElem* listMove(const int dir){
				if(dir == 1){
					listUp();
					}
				else if (dir == -1){
					listDown();
					}
				return (*liElems)[position]; 
				}
				
			liElem* moveTo(const int pos){
				if (pos < length){
					position = pos;
					modes::mode& lastMode = modes::getActiveMode();
					(*((*liElems)[position]))();
					if(modes::getActiveMode() == lastMode){
						draw();
					}
				}
				return (*liElems)[position];
				}
		};//End list control
		
	class listControllerCard : public listController{
		private:
			uint16_t cardOffset = 0;
		public:
			listControllerCard(std::vector<list::liElem*>* c_liElems, const listType c_type, const int c_yOffset, const int c_xOffset, const int c_clearWidth, const bool c_allowScroll, const int c_maxLines, const int c_dataXCoord, const bool c_showActive) : 
				listController(c_liElems, c_type, c_yOffset, c_xOffset, c_clearWidth, c_allowScroll, c_maxLines, c_dataXCoord, c_showActive){}
			
			void draw(){
				uint16_t patternsOnCard = 0;
				std::vector<liElem*>* theListVec = card::listFiles(cardOffset, maxLines, interface::pattUtils::currentLoadPatternName, patternsOnCard);
				length = patternsOnCard - cardOffset;
				liElems = theListVec;
				listController::draw();
				deleteList(theListVec);
				liElems = nullptr;
			}
			
			void listDown(){
				if (cardOffset < cardOffset + length-1){
					cardOffset++;
					draw();
					}
				}
			void listUp(){
				if (cardOffset > 0){
					cardOffset--;
					draw();
					}
				}
			liElem* listMove(const int dir){
				if(dir == 1){
					listUp();
					}
				else if (dir == -1){
					listDown();
					}
				return nullptr;
				}
			liElem* moveTo(const int pos){
				//I don't think this is used
				return nullptr;
				}
		};//End list control
	
	
	
	class listControllerPatMem : public listController{
		private:
			uint16_t previousPatternInList = 0;
		public:
			listControllerPatMem(std::vector<list::liElem*>* c_liElems, const listType c_type, const int c_yOffset, const int c_xOffset, const int c_clearWidth, const bool c_allowScroll, const int c_maxLines, const int c_dataXCoord, const bool c_showActive) : 
				listController(c_liElems, c_type, c_yOffset, c_xOffset, c_clearWidth, c_allowScroll, c_maxLines, c_dataXCoord, c_showActive){}
			
			void draw(){
				std::vector<list::liElem*>* theListVec = Sequencing::showPatternsInMemory(&interface::pattSwitch::selectedPattern, &previousPatternInList, maxLines);
				//std::vector<list::liElem*>* theListVec = new std::vector<list::liElem*>;
				length = (*theListVec).size();
				liElems = theListVec;
				listController::draw();
				list::deleteList(theListVec);
				liElems = nullptr;
			}
			
			void listDown(){
				if (interface::pattSwitch::selectedPattern < patMem::getLastPatternNum()){
					interface::pattSwitch::selectedPattern ++;
					draw();
					}
				}
				
			void listUp(){
				if (interface::pattSwitch::selectedPattern > 0){
					interface::pattSwitch::selectedPattern = previousPatternInList;
					draw();
					}
				}
				
			liElem* listMove(const int dir){
				if(dir == 1){
					listUp();
					}
				else if (dir == -1){
					listDown();
					}
					return nullptr;
				}
			liElem* moveTo(const int pos){
				//Does not matter?
				return nullptr;
				}
		};//End list control
	
	extern listControllerFixed	cardView; //Used in interface.h
	extern listControllerPatMem	memPatts; //Used in interface.h
	extern listControllerFixed	pattSwit; //Used in interface.h
	extern listControllerFixed	editSteps; //Used in interface.h
	extern listControllerFixed	copy; 
	extern listControllerFixed	MIDIInput; 
	extern listControllerFixed	MIDIOutput; 
	
	list::listController*	activeList	= nullptr;
	list::liElem*	activeliElem	= nullptr;
	
	
	//Held setters:
	//liElem* heldListItem = nullptr;
	std::function<void(int)> heldListItem = nullptr;
	
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
#endif