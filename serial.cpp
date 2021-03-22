#include "Arduino.h"
#include "compatibility.h"
#include "serial.h"

bool hasPrinted = false;
bool usePythonForSerial = false;

// template <typename T>
void printDebug(const char* file , int line, const char* func, const char* message, bool printFileStuff, bool newLine) {
	// startSerialTextMessage();
	const int bufferSize = 128;
	char buffer[bufferSize] = {0};
	
	//Add space at the start of the buffer for 'q' and the length
	// int startOffset = usePythonForSerial ? 2 : 0;
	
	if(usePythonForSerial){
		buffer[0] = 'x'; //Placeholder until we know length
		buffer[1] = 'q'; 
	}
	
	if(printFileStuff){
		//Print:
		const float time = (float)millis() / 1000.0;
		char timeString[10] = {0};
		sprintf(timeString, "%03f", time);
		
		char lineString[5] = {0};
		sprintf(lineString, "%d", line);
		
		char* fileName = strrchr(file, '\\') + 1;//This is to fix a bug I think
		// Serial.print("In ");
		// Serial.print(fileName ? fileName : file);//This is to fix a bug I think
		// Serial.print(" at ");
		// Serial.print(line);
		// Serial.print(" in ");
		// Serial.print(func);
		// Serial.print(" at ");
		// Serial.print(time);
		// Serial.print(": ");
		
		strcat(buffer, "In ");
		strcat(buffer, fileName ? fileName : file);
		strcat(buffer, " at ");
		strcat(buffer, lineString);
		strcat(buffer, " in ");
		strcat(buffer, func);
		strcat(buffer, " at ");
		strcat(buffer, timeString);
	}
	
	strcat(buffer, message);
	
	if(newLine){
		strcat(buffer, "\n");
	}
	
	strcat(buffer, "\x04");
	
	if(usePythonForSerial){
		int length = strlen(buffer);
		ASSERT(length < bufferSize);
	
		buffer[0] = length;
	}
	#ifdef USESERIAL
	Serial.print(buffer);
	#endif
	hasPrinted = true;
	
	// endSerialMessage();
	
}
void printDebug(const char* file , int line, const char* func, uint32_t message , bool printFileStuff, bool newLine) {
	char messageString[10] = {0};
	sprintf(messageString, "%lu", message);
	printDebug(file, line, func, message, printFileStuff, newLine);
}

void printDebug(const char* file , int line, const char* func, int message , bool printFileStuff, bool newLine) {
	char messageString[10] = {0};
	sprintf(messageString, "%d", message);
	printDebug(file, line, func, message, printFileStuff, newLine);
}

void printDebug(const char* file , int line, const char* func, double message , bool printFileStuff, bool newLine) {
	char messageString[10] = {0};
	sprintf(messageString, "%03f", message);
	printDebug(file, line, func, message, printFileStuff, newLine);
}
	
// template <typename T>
void lgw(const char* message){//With newline
	// startSerialTextMessage();
	printDebug("",0,"", message, false, true);
	//Serial.println(message);
	// endSerialMessage();
	}

	void lgw(uint32_t message){//With newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, true);
		//Serial.println(message);
		// endSerialMessage();
		}
		
	void lgw(int message){//With newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, true);
		//Serial.println(message);
		// endSerialMessage();
		}
		
	void lgw(double message){//With newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, true);
		//Serial.println(message);
		// endSerialMessage();
		}
		
// template <typename T>
void lgc(const char* message){//Without newline
	// startSerialTextMessage();
	printDebug("",0,"", message, false, false);
	// endSerialMessage();
	}

	
	void lgc(uint32_t message){//Without newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, false);
		// endSerialMessage();
		}
		
	void lgc(int message){//Without newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, false);
		// endSerialMessage();
		}
		
	void lgc(double message){//Without newline
		// startSerialTextMessage();
		printDebug("",0,"", message, false, false);
		// endSerialMessage();
		}


//Logging (to Serial) with or without newLine:
#define lg(message) (printDebug(__FILE__, __LINE__, __func__, (message)))
// #define lgw(message) (Serial.println((message)))
// #define lgc(message) (Serial.print((message)))
// #define lgc(message) (printDebug(__FILE__, __LINE__, __func__, (message), false))