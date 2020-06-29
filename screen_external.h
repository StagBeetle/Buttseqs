/***************************************************
  This is our GFX example for the Adafruit ILI9488 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "ILI9488_t3.h"

#define TFT_RST 6//No longer the default 8 because that is used for the Teensy audio board
#define TFT_DC 9
#define TFT_CS 10

ILI9488_t3 tft = ILI9488_t3(&SPI, TFT_CS, TFT_DC, TFT_RST);
//ILI9488_t3 tft = ILI9488_t3(&SPI, 255, 255, 255);