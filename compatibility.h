#pragma once
//#include "name.c"

extern "C"{
	extern int _getpid();
	extern int _kill(int pid, int sig);
	extern int _write();
	extern int __exidx_start();
	extern int __exidx_end();
}

namespace std{
	extern void __throw_bad_function_call();
	extern void __throw_out_of_range_fmt(char const*, ...);
	extern void __throw_bad_alloc();
	extern void __throw_length_error(char const*);
}

extern void assertionFailed(const char* file, const int line);//Justs prints to serial

//#define ASSERT(assertion) if (!(assertion)) assertionFailed(__FILE__, __LINE__)
	//If this even works, it does not do it in a safe way:
//#define ASSERT(assertion) if (!(assertion)) assertionFailed(__FILE__, __LINE__); return;
#define ASSERT(assertion) if (!(assertion)) assertionFailed(__FILE__, __LINE__)