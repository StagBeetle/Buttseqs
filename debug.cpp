#include "Arduino.h"
#include "forwarddec.h"
#include "buttons.h"

void pause(){//Pause until Serial characters received then ignore them and keep going
	// char c = '\0';
	// Serial.print("waiting...");
	// while(Serial.available() < 1){
		// // Serial.println(Serial.available() );
		// // delay(500);
	// };
	// while(Serial.available()){
		// c = Serial.read();
		// // delay(50);
	// };
	// Serial.println(c);
	// Serial.print(" ...end");
}

namespace buggerking{//Debug message processing:
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
			buttons::setButton(parameterRunningTotal, valueRunningTotal);
		}
		
		memset(parameter,0,sizeof(parameter));
		memset(value,0,sizeof(value));
	}

	void serialButtonCheck(){
		#ifdef USESERIAL
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
		#endif
		// int i = 0;
		// for(auto b: buttons::buttonsOutput){
			// if(b){lg(i);}
			// i++;
		// }
	}
	
	enum class serialMessage{
		buttonPress = 128,
		buttonRelease = 129,
		tempo = 130,
		none = 255
		};
	 
	
	int tempoVal = 120;
	int bytesReceived = 0;
	void checkSerialFromPython(){
		static uint16_t tempTempoVal;
		static serialMessage serialMessageIn;
		if(Serial.available()){
			byte serialData = Serial.read();
			if(serialData >= 128){
				serialMessageIn = static_cast<serialMessage>(serialData);
				bytesReceived = 0;
				}
			else{
				bytesReceived ++;
				switch (serialMessageIn){
					case serialMessage::buttonPress:
						buttons::setButton(serialData, false);
						break;
					case serialMessage::buttonRelease:
						buttons::setButton(serialData, true);
						break;
					case serialMessage::tempo:
						if (bytesReceived == 1){
							tempTempoVal = serialData << 7;
							//lg(1);
							//lg(tempTempoVal);
						}
						else if(bytesReceived == 2){
							tempTempoVal = tempTempoVal + serialData;
							//lg(2);
							//lg(tempTempoVal);
						}
						else {
							//lg(3);
							double tempo = tempTempoVal + (double)serialData/100.0;
							Sequencing::setTempo(tempo);
							//lg(tempTempoVal);
							//lg(serialData);
							draw::tempo();
							//lg("\n");
						}
						break;
					default:
						break;
					}
				}
			}
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