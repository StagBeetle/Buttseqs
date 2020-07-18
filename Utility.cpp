#include <bitset>	
#include <vector> 
#include "scheduled.h"
#include "Utility.h"
#include "forwarddec.h"
#include <cstring>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "Arduino.h"
//#include "forwarddec.h"

void nullFunc(){;}
void nullIntFunc(int u){;}

void strcatc(char* str, char character){//Assumes there is space in string
	while (*str != '\0'){
		str++;
	}
	*str = character;
}

namespace utl{
	int power(int base, int index){
		int output = 1;
		while(index > 0){
			output *= base;
			index--;
			}
		return output;
	}
	
	int8_t makeSigned(uint8_t n){
		return n-127;
	}
	
	int getPowerOfTwo(const long long number){//Be careful using this with weird numbers
		int powerIndex = 1;
		int powerOfTwo = 2;
		while (powerOfTwo < number){
			powerOfTwo *= 2;
			powerIndex ++;
		}
		return powerIndex;
	}

	void giveLongStringFromNums(char* theString, const std::vector<uint16_t> nums){
		int vectorLength = nums.size();
		for(int i = 0; i<vectorLength; i++){
			char singleString[6];
			sprintf(singleString,"%d",nums[i]);
			strcat(theString, singleString);
			if(i != vectorLength-1){
				strcat(theString, " ");
				}
			}
		}
				
	const char noteNamesSharp[25] = "C C#D D#E F F#G G#A A#B ";
	const char noteNamesFlat [25] = "C DbD EbE F GbG AbA BbB ";
	void getNoteFromNoteNumber(char* output, const int pitch){
		uint8_t octave = pitch / 12;
		char oct;
		if (octave < 10){
			oct = octave + 48;//Convert the number to a char of that number
			}
		else{
			oct = 'X';
			}
		uint8_t note = pitch % 12;
		if(interface::settings::useSharpNotes){
			memcpy(output, &noteNamesSharp[2*note], 2);
		} else {
			memcpy(output, &noteNamesFlat[2*note], 2);
		}
		memcpy(output+2, &oct, 1);
	}
	void getNoteNameFromNumber(char* output, const int pitch){
		if(interface::settings::useSharpNotes){
			memcpy(output, &noteNamesSharp[2*(pitch%12)], 2);
		} else {
			memcpy(output, &noteNamesFlat[2*(pitch%12)], 2);
		}
	}
			
	//Does it even work?:
	inline __attribute__((always_inline , optimize("unroll-loops"))) void delayClocks(long int clocks){//Delays 24 nanoSeconds
		for(int i = 0; i<clocks; i++){
			__asm__ __volatile__ ("nop\n\t");
		}
	}

	const char* getYesOrNo(const bool var){
		return var ? "yes" : "no";
	}
		
	void incrementString(char* string, uint8_t limit){//Get the number at the end of a string and increment it
		uint16_t number = 0;
		uint8_t firstNumPos = 0;
		int8_t firstNumFound = -1;
		for(int i = 0; i<limit + 1; i++){
			uint8_t potentialNumber = string[limit-i-1];
			if(potentialNumber == 0){//Ignore this stuff at end of pattern
				continue;
			} 
			else if(potentialNumber >= 48 && potentialNumber < 58){//If char is a digit
				if(firstNumFound == -1){
					firstNumFound = i;
				}
				number += (potentialNumber-48) * utl::power(10, i-firstNumFound);
				// lg(number);
			} 
			else {
				firstNumPos = min(limit - i, limit - 1);
				break;
			}
		}
		char numberStr[9] = {0};
		sprintf(numberStr, "%d", number);
		int numLength = strlen(numberStr);
		int numPosOffset = min(firstNumPos, limit - numLength);
		memcpy(string + numPosOffset, numberStr, numLength);				
	}
	
}//End Utility namespace
