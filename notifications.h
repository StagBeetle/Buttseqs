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
			static modes::mode* modeToReturnTo;
			notification(const char* c_errorString);
			notification() = default; //just for map
			void display(int lines = 1);
			// void display(){
				// display("");
			// }
			void display(std::vector<const char*> extraLines);
			// static void setLastMode(mode::editMode p_lastMode){
				// modeToReturnTo = p_lastMode;
				// }
			static void returnToMode();
	};
	
			notification::notification(const char* c_errorString) : 
				errorString(c_errorString){
			}
			void notification::display(int lines){
				modeToReturnTo = &modes::getLastMode();
				modes::switchToMode(modes::error, true);
				
				scrn::writeFillRect(x, y, w, h, scrn::getThemeColour(scrn::td::bg));
				const int thickness = 4;
				scrn::writeFillRect(x-thickness, y-thickness, w+thickness, thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x-thickness, y+h, w+thickness*2, thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x-thickness, y-thickness, thickness, h+thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x+w, y-thickness, thickness, h+thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::write(x + margin, scrn::height/2 - (lines * scrn::font::getTextHeight())/2, errorString);
				scrn::setScreenLock(true);
				}
			// void display(){
				// display("");
			// }
			void notification::display(std::vector<const char*> extraLines){
				int options = extraLines.size();
				display(options + 1);
				scrn::setScreenLock(false);
				for(int i = 0; i<options; i++){
					int textHeight = scrn::font::getTextHeight();
					int yline = scrn::height/2 - ((options+1) * textHeight)/2 + (i+1) * textHeight;
					scrn::write(x + margin, yline, i);
					scrn::write(x + margin + 20, yline, extraLines[i]);
				}
				scrn::setScreenLock(true);
			}
			// static void setLastMode(mode::editMode p_lastMode){
				// modeToReturnTo = p_lastMode;
				// }
			void notification::returnToMode(){
				scrn::setScreenLock(false);
				modes::switchToMode(*modeToReturnTo, true);
			}
			
	modes::mode* notification::modeToReturnTo = &modes::editNotes;
	
	notification memoryFull	("No more blocks available"	);
	notification arrangeMemoryFull	("No more space in arrangement"	);
	notification patternMemoryFull	("No more patterns available"	);
	notification stepFull	("Step full"	); //Trying to add more than 8 notes
	notification nameFull	("Name full"	); //Trying to add more than 8 chars to a pattern name
	notification noFileName	("No filename"	); //Trying to save pattern with no name
	notification invalidPatternHeaderOnCard	("Invalid pattern header on card"	);
	notification invalidPatternOnCard	("Invalid pattern on card"	);
	notification fileCouldNotBeOpened	("File could not be opened"	);
	notification trackBankFull	("No available slots in track bank"	);
	notification patternDoesNotExist	("No pattern"	);
	notification noPatternOnTrack	("No pattern on track"	);
	notification notImplemented	("Lazybones"	);
	notification noteOffMemoryFull	("Noteoff memory full"	);
	notification noEventSlots	("No event slots available"	);
	notification dataOutOfRange	("Data out of range"	);
	notification sdCardFailed	("SD card failed"	);
	notification saveFailed	("Save failed"	);
	notification impossibleError	("Impossible error: "	);//notifications::impossibleError.display	(__func__	);
	notification recordNoteOffWithoutOn	("Noteoff without Noteon"	);
	notification noSourcePattern	("No source pattern"	);
	notification noDestinationPattern	("No destination pattern"	);
	notification noSlave	("Slave not connected"	);
	notification noStepSelection	("No steps selected"	);
	notification patternTooLong	("Length longer than possible"	);
	notification barTooShort	("Bar shorter than possible"	);
	notification noProcess	("No Process in position"	);
	notification noProcessSpace	("No more process slots"	);
	//tification name	*	);
	notification saved	("File saved"	);
	notification sdCardConnected	("SD card connected"	);
	notification loadSuccess	("Load success"	);
	notification timeout	("Communication Timeout"	);

	// std::unordered_map<uint8_t, notification> ESPErrors = {
		// {1, saveFailed},
		// {2, sdCardFailed},
	// };

	void displayError(const uint8_t type){
		//(ESPErrors[type]).display();
	}

	void timeoutError(){
		timeout.display();
	}

}
#endif