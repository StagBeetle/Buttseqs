#define ENCODER_OPTIMIZE_INTERRUPTS

#include <Encoder.h>
#include <functional>
#include "encoder.h"

namespace encoders{

std::function<void(int, int)> allocator = nullptr;

class enco{
	public:
		enco(const int pinA, const int pinB) : 
		enc(pinA , pinB){}
		long readChange(){
			long newValue = enc.read();
			long returnValue = newValue - lastValue;
			lastValue = newValue;
			return returnValue;
		}//return the Change
	private:
		long lastValue = 0;
		Encoder enc;
};

const int numberOfEncoders = 4;

enco encoders[numberOfEncoders] = {
	{36, 35},
	{38, 37},
	{30, 39},
	{28, 29},
};

PROGMEM void begin(const std::function<void(int, int)> newAllocator) {
	allocator = newAllocator;
}

void check() {
	if(!allocator){return;}
	for(int i = 0; i < numberOfEncoders; i++){
		const long change = encoders[i].readChange();
		if(change){
			allocator(i, change);
		}
	}
}

}//end namespace