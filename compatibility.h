#ifndef compatibility_h
#define compatibility_h
//#include "name.c"

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
#endif

void assertionFailed(const char* file, const int line){
	Serial.print("Assertion failed on line ");
	Serial.print(line);
	Serial.print(" in ");
	Serial.print(file);
	Serial.println(" .");
}

// #define ASSERT(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, __ASSERT_FUNC, #__e))
// #define ASSERT(assertion) (if(!((assertion))){ __assert_func(__FILE__, __LINE__); })
#define ASSERT(assertion) if (!(assertion)) assertionFailed(__FILE__, __LINE__)

// char* lastCurlyFile = "";
// int lastCurlyLine = 0;

// void assignLastCurlyBrace(const char* file, const int line){
	// lastCurlyFile = file;
	// lastCurlyLine = line;
// }

//#define { {assignLastCurlyBrace(__FILE__, __LINE__);