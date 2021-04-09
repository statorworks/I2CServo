//
//I2C master or slave
//ATTINY416
//4-3-21 i2c as master for servo tester
//Author: Camilo
//Note: Os size optimization requires variables to be declared with 'volatile'
//todo: Resolve bus occasionally stuck with lines held low when disconnected and reconnected. 


#ifndef I2C_H_
#define I2C_H_

#include "Common/Common.h"

//adjust these for application
#define I2C_MAX_TRANSFER_BYTES 41   //prevent access exception
#define I2C_MAX_REG_COUNT 41		//prevent access exception
//
#define I2C_CLOCK_FREQ 10000000
//
#define I2C_MASTER 1   //less code space usage
//
#define I2C_BROADCAST_ADDRESS  0x00  //respond to standard i2c broadcast address until user sets new address and it's saved on eeprom


//Bus states (low level). see datasheet
#define I2C_MASTER_BUS_STATES_UNKNOWN	0
#define I2C_MASTER_BUS_STATES_IDLE		1
#define I2C_MASTER_BUS_STATES_OWNER		2
#define I2C_MASTER_BUS_STATES_BUSY		3

//CC 6-20
#define I2C_BUS_LOCKUP_TIMEOUT_MS 20

//I2c
//todo put this in a struct
volatile bool i2c_initialized;
volatile uint8_t	i2c_transaction_state; //read or write
#define I2C_TRANSACTION_IDLE   0
//#define I2C_TRANSACTION_READ   1			//addressR, data  (not supported here)
#define I2C_TRANSACTION_READ_COMBINED   2   //addressW, reg, addressR, data, data
#define I2C_TRANSACTION_WRITE  3

//simple interface variables
volatile uint8_t i2c_transfer_address;   //address 0-127  //Application sets this by calling set_address()
volatile uint8_t i2c_transfer_register;  //first register of access
volatile uint8_t* i2c_transfer_in_data;  //pointer to application receive buffer. Application sets this.
volatile uint8_t* i2c_transfer_out_data; //pointer to application send buffer. Application sets this.
volatile uint8_t i2c_transfer_length;    //expected data length
volatile uint8_t i2c_transfer_count;     //actual sent or received bytes
volatile uint8_t i2c_transfer_step;      //the state machine to track transfer
volatile bool	i2c_transfer_done;       //on stop bit. 
volatile bool	i2c_transfer_failed;     //something went wrong. 


//Optional crc8. Not part of the protocol but should make things more robust.
//The master sets the msb of the reg byte to signal crc desired. then a length byte must follow so slave can identify the last byte.
//example write: Address, 0x80, Length, 0x01, 0x02, 0x03, CRC  
volatile bool	i2c_use_crc;       //User sets this if master. The driver sets if slave.
volatile uint8_t i2c_crc_calc;     //On the go calculation as bytes are sent or received
volatile uint8_t i2c_crc_value;    //Last byte received, compared to crc_calc
volatile bool	i2c_crc_ok;        //After comparing

//lockup check
volatile uint16_t i2c_busy_time;
volatile uint32_t i2c_freq;

//Functions
bool i2c_Init(uint8_t i2c_n, bool master, uint8_t addr, uint32_t freq_hz);
void i2c_Check(uint8_t lapse_ms); //call every 1ms to look for bus lockup. No way to monitor this from interrupts alone.
	
//slave
void I2C_SlaveSetAddress(uint8_t address);
volatile void (*i2c_receive_complete_callback)(void); //Application sets this.

//master
bool I2C_MasterWrite(uint8_t i2c_n, uint8_t address, uint8_t register_offset, uint8_t* data, uint8_t length);
bool I2C_MasterRead(uint8_t i2c_n, uint8_t address, uint8_t register_offset, uint8_t* data, uint8_t length);
volatile void (*i2c_send_complete_callback)(void);    //Application sets this.

//optional crc8 checksum, not part of i2c protocol
//crc8-SMBus type
//width=8 poly=0x07 init=0x00 refin=false refout=false xorout=0x00 check=0xf4 residue=0x00 name="CRC-8/SMBUS"
uint8_t crc8(uint8_t* data, uint8_t length);
void crc8byte(uint8_t* crc_val, uint8_t val);

#endif
