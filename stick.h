//Rename Buttons.h to input.h
//include a function that switches func based on mode
/*
#ifndef stick_h
#define stick_h
namespace stickinput{
class stick{
	private:
		const static int res = 10;
		const static int maxPot = utl::power(2, res);
		const static int maxVal = maxPot / 2;
		const static int deadzone =  maxPot/16;
		const static int centre = maxPot/2;
		const static int slowRate = 1000;
		const static int fastRate = 50;
		long int timeSinceLastFunc = 0;
		const uint16_t pin;
		std::function<void(void)> posFunc;
		std::function<void(void)> negFunc;
	public:
		stick(int c_pin, std::function<void(void)> c_posFunc, std::function<void(void)> c_negFunc) :
		pin(c_pin),
		posFunc(c_posFunc),
		negFunc(c_negFunc){
			analogReadResolution(res);
		}
		checkStick(){
			//Depending on how long this takes, maybe do it a bit less than every loop
			int val = analogRead(pin);
			int abs = val > centre ? val - centre : centre - val;
			bool isNeg = val < centre;
			//how does this handle up then down?
			if (abs > deadzone){
				//Do a better mapping for this:
				int delayTime = Map(abs, 0, maxVal, slowRate, fastRate);
				if (millis() > timeSinceLastFunc + delayTime){
					if (isMinusX){
							c_negFunc();
						} else {
							c_posFunc();
						}
					}
				timeSinceLastFunc = millis();
			}	 	
		}
}

stick xStick(0, input::rFunc.trigger, input::lFunc.trigger);
stick yStick(0, input::dFunc.trigger, input::uFunc.trigger);

stickinput::xStick.checkStick();
stickinput::yStick.checkStick();

// input.h:
// class dirFunc{
	// private:
	// public:
		// trigger(){
//}
}
#endif
*/