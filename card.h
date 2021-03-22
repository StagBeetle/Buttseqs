/**
 * @file card.h
 *
 * The code concerning the SD card
 * 
 */
 
#ifndef card_stuff_h
#define card_stuff_h
#include "notifications.h"

//#include "card_external.h"

// #include "SdFat.h"
// #include "sdios.h"

namespace card{//For compilation without any functionality: 
	
	/** The log level enumeration type. */
	bool fileExists(char* name);
	bool fileExists();
	const char* getConnectedString();
	std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns);
	void begin();
	
	const char* getActiveName();
	
	//Save patterns with arrangement : true : false
	//Save pattern names in arrangement
		
	// //Default List
		void goToParentFolder();
		void newFolder(const char* name);
		void lookInside(); //(open / preview / enter folder)
		void deleteActive();
		void renameActive(const char* newName);
		void saveAll(const char* name);
		void loadAllFromActive();
		void cut();
		void copy();
		void paste();
		// "Save / Load Mode"
	// //In save mode

		// "save location" //: last, ask
			//toggleDefaultSaveLocation
		// "overwrite existing" //: overwrite, ask
			//toggleOverwrite
		// "Save active pattern"
		// "Save all patterns"	
		// "Save Settings"
		// "Save track"
		// "Save all tracks"
		// "Save arrangement"
		// "Save all arrangements"
		// "Save processChain"
	// //In load mode
		// "Parent "
		// "View" (open / prevew / enter folder)
		// "Delete"
		// "Rename"
		// "Load all in file"
		// "Copy"
		// "Paste"
		
		// "Load Pattern To Memory"
		// "Load Pattern To Track"
		// "Load Pattern To Track Bank"
		// "Load settings"
		// "Load trackSettingsTo..."
		// "Load arrangement"
			
	void loadItem();//switch on item type
	void loadPattern();
	void loadSettings();
	void loadTrackSettings(const int track);
	void loadArrangement();
	void loadProcessChain(const int track);
	
	void savePattern(const patMem::pattern_t pattern);
	void saveSettings();
	void saveTrackSettings(const Sequencing::track track);
	void saveArrangement(const uint8_t index);
	void saveProcessChain(const int trackNumber);
		
}

#endif
