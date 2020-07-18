#include <vector>
#include <functional>
#include <stdint.h>
#include "Arduino.h"
#include "forwarddec.h"

IntervalTimer sequence;
void startTimer(){sequence.begin(Sequencing::checkPlaying, 500);}
void stopTimer(){sequence.end();}

void setSequencePriority(const int priority){
	sequence.priority(priority);
}

namespace interface{
	namespace pattSwitch{
		uint16_t selectedPattern = 0;
	}
	namespace settings{
		unsigned int buttonHoldDelay = 250;
	}
}