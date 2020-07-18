//#include <chrono>
#include <cfloat>

/*
NOTES:
	get time passed since last:
		calculate the approximate start time based on 
	
	uint64_t timeBase = timestamp - SEBeatsToHostTicks((double)THIS->_tickCount / (double)SEMIDITicksPerBeat, THIS->_tempo);
	timeBase is number of ticks since start?
	Use same function as check sample
	
	Line 384:
		if ( THIS->_clockRunning && THIS->_tempo ) {
			// Calculate new timebase
			uint64_t timeBase = timestamp - SEBeatsToHostTicks((double)THIS->_tickCount / (double)SEMIDITicksPerBeat, THIS->_tempo);
			
			// Add to collected samples
			SESampleBufferIntegrateSample(&THIS->_timeBaseSampleBuffer, timeBase);
			
			// Calculate true time base from samples
			THIS->_timeBase = SESampleBufferCalculatedValue(&THIS->_timeBaseSampleBuffer);
		}

		*/

namespace clockReceive{
	//Tempo:
	
	double calculateTempoFromInterval(const double microsPerTick, const int pipsPerStep = 6, const int numOfSteps = 16){
		return 
		60000000.0 / 
		microsPerTick / 
		((static_cast<long double>(pipsPerStep) * static_cast<long double>(numOfSteps)) / 4.0L);
		//micros = 60 000 000/(BPM * PPQN)
	}
	
	long long calculateTickIntervalFromTempo(const double BPM){
		double ticksPerMinute = BPM * 6;
		double ticksPerMicrosecond = ticksPerMinute / (60.0 * 1000.0 * 1000.0);
		double microsecondsPerTick = 1.0 / ticksPerMicrosecond;
		return microsecondsPerTick;
	}
	
	long long timeAtLastClock = 0;
	
	long long ticksAddedToBuffer = 0;
	
	double averageTempo = 0;
	
	struct sampleBuffer{
		static const int bufferSize = 384 / 4;
		static constexpr float outlierThresholdRatio = 0.75;
		static const int numberOfOutliersBeforeChange = 3;
		static const int samplesBeforeOutlier = 12;
		static const int valuesForMean = 24;
		
		int bufferPosition = 0;
		long long accumulator = 0;
		int numberOfUsefulSamples = 0;
		uint64_t times[bufferSize]; //Circlebuffer?
		uint64_t outliers[numberOfOutliersBeforeChange] = {0};
		int numberOfOutliers = 0;
		double average = 0.0;
		double meanDeviation = 0.0;
		
		void checkSample(const uint64_t interval){
			// addSampleToBuffer(interval);
			// return;
			//All samples will eventually go in the main buffer
			//Outliers are initially saved in their own buffer
			//If they are determined to be normal variations, they are added en masse to the main buffer
			//Otherwise, the main buffer is reset
			bool outlier = false;	
			if(numberOfUsefulSamples < samplesBeforeOutlier){//If it's too early to look for outliers
				outlier = false;
			} else {
				//double outlierThreshold = average / 16; //Testing
				double outlierThreshold = outlierThresholdRatio * meanDeviation;
				outlier = interval > average + outlierThreshold || interval < average - outlierThreshold; //If the interval is outside the standard deviation
			}
			outlier = false; //FOR TESTING
			if(outlier){
				bool isOutlierConsistent = checkOutlierConsistent(interval);
				if(isOutlierConsistent){
					if(numberOfOutliers < numberOfOutliersBeforeChange){//Keep track of outliers...
						outliers[numberOfOutliers] = interval;
						numberOfOutliers ++;
						// lgc("O");
						// lgc("\t");
						return;
					} 
					else {//...Until the outlier buffer is full, then clear the main buffer and replace it
						resetBuffer();
						addOutliersToMainBuffer();
						addSampleToBuffer(interval);
						lg("OUTLIERCHANGE");
						// lgc("\t");
						return;
					}
				} 
				else { //Inconsistent outlier - Add existing outliers to buffer and maybe this new one will be an outlier in the other direction?
					addOutliersToMainBuffer();
					outliers[0] = interval;
					numberOfOutliers = 1;
					// lgc("X");
					// lgc("\t");
					return;
				}
			} 
			else {//Not an outlier
				// lgc(" ");
				// lgc("\t");
				addOutliersToMainBuffer();
				addSampleToBuffer(interval);
				return;
			}
		}
		
		// void calculateAverage(){
			// double averageTime = 0;
			// for(int i=0; i< numberOfUsefulSamples; i++){
				// averageTime += times[((bufferPosition - i) + bufferSize) % bufferSize];
			// }
			// averageTime /= numberOfUsefulSamples;
			// average = averageTime;
			// averageTempo = average;
		// }
		
		void calculateAverage(){
			average = accumulator / numberOfUsefulSamples;
			averageTempo = average;
		}
		
		void calculateMeanDeviation(){
			uint64_t sum = 0;
			int valuesToUse = min(valuesForMean, numberOfUsefulSamples);
			for (int i=0; i < valuesToUse ; i++ ) {//numberOfUsefulSamples
				int pos = (bufferPosition - i + bufferSize) % bufferSize ;
				//uint64_t absDifference = times[pos] > average ? times[pos] - average : average - times[pos];
				uint64_t absDifference = abs(times[pos] - average);
				sum += absDifference;
			}
			meanDeviation = (double)sum / (double)numberOfUsefulSamples;
			//meanDeviation = //TEMP FOR TEST
		}
		
		void addSampleToBuffer(const uint64_t interval){
			if(numberOfUsefulSamples == bufferSize){
				accumulator -= times[bufferPosition];
			}
			times[bufferPosition] = interval;
			numberOfUsefulSamples = numberOfUsefulSamples + 1;
			numberOfUsefulSamples = min(numberOfUsefulSamples, bufferSize);
			
			accumulator += interval;
			
			bufferPosition = (bufferPosition + 1) % bufferSize;
			calculateAverage();
			ticksAddedToBuffer++;
			calculateMeanDeviation();
		}
		
		void addOutliersToMainBuffer(){
			for ( int i=0; i<numberOfOutliers; i++ ) {
				addSampleToBuffer(outliers[i]);
			}
			// if(numberOfOutliers > 3){
				// lgc("numOut ");
				// lg(numberOfOutliers);
			// }
			numberOfOutliers = 0;
		}

		bool checkOutlierConsistent(const uint64_t interval){
			bool isOutlierGreaterThan = interval > average;
			for ( int i=0; i<numberOfOutliers; i++ ) {//Check all outliers are in the same direction
				// lgc((long)outliers[i]);
				// lgc("\t");
				if(isOutlierGreaterThan == (outliers[i] < average)){ //If wrong side
					// lgc("not same side | fail at: ");
					return false;
				}
			}
			return true;
		}
		
		void resetBuffer(){
			bufferPosition = 0;
			numberOfUsefulSamples = 0;
			average = 0;
			accumulator = 0;
		}
		
		void printBuffer(){
			lg("Buffer");
			for(int i=0; i< numberOfUsefulSamples; i++){
				int position = ((bufferPosition-1 - i) + bufferSize) % bufferSize;
				lgc(position);
				lgc("\t");
				lg((long)times[position]);
			}
		}
	
	};
		
	sampleBuffer tempoBuffer;
	sampleBuffer positionBuffer;
	
	
	double tempo = 0.0;
	
	long long ticksReceived = 0;

	const int historyLength = 6;
	const int samplesBeforeRecordingTempoHistory = 13;
	struct{double minimum; double maximum;} tempoHistory[historyLength] = {0};
	uint64_t tempoHistoryBucketSize = 500000; //
	int historyPos = 0;
	int lastHistoryPos = 0;
	
	static const double roundingCoefficients[] = { 0.0001, 0.001, 0.01, 0.1, 0.5, 1.0 }; // Precisions to round to, depending on signal stability
	
	bool hasReceivedFirstClock = false;

	// double getNumberOfTicksReceived(){
		// long long difference = micros() - timeAtLastClock;
		// double ticks = min(ticksReceived + (difference / average), ticksReceived + 1);
		// return ticks;
	// // Basically the idea is to count ticks, and calculate back from the time you received the last tick to when the clock started. So, say, you've counted 25 ticks, the most recent at time t_tick. You've seen 24 tick intervals since the clock started - so the start time t_start is t_tick minus 24 tick intervals ago. You take that timestamp and run it through a lowpass filter (averaging buffer) to take out the noise, and then you use that timestamp in your app to calculate position (i.e. at global time t_1, the timeline position is t_1 minus t_start).﻿
	// }
	
	void clearHistory(){
		for(auto &vals : tempoHistory){
			vals.minimum = DBL_MAX;
			vals.maximum = 0.0;
		}
	}
	
	void resetBuffer(){
		//numberOfOutliers = 0;
		tempoBuffer.resetBuffer();
		positionBuffer.resetBuffer();
		clearHistory();
	}

	void resetClock(){
		resetBuffer();
		hasReceivedFirstClock = false;
		ticksReceived = 0;
		ticksAddedToBuffer = 0;
	}
	

	long long clockDuration = 0;
	void onClock(){
		long long clockStartTime = micros();
		ticksReceived++;
		
		const long long timeNow = micros();
		const long long interval = timeNow - timeAtLastClock;
		timeAtLastClock = timeNow;
		
		if(!hasReceivedFirstClock){hasReceivedFirstClock = true; return;}
		//utl::sendSerialInteger('r', interval);
		tempoBuffer.checkSample(interval);
		//addSampleToBuffer(interval);
		
		//double basicTempo = calculateTempoFromInterval(interval, 6);
		double averageTempo = calculateTempoFromInterval(tempoBuffer.average, 6);
		double roundedTempo = averageTempo;
			

		if(tempoBuffer.numberOfUsefulSamples > samplesBeforeRecordingTempoHistory){
			historyPos = (micros() /tempoHistoryBucketSize) % historyLength;
			if ( historyPos != lastHistoryPos ) {
				// Clear this old bucket
				tempoHistory[historyPos].maximum = 0.0;
				tempoHistory[historyPos].minimum = DBL_MAX;
				lastHistoryPos = historyPos;
			}
			tempoHistory[historyPos].maximum = max(averageTempo, tempoHistory[historyPos].maximum);
			tempoHistory[historyPos].minimum = min(averageTempo, tempoHistory[historyPos].minimum);
			
			unsigned int roundingCoefficient = 0;
			for ( ; roundingCoefficient < (sizeof(roundingCoefficients)/sizeof(double))-1; roundingCoefficient++ ) {
				// For each rounding coefficient (starting small), compare the rounded tempo entries with each other.
				// If, for a given rounding coefficient, the rounded tempo entries all match, then we'll round using this coefficient.
				bool acceptableRounding = true;
				double comparisonValue = 0.0;
				for ( int i=0; i<historyLength; i++ ) {
					if ( tempoHistory[i].maximum == 0.0 ) continue;
					
					if ( comparisonValue == 0.0 ) {
						// Use the first value we come to for comparison
						comparisonValue = round(tempoHistory[i].maximum / roundingCoefficients[roundingCoefficient]) * roundingCoefficients[roundingCoefficient];
					}
					
					// Compare the value bounds for this entry against our comparison value
					double roundedMaxValue = round(tempoHistory[i].maximum / roundingCoefficients[roundingCoefficient]) * roundingCoefficients[roundingCoefficient];
					double roundedMinValue = round(tempoHistory[i].minimum / roundingCoefficients[roundingCoefficient]) * roundingCoefficients[roundingCoefficient];
					
					if ( fabs(roundedMaxValue - comparisonValue) > 1.0e-5 || fabs(roundedMinValue - comparisonValue) > 1.0e-5 ) {
						// This rounding coefficient doesn't give us a stable result - move on
						acceptableRounding = false;
						break;
					}
				}
				
				if ( acceptableRounding ) {
					break;
				}
			}
		
			// Apply rounding
			roundedTempo = round(averageTempo / roundingCoefficients[roundingCoefficient]) * roundingCoefficients[roundingCoefficient];
		}
		
		
		static long long lastTempoChange = 0;
		if(fabs(roundedTempo - tempo) > 0.001){
			tempo = roundedTempo;		
			Sequencing::setTempo(tempo);
			if(millis() - lastTempoChange > 200){
				scheduled::newEvent(scheduled::lOE::drawTempo, draw::tempo, 0);
				lastTempoChange = millis();
			}
		}
		long long clockEndTime = micros();
		long long duration = clockEndTime - clockStartTime;
		clockDuration = max(duration, clockDuration);
	}
}