//SD Card functions
#ifndef card_stuff_h
#define card_stuff_h
#include "notifications.h"

//#include "card_external.h"

// #include "SdFat.h"
// #include "sdios.h"

namespace card{//For compilation without any functionality: 

	extern bool fileExists(char* name);
	extern void deletePattern(const char*);
	extern void setup();
	extern const char* getConnectedString();
	extern std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns);
	
	void loadActiveFile();
	void viewActiveFile();
	void save();
	//All patterns, selected patterns, 
}

#endif
