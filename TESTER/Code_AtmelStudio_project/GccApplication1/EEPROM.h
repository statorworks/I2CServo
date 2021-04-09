//
//EEPROM.h
//NVMCTRL ATTINY416
//Based on samd51 driver 
//  Author: CAMILO
//
//Key protected Nvmctrl commands require command to be written within four cycles after key.
//This requires it to be done in assembly. Protected_io.S, assembler.h and gas.h copied from AtmelStart flash demo for this purpose.

#ifndef EEPROM_H_
#define EEPROM_H_

#include "Common/Common.h"



//bool EEPROM_initialized;

//max size
//bool EEPROM_Init(void);  //not needed
bool EEPROM_Write(uint16_t offset, uint8_t* data, uint16_t length);
bool EEPROM_Read(uint16_t offset, uint8_t* data, uint16_t length);

#endif
