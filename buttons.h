//Reading the input from the buttons in the matrices and attached shift registers and then sending them
#ifndef buttons_h
#define buttons_h
#include <vector>
#include <functional>
#include "Utility.h"
#include "forwarddec.h"
#include "Arduino.h"
#include "mode.h"

namespace buttons{
	extern void begin(const std::function<void(void)> callback);
	extern void loop(const bool recordModeNotesOnly = false);//recordModeNotesOnly is a special thing for record mode to stop jitter (I hope)
	extern void setButton(const int num, const int status);
}//end namespace


#endif