#include "Utility.h"
#include "screen.h"
#include "notifications.h"
#include "forwarddec.h"
//#include <unordered_map>

namespace notifications{

			notification::notification(const char* c_errorString) : 
				errorString(c_errorString){
			}
			void notification::display(int lines){
				scrn::setScreenLock(false);
				// lg("display");
				//modeToReturnTo = &modes::getLastMode();
				modes::setDialog(modes::focusedContext::error);
				
				scrn::writeFillRect(x, y, w, h, scrn::getThemeColour(scrn::td::bg));
				const int thickness = 4;
				scrn::writeFillRect(x-thickness, y-thickness, w+thickness, thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x-thickness, y+h, w+thickness*2, thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x-thickness, y-thickness, thickness, h+thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::writeFillRect(x+w, y-thickness, thickness, h+thickness, scrn::getThemeColour(scrn::td::acc1));
				scrn::write(x + margin, scrn::height/2 - (lines * scrn::font::getTextHeight())/2, errorString);
				scrn::setScreenLock(true);
				// scrn::update();
				// delay(5000);
			}
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
			
	//modes::mode* notification::modeToReturnTo = &modes::editNotes;
	
	notification memoryFull	("No more blocks available"	);
	notification arrangeMemoryFull	("No more space in arrangement"	);
	notification patternMemoryFull	("No more patterns available"	);
	notification stepFull	("Step full"	); //Trying to add more than 8 notes
	notification nameFull	("Name full"	); //Trying to add more than 8 chars to a pattern name
	notification nameEmpty	("Name empty"	); //Trying to add more than 8 chars to a pattern name
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
	notification noDataSpace	("No space for data"	);
	notification dataNotFound	("Data not found"	);
	//tification name	*	);
	notification saved	("File saved"	);
	notification sdCardConnected	("SD card connected"	);
	notification loadSuccess	("Load success"	);
	notification timeout	("Communication Timeout"	);
	notification successfulError	("Successful Error Initiated"	);

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