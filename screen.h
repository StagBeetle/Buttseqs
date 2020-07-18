//Sending stuff to the screen
//A layer above whatever graphics library is used.
#ifndef screen_h
#define screen_h

#include <vector>
#include "mode.h"

namespace scrn{
	/*--------------------------------------------------------Font-----------------------------------------------*/
	class font{
		private:
			static const uint16_t textHeight = 18; //How tall the font is
			static const uint16_t charWidth  = 15; //Max width of char
		public:
			static uint16_t getTextHeight()	{return textHeight;	}
			//static uint16_t getBlankHeight()	{return textHeight;	}
			static uint16_t getCharWidth()	{return charWidth;	}
	};
	/*--------------------------------------------------------Colour-----------------------------------------------*/

	struct colour{
		private:
			uint8_t channels[3] = {0};
			// uint8_t m_r;
			// uint8_t m_g;
			// uint8_t m_b;
		public:
			colour (uint8_t c_r, uint8_t c_g, uint8_t c_b){
				channels[0] = c_r;
				channels[1] = c_g;
				channels[2] = c_b;
			}
			
			colour() = default;
			
			uint16_t gSC() const { //Get screen colour
				//convert RGB 255 255 255 colours into 16 bits
				uint16_t r = (channels[0] >> 3) << 11;
				uint16_t g = (channels[1] >> 2) << 5;
				uint16_t b = (channels[2] >> 3) << 0;
				//print (r,g,b)
				return r + g + b;
			}
			
			void editColour(const int channel, const uint8_t value){
				if(channel < 3){
					channels[channel] = value;
				}
			}
			void copyToArray(uint8_t* out) const{
				out[0] = channels[0];
				out[1] = channels[1];
				out[2] = channels[2];
				//memcpy((void*)out, this, 3);
			}
			
			uint8_t getColourChannelValue(const int editColourChannel) const{
				return channels[editColourChannel];
			}
	};
	
	class td{public: enum themeDescriptor : uint8_t{
		bg	= 0,
		acc1	= 1,
		acc2	= 2,
		fg	= 3,
		text	= 4,
		highlight	= 5,
		max
	};};
	
	const char* const colourDescriptors[] = {
		"Backgrnd"	,
		"Accent 1"	,
		"Accent 2"	,
		"Foregrnd"	,
		"Text"	,
		"Highlight"	,
	} ;
	
	const char* const colourChannelNames[] = {
		"Red"	,
		"Green"	,
		"Blue"	,
	} ;
	
	struct theme{
		private:
			colour colours[6];
		public:
			theme(const std::vector<colour> c_colours){
				int limit = min(c_colours.size(), td::max);
				for(int i=0; i<limit; i++){
					colours[i] = c_colours[i];
				}
			}
			
			template<typename T>colour get(const T index){
				int val = static_cast<int>(index);
				if(val < td::max){
					return colours[val];
				} else {
					return {255,128,255};
				}
			}
			
			void editColour(const uint8_t editColourDescriptor, const uint8_t editColourChannel, const uint8_t value){
				if(editColourDescriptor < td::max){
					colours[editColourDescriptor].editColour(editColourChannel, value);
				}
			}
			
			uint8_t getColourChannelValue(const int editColourDescriptor, const int editColourChannel) const {
				return colours[editColourDescriptor].getColourChannelValue(editColourChannel);
			}
	};
	
	
	extern colour getThemeColour(const int index);
	extern uint8_t getColourChannelValue(const int editColourDescriptor, const int editColourChannel);
	extern void editColour(const uint8_t editColourDescriptor, const uint8_t editColourChannel, const uint8_t value);
	
	colour hslToRgb(float h, float s, float l);
	
	/*--------------------------------------------------------Send Stuff-----------------------------------------------*/


	extern void brightness(uint8_t data);
	extern void setup();
	extern void update();
	extern void fillScreen(const colour c);
	extern void blankScreen();
	extern void writeFillRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c);
	extern void writeDrawRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c);
	extern void write(const int16_t x, const int16_t y, const char *str);
	extern void write(const int16_t x, const int16_t y, int num);
	extern void write(const int16_t x, const int16_t y, double num);
	extern void setTextColor(const colour c);
	extern void writeFastVLine(const int16_t x, const int16_t y, const int16_t h, const colour c);
	extern void writeFastHLine(const int16_t x, const int16_t y, const int16_t w, const colour c);
	extern void writeLine(const int16_t x, const int16_t y, const int16_t x2, const int16_t y2, const colour c);
	extern void writeFillTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c);
	extern void writeEdgeTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c);
	extern void writePixel(const int16_t x, const int16_t y, const colour c);
	extern void drawCircle(const int x, const int y, const int r, const colour c);
	extern void drawFillCircle(const int x, const int y, const int r, const colour c);
	extern void setTextColor(const uint16_t color, const uint16_t backgroundcolor);
	extern void setTextSize(uint8_t size);
	extern void setTextWrap(boolean w);
	extern void write(uint8_t character);
	extern void drawChar(const int16_t x, const int16_t y, char c, const uint16_t color, const uint16_t bg, uint8_t size);
	extern void setScreenLock(const bool status); //So notifications stay on the screen
	
	/*--------------------------------------------------------Screen variables Stuff-----------------------------------------------*/
	class displayVar{
		private:
			static const uint8_t maxLabelLength = 8;
			const uint16_t xcoord; 
			const uint16_t ycoord;
			const uint16_t labelWidth; //In pixels
			const uint16_t blankWidth;
			const modes::modeField activeModes;
			const td::themeDescriptor textColour;
			const td::themeDescriptor bgColour;
			char label[maxLabelLength+2] = {0};
			//const std::function<void(void)> preDrawFunc;
		public:
			displayVar(const char* c_label, const uint16_t c_xcoord, const uint16_t c_ycoord, const uint16_t c_labelMinWidth, const uint16_t c_blankWidth, const modes::modeField c_activeModes, const td::themeDescriptor c_textColour = td::text, const td::themeDescriptor c_bgColour = td::bg) : 
				xcoord(c_xcoord), 
				ycoord(c_ycoord),
				labelWidth(c_labelMinWidth),
				blankWidth(c_blankWidth),
				activeModes(c_activeModes),
				textColour(c_textColour),
				bgColour(c_bgColour){
					int length = min(strlen(c_label), maxLabelLength);
					strncpy(label, c_label, length);
				}
			void update(const char* value){
				if(activeModes.isInField(modes::getActiveMode())){
					writeFillRect(xcoord, ycoord, blankWidth, font::getTextHeight(), getThemeColour(bgColour));
					setTextColor(getThemeColour(textColour));
					if(strlen(label)){
						write(xcoord, ycoord, label);
						}
					// lg(value);
					write(xcoord + labelWidth, ycoord, value);
				}
			}
			void update(long long int value){
				char buffer[10] = {0};
				sprintf(buffer, "%lld", value);
				//itoa(value, buffer, 10);
				update(buffer);
			}
			void showActive(){
				const uint8_t indicatorSize = 5;
				writeFillRect(xcoord-indicatorSize, ycoord, indicatorSize, font::getTextHeight(), getThemeColour(textColour));
			}
			void showInactive(){
				const uint8_t indicatorSize = 5;
				writeFillRect(xcoord-indicatorSize, ycoord, indicatorSize, font::getTextHeight(), getThemeColour(bgColour));
			}
		};
		
	extern int topOffset; //Size of mode bar at top of screen
	static const int width  = 480;
	static const int height = 360;
	
	extern int rowSize;
	extern int columnSize;
	extern int initialHeight;
	
//Maybe do this better:
	//label, x, y, labelWidth, blankWidth, modes)
	//Edit Notes:
	extern displayVar octave	;
	extern displayVar viewBar	;
	extern displayVar track	;
	extern displayVar tempo	;
	extern displayVar priority	;
	extern displayVar advance	;
	extern displayVar substep	;
	//EditSteps: 	;
	extern displayVar velocity	;
	extern displayVar length	;
	extern displayVar accent	;
	extern displayVar legato	;
	//All	;
	//extern displayVar  editPattVar	;
	extern displayVar trackGlobal	;
	//PattUtil	;
	extern displayVar pattName	;
	extern displayVar pattNum	;
	extern displayVar sdStatus	;
	//Other: 	;
	extern displayVar editSteps	;
	//Process: 	;
	//extern displayVar rangeMode	;
	extern displayVar activePattNum	;
	extern displayVar activePattName	;
	extern displayVar rangeStart	;
	extern displayVar rangeEnd	;
	//Copy: 	;
	extern displayVar sourcePattNum	;
	extern displayVar sourcePattName	;
}
#endif