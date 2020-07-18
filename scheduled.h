//Scheduling events to be called after a delay
#ifndef Scheduled_h
#define Scheduled_h
#include <functional>
namespace scheduled{
	//An event can overwrite one of the same type. Has an array of one of each type. Probably a bit wasteful 
	struct lOE { enum listOfEvents{ 
		setVarAfterDelay = 0	,
		drawPianoRoll	,
		drawTempo	,
		patternSwitch	,
		LEDoff	,
		misc	,
		unmute	,
		arrangeChaser	,
		//arrangeChaser2	,
		redraw	,
		recordIcon	,
		mode	,
		max	,
	};};
	
	extern void newEvent(const lOE::listOfEvents type, std::function<void()> c_functionToDo, const long int c_timeUntilAction);
	extern void clearEvent(const lOE::listOfEvents type);
	extern void checkFinishedEvents();
}
#endif