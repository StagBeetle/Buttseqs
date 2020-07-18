
// #include "card.h"

// #include "SdFat.h"
// #include "sdios.h"
// #include "list.h"
// #include "card.h"

// namespace card{//For compilation without any functionality: 

	// // SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
	// // 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
	// #define SD_FAT_TYPE 1

	// // SDCARD_SS_PIN is defined for the built-in SD on some boards.
	// #ifndef SDCARD_SS_PIN
	// const uint8_t SD_CS_PIN = SS;
	// #else  // SDCARD_SS_PIN
	// // Assume built-in SD is used.
	// const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
	// #endif  // SDCARD_SS_PIN

	// // Try to select the best SD card configuration.
	// #if HAS_SDIO_CLASS
	// #define SD_CONFIG SdioConfig(FIFO_SDIO)
	// #elif ENABLE_DEDICATED_SPI
	// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
	// #else  // HAS_SDIO_CLASS
	// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
	// #endif  // HAS_SDIO_CLASS
	// //------------------------------------------------------------------------------

	// #if SD_FAT_TYPE == 0
	// SdFat sd;
	// File file;
	// File root;
	// #elif SD_FAT_TYPE == 1
	// SdFat32 sd;
	// File32 file;
	// File32 root;
	// #elif SD_FAT_TYPE == 2
	// SdExFat sd;
	// ExFile file;
	// ExFile root;
	// #elif SD_FAT_TYPE == 3
	// SdFs sd;
	// FsFile file;
	// FsFile root;
	// #endif  // SD_FAT_TYPE
	
	// void setup(){}
	
	// bool fileExists(char* name){
		// return true;
	// }	
	// const char* getConnectedString(){
		// return "no";
	// }		
	// std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns){
		// return nullptr;
	// }
	// void deletePattern(const char* name){
	// }		
	// int readPatternHeaderFromCard(const char* name, uint8_t* outBuffer){
	// return 1;}
		
	// int16_t readNextStepFromCard(const char* name, uint8_t* outBuffer, uint8_t& stepNumber){
		// return 0;
	// }
		
	
// }



//SD Card functions
#include "notifications.h"

//#include "card_external.h"

#include "SdFat.h"
#include "sdios.h"
#include "list.h"
#include "card.h"

namespace card{

	// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
	// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
	#define SD_FAT_TYPE 1

	// SDCARD_SS_PIN is defined for the built-in SD on some boards.
	#ifndef SDCARD_SS_PIN
	const uint8_t SD_CS_PIN = SS;
	#else  // SDCARD_SS_PIN
	// Assume built-in SD is used.
	const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
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
	File root;
	#elif SD_FAT_TYPE == 1
	SdFat32 sd;
	File32 file;
	File32 root;
	#elif SD_FAT_TYPE == 2
	SdExFat sd;
	ExFile file;
	ExFile root;
	#elif SD_FAT_TYPE == 3
	SdFs sd;
	FsFile file;
	FsFile root;
	#endif  // SD_FAT_TYPE


	// bool fileExists(char* name){return true;}
	// void deletePattern(const char*){;}
	// //void setup(){sd.begin(SD_CONFIG);}
	// const char* getConnectedString(){return "this";}
	// std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns){return {};}
	
	//const int maxDirectoryLength;
	
	const char activeDirectory[maxDirectoryLength] = "/";
	char patternSuffix[] = ".bsq";
	
	bool isAvailable = false;
	bool saveError = false;
	
	void setup(){
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
		
	const char* getConnectedString(){
		return isAvailable ? "Card available" : "Card unavailable";
		}
		
	std::vector<list::liElem*>* listFiles(const uint16_t offset, const uint16_t limit, char* firstName, uint16_t &numberOfPatterns){
		std::vector<list::liElem*>* names = new std::vector<list::liElem*>;
		uint16_t counter = 0;
		if (root.open(activeDirectory)){
			while (file.openNext(&root, O_READ) && counter < offset + limit + 1){
				if (counter >= offset && counter < offset+limit){
					char fileName[maxDirectoryLength+1] = {0};
					file.getName(&(fileName[0]), maxDirectoryLength);
					names->push_back(new list::liElemD(fileName));
					// lg(fileName);
					if(counter == offset){
						strcpy(firstName, fileName);
					}
				}
				file.close();
				counter++;
			}
			file.close();
			numberOfPatterns = counter;
		}
		return names;
	}

	// void saveBegin(const char* name){ //Open file
		// sd.chdir(patternDir);
		// char filename[maxFileLength] = {0};
		// strncat(filename, name, 8);
		// strncat(filename, patternSuffix, 4);
		// file = sd.open(filename, O_RDWR | O_CREAT);
	// }
	// void saveData(const void* data, int length){ //Append data
		// int success = file.write(data, length);
		// saveError = success == -1;
		// }
		
	// void saveData(uint8_t data){ //Append data
		// int success = file.write(data);
		// saveError = success == -1;
		// }
		
	// void saveEnd(){ //close file
		// file.close();
		// if(!saveError){notifications::saved.display();}
		// else{notifications::saveFailed.display();}
		// }
		
	void deletePattern(const char* name){
		if(file.open(name, O_RDWR)){
			file.remove();
			file.close();
		}
	}
		
		
	int readPatternHeaderFromCard(const char* name, uint8_t* outBuffer){
		// sd.chdir(patternDir);
		// if (file.open(name, O_READ)){
			// const int result = file.read(static_cast<void*>(outBuffer), patMem::patternDataSize);
			// if(result < patMem::patternDataSize){
				// notifications::invalidPatternHeaderOnCard.display();
				// return -1;
				// }
			// }
		// else{
			// notifications::fileCouldNotBeOpened.display();
			// return -1;
			// }
		return 1;
		}
		
	int16_t readNextStepFromCard(const char* name, uint8_t* outBuffer, uint8_t& stepNumber){
		// //Returns stepnumber or -1
		// //Must come after readPatternHeaderFromCard
		// int16_t step = file.read();
		// if(step == -1){
			// file.close();
			// notifications::loadSuccess.display();
			// return -1; //No more steps
			// }
		// else{
			// int16_t success = file.read(static_cast<void*>(outBuffer), patMem::stepBlockSize);
			// if (success < patMem::stepBlockSize){
				// notifications::invalidPatternOnCard.display();
				// file.close();
				// return -1;
				// }
			// return step; 
			// }
			return 0;
		}
		
	
}
