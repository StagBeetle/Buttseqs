#pragma once

namespace joint{
	//screensharing and button mapping
	//Buttons, LEDs, screen, MIDI, encoders+joystick
	
	enum class dataType : uint8_t {
		none	= 0,
		button	= 1,
		LED	= 2,
		screen	= 3,
		MIDI	= 4,
		encoder	= 5,
		joystick	= 6,
	};
	
	
	bool shareData(const dataType type, const uint8_t* data, const size_t length); //Return false for failure.
	bool receiveData(const uint8_t* data, const size_t length);
	
}