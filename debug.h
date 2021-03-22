#ifndef debug_h
#define debug_h

void pause();

namespace buggerking{
extern void processMessage();
extern void serialButtonCheck();
extern void checkSerialFromPython();
extern void addLoopTimeToAverage();
extern int getAverageLoopTime();
}

#endif

