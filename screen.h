//Sending stuff to the screen
//A layer above whatever graphics library is used.
#ifndef screen_h
#define screen_h
#include "screen_external.h"

namespace scrn{
	const int brightnessPin = A0;

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
			colour (uint8_t c_r = 255, uint8_t c_g = 25, uint8_t c_b = 255){
				channels[0] = c_r;
				channels[1] = c_g;
				channels[2] = c_b;
			}
			
			uint16_t gSC() const { //Get screen colour
				//convert RGB 255 255 255 colours into 16 bits
				uint16_t r = (channels[0] >> 3) << 0;
				uint16_t g = (channels[1] >> 2) << 5;
				uint16_t b = (channels[2] >> 3) << 11;
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
			
			uint8_t getColourChannelValue(const int editColourChannel){
				return channels[editColourChannel];
			}
	};
	
	// typedef uint16_t color_t;

	// static uint16_t color565(const uint8_t r, const uint8_t g, const uint8_t b) {
		// return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	// }
	
	/*
	const colour bright[4] = {
		{200,  55,  55}, //BG
		{238, 162,  17}, //Accent 1
		{ 33, 120, 103}, //Accent 2
		{ 34,  55,  55}  //FG
	};
	*/
	class td{public: enum themeDescriptor : uint8_t{
		bg	= 0,
		acc1	= 1,
		acc2	= 2,
		fg	= 3,
		text	= 4,
		highlight	= 5,
		none	= 6,
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
					return colours[td::none];
				}
			}
			
			void editColour(const uint8_t editColourDescriptor, const uint8_t editColourChannel, const uint8_t value){
				if(editColourDescriptor < td::max){
					colours[editColourDescriptor].editColour(editColourChannel, value);
				}
			}
			
			uint8_t getColourChannelValue(const int editColourDescriptor, const int editColourChannel){
				return colours[editColourDescriptor].getColourChannelValue(editColourChannel);
			}
	};
	
	theme mainTheme = {{
		{ 50,  50,  50},	//BG
		{100, 100, 100},	//Accent 1
		{150, 150, 150},	//Accent 2
		{200, 200, 200},	//FG
		{255, 200,  10},	//text
		{200,  50,  20},	//highlight
	}};
	
	template<typename T>colour getThemeColour(const T index){
		return mainTheme.get(index);
	}
	
	/*--------------------------------------------------------Send Stuff-----------------------------------------------*/
	const static char replacer[] = "\x1d"; //Group separator
	
	bool screenLock = false; //If true, cannot draw to screen
	
	// enum class drawType_t : uint8_t{
		// none	= 0,
		// fill	= 1,
		// rectFill	= 2,
		// rectEdge	= 3,
		// textColor	= 4,
		// write	= 5,
		// vline	= 6,
		// hline	= 7,
		// triEdge	= 8,
		// triFill	= 9
	// };
	
	enum class iconIndex : uint8_t{
		steps	= 0,
		notes	= 1,
		vertical	= 2,
		horizontal	= 3,
		data	= 4,
		press	= 5,
		release	= 6,
		hold	= 7,
	};
	
	const uint8_t eventOffset = static_cast<uint8_t>(iconIndex::press);
	
	// void intToByteArray(const int num, uint8_t* array){
		// *array       = num / 128;
		// *(array + 1) = num % 128;
	// }
	
	// void pythonSend(const comms::MOSIType_t type, const uint8_t* message, const int length){
		// int arrLength = length + 3;
		// char str[arrLength];
		// //memset(str, 'x', arrLength);
		// str[0] = 'A';
		// str[1] = static_cast<char>(type);
		// memcpy(str+2, message, length);
		// str[arrLength - 1] = 4;
		// Serial.write(str, arrLength);
	// }
	
	void brightness(uint8_t data){
		analogWrite(brightnessPin, data);
	}
	
	void setup(){
		brightness(255);
		tft.begin();
		tft.setRotation(3);
		tft.fillScreen(ILI9488_BLACK);
		//tft.setTextColor(ILI9488_YELLOW);
		//tft.setTextSize(2);
		//tft.setFont(Arial_12);
		tft.setOrigin(0,0);
		tft.invertDisplay(1);
		//tft.useFrameBuffer(true); 
	}
	
	// void screenMessage(const uint8_t type, const uint8_t* message, const int length){
		// if(!screenLock){
			// // lg(*message);
			// //pythonSend(type, message, length);
			// //serialObject.write(message, length);
			// comms::send(comms::pM::screen, type, message, length);
		// }
	// }
	

		
	void fillScreen(const colour c){
		tft.fillScreen(c.gSC());
	}
	
	void blankScreen(){
		fillScreen(getThemeColour(scrn::td::bg));
	}

	void writeFillRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c){
		tft.fillRect(x, y, w, h, c.gSC());
	}
	
	void writeDrawRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c){
		tft.drawRect(x, y, w, h, c.gSC());
	}
	
	void write(const int16_t x, const int16_t y, const char *str){
		tft.setCursor(x, y);
		tft.print(str);
	}
	
	void write(const int16_t x, const int16_t y, int num){
		char string[6] = {0};
		sprintf(string, "%i", num);
		write(x, y, string);
	}
	
	void write(const int16_t x, const int16_t y, double num){
		char string[32] = {0};
		sprintf(string, "%f", num);
		write(x, y, string);
	}
	
	void setTextColor(const colour c){
		tft.setTextColor(c.gSC());
	}
	
	void writeFastVLine(const int16_t x, const int16_t y, const int16_t h, const colour c){
		tft.drawFastVLine(x, y, h, c.gSC());
	}
	
	void writeFastHLine(const int16_t x, const int16_t y, const int16_t w, const colour c){
		tft.drawFastHLine(x, y, w, c.gSC());
	}
	void writeLine(const int16_t x, const int16_t y, const int16_t x2, const int16_t y2, const colour c){
		tft.drawLine(x, y, x2, y2, c.gSC());
	}
	void writeFillTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c){
		tft.fillTriangle(x0, y0, x1, y1, x2, y2, c.gSC());
	}
	void writeEdgeTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c){
		tft.drawTriangle(x0, y0, x1, y1, x2, y2, c.gSC());
	}
	void writePixel(const int16_t x, const int16_t y, const colour c){
		tft.drawPixel(x, y, c.gSC());
	}
	void writeIcon(const int16_t x, const int16_t y, const iconIndex i, const colour color){
		
	}

	void setTextColor(const uint16_t color, const uint16_t backgroundcolor){}
	void setTextSize(uint8_t size){}
	void setTextWrap(boolean w){}
	void write(uint8_t character){}
	void drawChar(const int16_t x, const int16_t y, char c, const uint16_t color, const uint16_t bg, uint8_t size){}

	void setScreenLock(const bool status){
		screenLock = status;
	}
	
	/*--------------------------------------------------------Basic Stuff-----------------------------------------------*/
	//int topOffset = font::getTextHeight(); //Size of mode bar at stop
	int topOffset = 20; //Size of mode bar at top of screen
	const int width  = 480;
	const int height = 320;
	// enum class varType : uint8_t{ //dELETE
		// normal = 0,
		// bg
	// };
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
	
	int rowSize = 20; //How far each text is away vertically
	int columnSize = 160; //How far each text is away horizontally
	int initialHeight = 280;
	
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