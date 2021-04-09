//
// Common.h
// ATTINY416
// Created: 5/6/2019 5:35:38 PM
//  Author: Camilo
// 
//Whatever needs to be seen by all modules go here.

#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>		   //mcu definitions
#include <avr/interrupt.h> //
#include <stdbool.h>	   //support for boolean type


//system
#define DISABLE_INTERRUPTS  cli() //SREG &= ~(0x80)
#define ENABLE_INTERRUPTS   sei() // SREG |= 0x80
#define SYSTEM_CLOCK_FREQ 20000000  //internal 20Mhz
#define PERIPH_CLOCK_FREQ 10000000  //with peripheral prescaler = x2   10Mhz@3.3v

//ATTINY416 abs pin count
#define PIN_ATTINY_PA0 0
#define PIN_ATTINY_PA1 1
#define PIN_ATTINY_PA2 2
#define PIN_ATTINY_PA3 3
#define PIN_ATTINY_PA4 4
#define PIN_ATTINY_PA5 5
#define PIN_ATTINY_PA6 6
#define PIN_ATTINY_PA7 7
#define PIN_ATTINY_PB0 8
#define PIN_ATTINY_PB1 9
#define PIN_ATTINY_PB2 10
#define PIN_ATTINY_PB3 11
#define PIN_ATTINY_PB4 12
#define PIN_ATTINY_PB5 13
#define PIN_ATTINY_PB6 14
#define PIN_ATTINY_PB7 15
#define PIN_ATTINY_PC1 16
#define PIN_ATTINY_PC2 17
#define PIN_ATTINY_PC3 18


#define OK 1
#define KO 0

//This assembly function is needed on ATTINY416 to write key protected registers and memory in under 4-cycles.
//Obtained from AtmelStart flash demo, protected_io.S assembler.h and gas.h.
//don't forget to add include directory to assembler options i.e. $(SolutionDir)\GccApplication1 
extern void protected_write_io(void *addr, uint8_t magic, uint8_t value);

#endif /* COMMON_H_ */