#ifndef encoder_h
#define encoder_h

namespace encoders{

extern void setup(const std::function<void(int, int)> newAllocator) ;
extern void check();

}//end namespace
#endif