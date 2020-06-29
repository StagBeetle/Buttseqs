#ifndef Utility_h
#define Utility_h

#include <bitset>	
#include <vector> 
#include "scheduled.h"
//#include "forwarddec.h"

typedef void (*voidvoid) ();
typedef void (*voidint)  (int);
typedef void (*voiduint8t)  (uint8_t);
typedef void (*voidbool) (bool);
typedef void (*voidu3) (uint8_t, uint8_t, uint8_t);

//Functions:
typedef const char* (*charpvoid) ();
typedef int (*intvoid) ();
typedef bool (*boolvoid) ();

void nullFunc(){;}
void nullIntFunc(int u){;}
typedef unsigned char pin;
typedef unsigned char MIDIvalue;

bool enableTransmission = true;

int max(int a, int b) {return (a > b) ? a : b;}
int min(int a, int b) {return (a < b) ? a : b;}

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
	
	//unsigned int
	template <size_t N>
	int getFirstSetBit(const std::bitset<N> set){
		for(size_t i=0; i<N; i++){
			if(set.test(i)){
				return i;
			}
		}
		return -1;
	}
		
	template<typename T>bool isEven(T number){
		if (number % 2 == 0) {return true;}
		else {return false;}
		}
	
	class timer{
		public:
			void reset(){time = micros();}
			void finish(){
				time = micros() - time;
				Serial.print("Time passed: ");
				Serial.print(time);
				Serial.print('\n');
			}
		private:
			unsigned long time;
		};

	template<typename T, typename S>void arrayOR(T &editArray, const S otherArray){
		for (unsigned int i = 0; i < editArray.size(); i ++){
			editArray[i] |= otherArray [i];
			}
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
		
	// int8_t gPFK(const uint8_t key){//GetPitchFromKey //Return -1 for error
		// int pitch = 12 * interface::keyboardOctave + key;
		// return (pitch < 128) ? pitch : pitch - 12;
	// }
		
	const char noteNamesSharp[25] = "C C#D D#E F F#G G#A A#B ";
	const char noteNamesFlat [25] = "C DbD EbE F GbG AbA BbB ";
	void getNoteFromNoteNumber(char* output, const int pitch){
		uint8_t octave = pitch / 12;
		char oct;
		if (octave < 10){
			oct = octave + 48;
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
	
	// void makeTempoString(char* outString, const double tempo){
		// sprintf(outString, "%d" , tempo/100);
		// strcat(outString, ".");
		// char number[4];
		// sprintf(number, "%02d" , tempo%100);
		// strcat(outString, number);
	// }
	
	//newLine does nothing
	void sendSerialMessage(const char type, const char* message, const bool newLine = true){
		//if (type == 'A'){//Can be replaced by if type == 'z' etc.
			const int length = strlen(message) + 3 + 1;
			char str[length];
			memset (str, 0, length);
			str[0] = type;
			strcat(str, message);
			str[length-3] = newLine ? '\n' : ' ';
			str[length-2] = 4;
			if(enableTransmission){
				Serial.write(str);// end transmission
			}
			//Serial.send_now();
			//Serial.send_now();
		//}
	}
		
	template<typename T>void sendSerialInteger(const char type, const T number, const bool newLine = true){
		char string[12];
		long long int num = static_cast<long long int>(number);
		sprintf(string, "%lld" , num);
		sendSerialMessage(type, string, newLine);
	}
		
	void sendSerialFloat(const char type, const double number, const bool newLine = true){
		char string[12];
		sprintf(string, "%0.4lf" , number);
		sendSerialMessage(type, string, newLine);
	}
		
	void sendSerialArray(const char type, const char* message, const int length){
		int l = length + 2;
		char str[l];
		str[0] = type;
		strncpy(str+1, message, length);
		str[l-1] = 4;
		Serial.write(str, l);// send transmission
	}
		
	//Does it even work?:
	inline __attribute__((always_inline , optimize("unroll-loops"))) void delayClocks(long int clocks){//Delays 24 nanoSeconds
		for(int i = 0; i<clocks; i++){
			__asm__ __volatile__ ("nop\n\t");
		}
	}
	
	// const uint8_t LEDpin = 13;
	// void setup(){
		// pinMode(LEDpin, OUTPUT);
	// }
	
	// void LEDoff(){
		// digitalWriteFast(LEDpin, LOW);
	// }
		
	// void LEDflash(){
		// digitalWriteFast(LEDpin, HIGH);
		// scheduled::newEvent(scheduled::lOE::LEDoff, LEDoff,5);
	// }
	
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
template <unsigned int N>
void forEachIf(std::bitset<N> set, std::function<void(unsigned int)> func){
	for(unsigned int i=0; i<N; i++){
		if(set.test(i)){
			func(i);
		}
	}
}

// void lg(const char* data){
	// utl::sendSerialMessage('q', data);
	// }
// void lg(char* data){
	// utl::sendSerialMessage('q', data);
	// }
// template<typename T>void lg(const T data){
	// utl::sendSerialInteger('q', data);
	// }
// void lg(const double data){
	// utl::sendSerialFloat('q', data);
	// }


// void lg(const char* data, const long long num){
	// utl::sendSerialMessage('q', data, false);
	// utl::sendSerialInteger('q', num);
	// }
	
// void lgc(const char* data){
	// utl::sendSerialMessage('q', data, false);
	// }
// template<typename T>void lgc(const T data){
	// utl::sendSerialInteger('q', data, false);
	// }
// void lgc(const double data){
	// utl::sendSerialFloat('q', data, false);
	// }

//Doesn't work:
// long long lastTime = 0;
// class timeLog{
	// public:
		// timeLog(const int c_timeBetweenLogsInMillis = 100) : timeBetweenLogsInMillis(c_timeBetweenLogsInMillis){}
		// template<typename T>void lg(T message, const int c_timeBetweenLogsInMillis = -1){
			// if(c_timeBetweenLogsInMillis >= 0){
				// timeBetweenLogsInMillis = c_timeBetweenLogsInMillis;
			// }
			// int timeSinceLast = millis() - timeAtLastLog;
			// if(millis() - lastTime > 500){
				// lg(timeSinceLast);
				// lastTime = millis();
			// }
			// if(timeSinceLast >= timeBetweenLogsInMillis){
				// timeAtLastLog = millis();
				// lg(message);
			// }
		// }
	// private:
		// long long timeAtLastLog = 0;
		// int timeBetweenLogsInMillis;
// };

const uint16_t logMax = 256;
uint16_t logLast = 0;
long logArray[logMax] = {0};
void addToLogArray(const uint16_t data){
	if(logLast < logMax){
		logArray[logLast] = data;
		logLast++;
		}
	}
void printLogArray(){
	for (uint16_t i = 0; i<logLast; i++){
		lg(logArray[i]);
		}
	}

template <typename T>
void typecopy(volatile T& dst, const volatile T& src){
	static_assert(std::is_copy_constructible<T>::value, "not copy constructible");
	const size_t size = sizeof(T);
	uint8_t* dst_p = reinterpret_cast<uint8_t*>(const_cast<T*>(&dst));
	uint8_t* src_p = reinterpret_cast<uint8_t*>(const_cast<T*>(&src));
	for(size_t i = 0; i<size; i++){
		*dst_p++ = *src_p++;
	}
}

#endif