#pragma once
#include "Arduino.h"
#include "compatibility.h"
//This is used for Python to see that a message has ended 

extern bool usePythonForSerial;
extern bool hasPrinted;

// void startSerialTextMessage();

void endSerialMessage();


// template <typename T>
void printDebug(const char* file , int line, const char* func, const char* message = "", bool printFileStuff = true, bool newLine = true);
void printDebug(const char* file , int line, const char* func, uint32_t message , bool printFileStuff = true, bool newLine = true);
void printDebug(const char* file , int line, const char* func, int message , bool printFileStuff = true, bool newLine = true);
void printDebug(const char* file , int line, const char* func, double message , bool printFileStuff = true, bool newLine = true);
	
// template <typename T>
void lgw(const char* message);
void lgw(uint32_t message);
void lgw(int message);
void lgw(double message);
		
void lgc(const char* message);
void lgc(uint32_t message);
void lgc(int message);		
void lgc(double message);


//Logging (to Serial) with or without newLine:
#define lg(message) (printDebug(__FILE__, __LINE__, __func__, (message)))
// #define lgw(message) (Serial.println((message)))
// #define lgc(message) (Serial.print((message)))
// #define lgc(message) (printDebug(__FILE__, __LINE__, __func__, (message), false))