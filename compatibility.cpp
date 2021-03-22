#include "compatibility.h"
#include "Arduino.h"

extern "C"{
	int _getpid(){ return -1;}
	int _kill(int pid, int sig){ return -1; }
	int _write(){return -1;}

	int __exidx_start(){ return -1;}
	int __exidx_end(){ return -1; }
}

namespace std{
	void __throw_bad_function_call(){ for(;;){} }
	void __throw_out_of_range_fmt(char const*, ...){ for(;;){} }
	void __throw_bad_alloc(){ for(;;){} }
	void __throw_length_error(char const*){ for(;;){} }
}


void assertionFailed(const char* file, const int line){
	#ifdef USESERIAL
	Serial.print("Assertion failed on line ");
	Serial.print(line);
	Serial.print(" in ");
	Serial.print(file);
	Serial.println(" .");
	#endif
}