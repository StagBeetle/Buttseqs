#include "Arduino.h"
#include "forwarddec.h"

namespace debug{//Debug message processing:
	char parameter [128] = {0};
	char value [128] = {0};
	bool shouldSetValue = false;

	void processMessage(){//Used for simulating buttons over serial in the format: 77:1; for button 77 press and 77:0; for 77 release
		lg(parameter);
		lg(value);
		
		int parameterRunningTotal = 0;
		int valueRunningTotal = 0;
		sscanf(parameter, "%d", &parameterRunningTotal); 
		sscanf(value, "%d", &valueRunningTotal); 
		
		lg(parameterRunningTotal);
		lg(valueRunningTotal);
			
		if(parameterRunningTotal >= 0 && parameterRunningTotal < 81){
			//buttons::buttonsDEBUG[parameterRunningTotal] = valueRunningTotal;
		}
		
		memset(parameter,0,sizeof(parameter));
		memset(value,0,sizeof(value));
	}

	void serialButtonCheck(){
		while(Serial.available()){
			char c = Serial.read();
			char miniBuffer [2] = {0};
			switch(c){
				case ';':
					processMessage();
					shouldSetValue = false;
					break;
				case ':':
					//setControl();
					shouldSetValue = true;
					break;
				case '/':
					memset(parameter,0,sizeof(parameter));
					memset(value,0,sizeof(value));
					break;
				default:
					miniBuffer[0] = c;
					if(shouldSetValue){
						strcat(value, miniBuffer);
					} else {
						strcat(parameter, miniBuffer);
					}
					break;
			}
		}
		
		// int i = 0;
		// for(auto b: buttons::buttonsOutput){
			// if(b){lg(i);}
			// i++;
		// }
	}
	const int numberOfLoopAverages = 16;
	int loopAverages[numberOfLoopAverages] = {0};
	int averagePosition = 0;
	long long lastTime = 0;
	
	void addLoopTimeToAverage(){
		const auto time = micros();
		const int difference = time - lastTime;
		lastTime = time;
		
		loopAverages[averagePosition] = difference;
		averagePosition++;
		averagePosition %= numberOfLoopAverages;
		
	}
	int getAverageLoopTime(){
		long long sum = 0;
		for(auto a : loopAverages){
			sum += a;
		}
		sum /= numberOfLoopAverages;
		return sum;
	}
}//End namespace