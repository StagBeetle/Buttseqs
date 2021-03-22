//SD Card functions
#include "notifications.h"

//#include "card_external.h"

#include "SdFat.h"
#include "sdios.h"
#include "list.h"
#include "card.h"

namespace card{

	//SD Hardware
		// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
		// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
		#define SD_FAT_TYPE 3

		// SDCARD_SS_PIN is defined for the built-in SD on some boards.
		#ifndef SDCARD_SS_PIN
		const uint8_t SD_CS_PIN = SS;
		#else  // SDCARD_SS_PIN
		// Assume built-in SD is used.
		const uint8_t SD_CS_PIN = BUILTIN_SDCARD;
		#endif  // SDCARD_SS_PIN

		// Try to select the best SD card configuration.
		#if HAS_SDIO_CLASS
		#define SD_CONFIG SdioConfig(FIFO_SDIO)
		#elif ENABLE_DEDICATED_SPI
		#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
		#else  // HAS_SDIO_CLASS
		#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
		#endif  // HAS_SDIO_CLASS
		
		//------------------------------------------------------------------------------

		#if SD_FAT_TYPE == 0
			SdFat sd;
			File file;
			File directory;
		#elif SD_FAT_TYPE == 1
			SdFat32 sd;
			File32 file;
			File32 directory;
		#elif SD_FAT_TYPE == 2
			SdExFat sd;
			ExFile file;
			ExFile directory;
		#elif SD_FAT_TYPE == 3
			SdFs sd;	//The SD card
			FsFile file;	//The currently highlighted file
			FsFile directory;	//The current directory
		#endif  // SD_FAT_TYPES
	//
	const int maxNameLength = 128;
	const int maxLineLength = 256; //How many bytes each line(entry within a file) can be
	char delimiter[2]	 = "\n";
	const char patternSuffix[] = ".bsq";
	
	char activeEntry[maxNameLength] = {0};
	char activeDirectory[maxDirectoryLength] = "/";

	enum class entryType : uint8_t {
		none	,
		file	,
		folder	,
		pattern	, 
		arrangement	,
		settings	,
		trackSettings	,
		patternChain	,
	};
	
	entryType activeType = entryType::none;
	
	const char* getActiveEntryName(){
		file.getName(activeEntry, maxNameLength);
		activeType = file.isDir() ? entryType::folder : entryType::file;
		return activeEntry;
	}
	
	const char* getActiveDirectoryName(){
		directory.getName(activeDirectory, maxDirectoryLength);
		return activeDirectory;
	}
	
	void updateActive(){
		getActiveDirectoryName();
		getActiveEntryName();
	}
	
	bool isAvailable = false;
	bool saveError = false;
	
	void begin(){
		static bool firstRun = true;
		if (!sd.begin(SD_CONFIG)) {
			//sd.initErrorHalt();
			isAvailable = false;
			if(firstRun){
				scheduled::newEvent(scheduled::lOE::misc,[]{
					notifications::sdCardFailed.display();
				}, 2000);
			} else {
				notifications::sdCardFailed.display();
			}
		}
		else{
			isAvailable = true;
			if(!firstRun){
				notifications::sdCardConnected.display();
			}
		}
		firstRun = false;
	}
	
	bool fileExists(char* name){
		return sd.exists(name);
	}
	
	bool fileExists(){
		return sd.exists(activeEntry);
	}
		
	const char* getConnectedString(){
		return isAvailable ? "Card available" : "Card unavailable";
	}
		
	std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns){
		std::vector<list::liElem*>* names = new std::vector<list::liElem*>;
		uint16_t counter = 0;
		if (directory.open(activeDirectory)){
			while (file.openNext(&directory, O_READ) && counter < offset + limit + 1){
				if (counter >= offset && counter < offset+limit){
					char fileName[maxDirectoryLength+1] = {0};
					file.getName(&(fileName[0]), maxDirectoryLength);
					names->push_back(new list::liElemD(fileName));
					// lg(fileName);
					if(counter == offset){
						strcpy(firstName, fileName);
						getActiveEntryName();
					}
				}
				file.close();
				counter++;
			}
			//file.close(); This is a duplicate close. Looks unnecessary
			file.open(activeEntry, O_RDONLY);
			numberOfPatterns = counter;
		}
		return names;
	}
		
	// void deletePattern(const char* name){
		// if(file.open(name, O_RDWR)){
			// file.remove();
			// file.close();
		// }
	// }
	
	const char* getActiveName(){
		return activeEntry;
	}
	
	void goToParentFolder(){
		sd.chdir("../");
		//sd.vwd().getName(activeDirectory);
		getActiveDirectoryName();
	}
	
		// none	,
		// file	,
		// folder	,
		// pattern	, 
		// arrangement	,
		// settings	,
		// trackSettings	,
		// patternChain	,
		
		
	void lookInside(){ //(open / preview / enter folder)
		switch(activeType){
			case entryType::file:
				//file.open(activeEntry, O_RDONLY);
				loadAllFromActive();
				break;
			case entryType::folder:
				sd.chdir(activeEntry);
				list::cardFiles.draw();
				//sd.cwd().getName(activeDirectory);
				getActiveDirectoryName();
				break;
			default:
				break;
		}
	}
	
	void newFolder(const char* name){
		sd.mkdir(name);
	}
	
	void deleteActive(){
		if(file.open(activeEntry, O_RDWR)){
			file.remove();
			file.close();
		}
	}
	void renameActive(const char* newName){
		if(file.open(activeEntry, O_RDWR)){
			file.rename(newName);
			file.close();
		}
	}
	void saveAll(const char* name){
		//patternStorage
		// savePattern(const patMem::pattern_t pattern);
		// saveSettings();
		// saveTrackSettings(const Sequencing::track track);
		// saveArrangement(const uint8_t index);
		// saveProcessChain(const int trackNumber);
	}
	void loadAllFromActive(){
		file.rewind();
		char buffer[maxLineLength] = {0};
		const int length = file.fgets(buffer, maxLineLength, delimiter);
		switch(buffer[0]){
			default:
				break;
			case 'p':
				break;
			case 's':
				lg(length);
				break;
		}
	}
	void cut(){
		
	}
	void copy(){}
	void paste(){}
		
}
