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

typedef const char* (*charpvoid) ();
typedef int (*intvoid) ();
typedef bool (*boolvoid) ();

typedef unsigned char pin;
typedef unsigned char MIDIvalue;

extern void nullFunc();
extern void nullIntFunc(int u);

extern void strcatc(char* str, char character);//Assumes there is space in string

namespace utl{
	extern int power(int base, int index);
	
	extern int8_t makeSigned(uint8_t n);
	extern int getPowerOfTwo(const long long number);//Be careful using this with weird numbers
	
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
	

	template<typename T, typename S>void arrayOR(T &editArray, const S otherArray){
		for (unsigned int i = 0; i < editArray.size(); i ++){
			editArray[i] |= otherArray [i];
			}
		}

	extern void giveLongStringFromNums(char* theString, const std::vector<uint16_t> nums);
		
	extern void getNoteFromNoteNumber(char* output, const int pitch);
	extern void getNoteNameFromNumber(char* output, const int pitch);
		
	//Does it even work?:
	extern inline __attribute__((always_inline , optimize("unroll-loops"))) void delayClocks(long int clocks);
	
	extern const char* getYesOrNo(const bool var);
		
	extern void incrementString(char* string, uint8_t limit);
	
}//End Utility namespace
template <unsigned int N>
void forEachIf(std::bitset<N> set, std::function<void(unsigned int)> func){
	for(unsigned int i=0; i<N; i++){
		if(set.test(i)){
			func(i);
		}
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