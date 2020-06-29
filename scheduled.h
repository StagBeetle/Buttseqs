//Scheduling events to be called after a delay
#ifndef Scheduled_h
#define Scheduled_h
#include <functional>
namespace scheduled{
	//An event can overwrite one of the same type
	struct lOE { enum listOfEvents{ 
		setVarAfterDelay = 0	,
		drawPianoRoll	,
		drawTempo	,
		patternSwitch	,
		LEDoff	,
		misc	,
		unmute	,
		arrangeChaser	,
		arrangeChaser2	,
		max	,
	};};
	
	// struct eventID{
		// private:
			// int8_t ID;
		// public:
			// eventID() :
				// ID(-1){}
			// eventID(const int8_t c_ID) :
				// ID(c_ID){}
			// bool isValid(){
				// return ID < 0 ? false : true;
				// }
			// int8_t get() const{
				// return ID;
				// }
			// void invalidate(){
				// ID = -1;
				// }
	// };
	
	const int maxScheduledEvents = lOE::max;
	
	class scheduledEvent{
		private:
			std::function<void()> functionToDo;
			bool active = false;
			unsigned long long timeEventWasCreated; //The time the constructor was called - -remake this without this to save space
			unsigned long long timeUntilAction; //time in millis before the thing will happen 
		public:
			scheduledEvent() = default;//Constructor
			void set(std::function<void()> c_functionToDo, long int c_timeUntilAction){
				functionToDo = c_functionToDo;
				timeUntilAction = c_timeUntilAction;
				timeEventWasCreated = millis(); //Get the time this constructor was called
				active = true;
				}
			bool isActive(){
				return active;
				}
			void setInactive(){
				active = false;
			}	
			std::function<void()> getFunc(){
				return functionToDo;
			}
			void showTime() const{
				// lg(static_cast<long long>(timeEventWasCreated	));
				// lg(static_cast<long long>(timeUntilAction	));
				// lg(static_cast<long long>(timeEventWasCreated + timeUntilAction - millis()	));
			}
			bool checkTimeAndPerformAction(){ //Check if it's time to do the thing -- return true if action performed
				if(active && millis() >= timeEventWasCreated + timeUntilAction){
					functionToDo();
					active = false;
					return true;
				}
				else {return false;}
			}
	};
		
	static scheduledEvent scheduledEvents [maxScheduledEvents]; // array of these events
	
	void newEvent(const lOE::listOfEvents type, std::function<void()> c_functionToDo, const long int c_timeUntilAction){
		//if(type == lOE::arrangeChaser){ lg("newEvent");}
		scheduledEvents[type].set(c_functionToDo, c_timeUntilAction);
	}
	
	// void updateEvent(const eventID id, std::function<void()> c_functionToDo, const long int c_timeUntilAction, const bool important = false;){
		// scheduledEvents[id.get()].set(c_functionToDo, c_timeUntilAction);
	// }
		
	void clearEvent(const lOE::listOfEvents type){
		scheduledEvents[type].setInactive();
		//theEvent.invalidate();
		}
	
	void checkFinishedEvents(){ //The function to call externally that checks each object //checkScheduledEvents
		for(int i=0; i<maxScheduledEvents; i++){
			scheduledEvents[i].checkTimeAndPerformAction(); //check if it's time to do it and then do it
		}
	}
}
#endif