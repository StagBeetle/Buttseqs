#include <stdint.h> 

#include <Wire.h>
#define ISSI_REG_CONFIG_PICTUREMODE 0x00
#define ISSI_CONF_PICTUREMODE 0x00
#define ISSI_REG_CONFIG  0x00
#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_PICTUREFRAME  0x01
#define ISSI_ADDR_DEFAULT 0x74
#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B    // helpfully called 'page nine'

#define PWM_REGISTER 0x24 

const int _i2caddr = ISSI_ADDR_DEFAULT;
//Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();

void selectBank(uint8_t b) {
	Wire.beginTransmission(_i2caddr);
	Wire.write((byte)ISSI_COMMANDREGISTER);
	Wire.write(b);
	Wire.endTransmission();
}

void writeRegister8(uint8_t b, uint8_t reg, uint8_t data) {
  selectBank(b);
  Wire.beginTransmission(_i2caddr);
  Wire.write((byte)reg);
  Wire.write((byte)data);
  Wire.endTransmission();
}

void displayFrame(uint8_t f) {
	if (f > 7) f = 0;
	writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, f);
}