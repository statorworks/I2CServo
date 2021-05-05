//
//I2C servo tester
//ATtiny816
//Based on servo project
//Note: Os size optimization requires variables to be declared with 'volatile'
//
//Note:Key protected cpu registers require to be written within four cycles after key.
//This requires it to be done in assembly. Protected_io.S, assembler.h and gas.h copied from AtmelStart flash demo for this purpose.
//
//Note: changing the device under properties->device changes the 'packs' setting from 1.3 to 1.4 giving a .bss error
//due to an atmel typo in one of the files. Manually revert to 1.3 to solve this problem.
//
//FUSES: BOD Enabled at 2.6v, FRQOSC 20Mhz, Reset pin config= UPDI,  watchdog timer 0.256Sec



#ifndef MAIN_H_
#define MAIN_H_

#include "Common/Common.h"
#include "I2C.h"
#include "EEPROM.h"
#include <avr/wdt.h>


#define I2C_SERVO_TESTER_FIRMWARE_VERSION 1 

//With size optimization debugging is difficult if variables are not 'volatile', but volatile increases code size.
//Commenting volatile here saves about 3% of code space for release.
#define VOLATILE //volatile   


#define MAX_AD_CHANNELS 12
//
#define GREEN_LED_ON		PORTC_OUTSET = PIN2_bm; 
#define GREEN_LED_OFF		PORTC_OUTCLR = PIN2_bm; 
#define GREEN_LED_TOGGLE	PORTC_OUTTGL = PIN2_bm; 
//
#define RED_LED_ON			PORTC_OUTSET = PIN3_bm;
#define RED_LED_OFF			PORTC_OUTCLR = PIN3_bm;
#define RED_LED_TOGGLE		PORTC_OUTTGL = PIN3_bm; 

//Can test a regular servo with this on auxiliary header terminal
#define SERVO_PULSE_UP		PORTC_OUTSET = PIN0_bm; 
#define SERVO_PULSE_DOWN	PORTC_OUTCLR = PIN0_bm; 


//Don't forget to include any headers. 
//attiny416 linking won't warn about undeclared items and runtime will have corruption.
//states
bool timeslice;
uint8_t state_time;
 
//I2C
#define I2C_N  0
#define I2C_IS_FREE (i2c_transaction_state==I2C_TRANSACTION_IDLE)
#define I2C_IS_READING (i2c_transaction_state==I2C_TRANSACTION_READ_COMBINED)
//i2c receive
uint8_t  i2cservo_in_data[I2C_MAX_TRANSFER_BYTES];
//
//valid, i2c addresses not specifically reserved by the standard.
//Some of the others act up, as they seem meant for alternative behavior.
#define I2C_ADDRESS_RANGE_START 4
#define I2C_ADDRESS_RANGE_END 120

//vars
uint8_t button_down_time;
uint8_t button_up_time;
uint16_t old_pot_pow;
uint16_t old_pot_pos;
uint8_t data_out[10];
uint8_t red_led_blink_count;
uint8_t green_led_blink_count;
uint8_t led_blink_period_count;
#define LED_BLINK_PERIOD 2


//Servo register offsets.
#define REG_FIRMWARE_VERSION		0x00
#define REG_I2C_ADDRESS				0x01
#define REG_STOP					0x02
#define REG_PARK_TYPE				0x03
#define REG_MOTOR_POLARITY			0x04
#define REG_CONTINUOUS_ROTATION		0x05
#define REG_MAX_POWER				0x06
#define REG_MAX_SPEED_H				0x07
#define REG_MAX_SPEED_L				0x08
#define REG_RAMP_TIME_H				0x09
#define REG_RAMP_TIME_L				0x0A
#define REG_RAMP_CURVE				0x0B
#define REG_DEADBAND				0x0C
//
#define REG_TARGET_POSITION_H		0x0D
#define REG_TARGET_POSITION_L		0x0E
#define REG_TARGET_POSITION_NEXT_H	0x0F  //allows setting next target without waiting or checking for completion
#define REG_TARGET_POSITION_NEXT_L	0x10  //automatically loaded into target position.
//
#define REG_CURRENT_STATE			0x11
#define REG_CURRENT_POSITION_H		0x12
#define REG_CURRENT_POSITION_L		0x13
#define REG_CURRENT_VELOCITY_H		0x14
#define REG_CURRENT_VELOCITY_L		0x15
#define REG_CURRENT_POWER			0x16
#define REG_CURRENT_TEMPERATURE		0x17
#define REG_LAST_CRC8			    0x18
//
#define REG_PROGRAM_POSITION_0_H    0x19  // 19,1A,1B,1C,1D,1E,1F,20,21,22,23,24,25,26,27,28
#define REG_PROGRAM_POSITION_0_L    0x1A  //don't list all the eight slots here
#define REG_PROGRAM_REPS            0x29  
#define REG_SAVE					0x2A  


//functions
void init(void);
void StateManager(void);
uint8_t GetDipSwitch(void);
uint16_t GetAD(uint8_t n);
void SendPulse(uint16_t time_us);//used to test a regular servo
//
void testing(void);
int16_t BE16(int16_t val);//switch endiannes on 16-bit data
uint32_t myabs(int32_t v);  //CC 6-20 built-in abs() is bad


#endif /* MAIN_H_ */