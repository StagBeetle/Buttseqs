#include <functional>
#include "scheduled.h"
#include "forwarddec.h"
#include "Arduino.h"

namespace scheduled{
	const int maxScheduledEvents = lOE::max;
	
	void* scheduledPointer;//DEBUG HACK
	
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
			bool checkTimeAndPerformAction(){ //Check if it's time to do the thing -- return true if action performed
				// if((void*)this == scheduledPointer){
					// lg((long)(millis() - timeEventWasCreated));
					// lg(active);
				// }
				if(active && millis() - timeEventWasCreated > timeUntilAction){
					active = false;
					functionToDo();
					return true;
				}
				else {
					return false;
				}
			}
	};
		
	static scheduledEvent scheduledEvents [maxScheduledEvents]; // array of these events
	
	void newEvent(const lOE::listOfEvents type, std::function<void()> c_functionToDo, const long int c_timeUntilAction){
		scheduledEvents[type].set(c_functionToDo, c_timeUntilAction);
		scheduledPointer = (void*)&(scheduledEvents[lOE::recordIcon]);//DEBUG
	}
	
	void clearEvent(const lOE::listOfEvents type){
		scheduledEvents[type].setInactive();
		}
	
	void checkFinishedEvents(){ //The function to call externally that checks each object //checkScheduledEvents
		for(int i=0; i<maxScheduledEvents; i++){
			scheduledEvents[i].checkTimeAndPerformAction(); //check if it's time to do it and then do it
		}
	}
}