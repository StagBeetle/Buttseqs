//Errors that do something
#ifndef Errors_h
#define Errors_h
#include "Utility.h"
#include "screen.h"
//#include <unordered_map>

namespace notifications{
	class notification{
		private:
			const char* errorString;
			static const int x = 40;
			static const int y = 80;
			static const int w = scrn::width - 2 * x;
			static const int h = scrn::height - 2 * y;
			static const int margin = 20;
		public:
			//static modes::mode* modeToReturnTo;
			notification(const char* c_errorString);
			notification() = default; //just for map
			void display(int lines = 1);
			void display(std::vector<const char*> extraLines);
			static void returnToMode();
	};
	
	extern notification memoryFull	;
	extern notification arrangeMemoryFull	;
	extern notification patternMemoryFull	;
	extern notification stepFull	; //Trying to add more than 8 notes
	extern notification nameFull	; //Trying to add more than 8 chars to a pattern name
	extern notification noFileName	; //Trying to save pattern with no name
	extern notification invalidPatternHeaderOnCard	;
	extern notification invalidPatternOnCard	;
	extern notification fileCouldNotBeOpened	;
	extern notification trackBankFull	;
	extern notification patternDoesNotExist	;
	extern notification noPatternOnTrack	;
	extern notification notImplemented	;
	extern notification noteOffMemoryFull	;
	extern notification noEventSlots	;
	extern notification dataOutOfRange	;
	extern notification sdCardFailed	;
	extern notification saveFailed	;
	extern notification impossibleError	;//notifications::impossibleError.display	(__func__	);
	extern notification recordNoteOffWithoutOn	;
	extern notification noSourcePattern	;
	extern notification noDestinationPattern	;
	extern notification noSlave	;
	extern notification noStepSelection	;
	extern notification patternTooLong	;
	extern notification barTooShort	;
	extern notification noProcess	;
	extern notification noProcessSpace	;
	//tification name	;
	extern notification saved	;
	extern notification sdCardConnected	;
	extern notification loadSuccess	;
	extern notification timeout	;

	void displayError(const uint8_t type);

	void timeoutError();

}
#endif