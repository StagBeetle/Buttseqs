#include "screen.h"

#include "SPI.h"
#include "ILI9488_t3.h"
#include "ili9488_t3_font_Arial.h"
#include "ili9488_t3_font_ArialBold.h"

#define TFT_RST 6//No longer the default 8 because that is used for the Teensy audio board
#define TFT_DC 9
#define TFT_CS 10

ILI9488_t3 tft = ILI9488_t3(&SPI, TFT_CS, TFT_DC, TFT_RST);
//ILI9488_t3 tft = ILI9488_t3(&SPI, 255, 255, 255);

namespace scrn{
	const int brightnessPin = A0;
	
	theme mainTheme = {{
		{ 50,  50,  50},	//BG
		{100, 100, 100},	//Accent 1
		{150, 150, 150},	//Accent 2
		{200, 200, 200},	//FG
		{255, 200,  10},	//text
		{200,  50,  20},	//highlight
	}};
	
	colour getThemeColour(const int index){
		return mainTheme.get(index);
	}
	
	uint8_t getColourChannelValue(const int editColourDescriptor, const int editColourChannel){
		return mainTheme.getColourChannelValue(editColourDescriptor, editColourChannel);
	}
	
	void editColour(const uint8_t editColourDescriptor, const uint8_t editColourChannel, const uint8_t value){
		mainTheme.editColour(editColourDescriptor, editColourChannel, value);
	}
	
	/*--------------------------------------------------------Send Stuff-----------------------------------------------*/
	
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
	
	bool screenLock = false; //Unused
	bool hasChanged = false; //Unused
	
	const uint8_t eventOffset = static_cast<uint8_t>(iconIndex::press);

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
		tft.setFont(Arial_12);
		tft.setOrigin(0,0);
		//tft.invertDisplay(1);
		tft.useFrameBuffer(true);
		//tft.useFrameBuffer(true); 
	}
	
	void update(){
		if(screenLock){return;}
		if(hasChanged){
			//tft.updateScreen();
			tft.updateScreenAsync(true);
			hasChanged = false;
		}
	}
		
	void fillScreen(const colour c){
		tft.fillScreen(c.gSC());
		hasChanged = true;
	}
	
	void blankScreen(){
		fillScreen(getThemeColour(scrn::td::bg));
		hasChanged = true;
	}

	void writeFillRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c){
		tft.fillRect(x, y, w, h, c.gSC());
		hasChanged = true;
	}
	
	void writeDrawRect(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const colour c){
		tft.drawRect(x, y, w, h, c.gSC());
		hasChanged = true;
	}
	
	void write(const int16_t x, const int16_t y, const char *str){
		tft.setCursor(x, y);
		tft.print(str);
		hasChanged = true;
	}
	
	void write(const int16_t x, const int16_t y, int num){
		char string[6] = {0};
		sprintf(string, "%i", num);
		write(x, y, string);
		hasChanged = true;
	}
	
	void write(const int16_t x, const int16_t y, double num){
		char string[32] = {0};
		sprintf(string, "%f", num);
		write(x, y, string);
		hasChanged = true;
	}
	
	void setTextColor(const colour c){
		tft.setTextColor(c.gSC());
		hasChanged = true;
	}
	
	void writeFastVLine(const int16_t x, const int16_t y, const int16_t h, const colour c){
		tft.drawFastVLine(x, y, h, c.gSC());
		hasChanged = true;
	}
	
	void writeFastHLine(const int16_t x, const int16_t y, const int16_t w, const colour c){
		tft.drawFastHLine(x, y, w, c.gSC());
		hasChanged = true;
	}
	void writeLine(const int16_t x, const int16_t y, const int16_t x2, const int16_t y2, const colour c){
		tft.drawLine(x, y, x2, y2, c.gSC());
		hasChanged = true;
	}
	void writeFillTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c){
		tft.fillTriangle(x0, y0, x1, y1, x2, y2, c.gSC());
		hasChanged = true;
	}
	void writeEdgeTri(const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const colour c){
		tft.drawTriangle(x0, y0, x1, y1, x2, y2, c.gSC());
		hasChanged = true;
	}
	void writePixel(const int16_t x, const int16_t y, const colour c){
		tft.drawPixel(x, y, c.gSC());
		hasChanged = true;
	}

	void drawCircle(const int x, const int y, const int r, const colour c){
		tft.drawCircle(x, y, r, c.gSC());
		hasChanged = true;
	}
	
	void drawFillCircle(const int x, const int y, const int r, const colour c){
		tft.fillCircle(x, y, r, c.gSC());
		hasChanged = true;
	}

	void setTextColor(const uint16_t color, const uint16_t backgroundcolor){}
	void setTextSize(uint8_t size){}
	void setTextWrap(boolean w){}
	void write(uint8_t character){}
	void drawChar(const int16_t x, const int16_t y, char c, const uint16_t color, const uint16_t bg, uint8_t size){}
	
	void setScreenLock(const bool status){
		screenLock = status;
	}

	uint8_t to255(float v) {
		return (int)min(255,256*v);
	}

	/** Helper method that converts hue to rgb */
	float hueToRgb(float p, float q, float t) {
		if (t < 0)
			t += 1;
		if (t > 1)
			t -= 1;
		if (t < 1/6)
			return p + (q - p) * 6 * t;
		if (t < 1/2)
			return q;
		if (t < 2/3)
			return p + (q - p) * (2/3 - t) * 6;
		return p;
	}
	
	colour hslToRgb(float h, float s, float l){
		float r, g, b;

		if (s == 0) {
			r = g = b = l; // achromatic
		} else {
			float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
			float p = 2 * l - q;
			r = hueToRgb(p, q, h + 1/3);
			g = hueToRgb(p, q, h);
			b = hueToRgb(p, q, h - 1/3);
		}
		return {to255(r), to255(g), to255(b)};
	}
	
	/*--------------------------------------------------------Basic Stuff-----------------------------------------------*/
	//int topOffset = font::getTextHeight(); //Size of mode bar at stop
	int topOffset = 20; //Size of mode bar at top of screen
	int rowSize = 20; //How far each text is away vertically
	int columnSize = 160; //How far each text is away horizontally
	int initialHeight = 280;
	
}
