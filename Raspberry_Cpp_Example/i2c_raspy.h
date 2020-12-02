#ifndef I2C_RASPY_H
#define I2C_RASPY_H
//
//Statorworks 2020
//Simple wrapper for Raspbian I2C.
/* 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
//-------------------------
//SYSTEM PREPARATION:
//
//-Un-blacklist:
//sudo nano /etc/modprobe.d/raspi-blacklist.conf
//comment out #blacklist i2c-bcm2708
//
//sudo nano /etc/modules
//add 'snd-bcm2835'
//
//-Check list of boot-load time modules
//and add two more modules to the list there:
//i2c-dev 
//i2c-bcm2708
//
//-Create a file /etc/udev/rules.d/90-i2c.rules and add the line:
//KERNEL=="i2c-[0-7]",MODE="0666"
//
//-(Optional) To adjust the clock speed of the I2C
//create file /etc/modprobe.d/i2c.conf with this line:
//options i2c_bcm2708 baudrate=400000
//
//-i2c pacakges setup. this can be put on a script
//sudo apt-get update
//sudo apt-get install python-smbus
//sudo apt-get install i2c-tools
//sudo adduser pi i2c
//sudo apt-get install libi2c-dev
//sudo reboot
//
//if you don't want to use I2C as an administrator
//give everyone read-write access to the devices:
//sudo chmod o+rw /dev/i2c-*
//
//-Check i2c bus present on system
//i2cdetect -y 0        <<or -y 1
//

#include <stdint.h>
#include <stdio.h>
//
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>  //<file close() declared here
//
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>


//These to change frequency by accessing bcm2835 register directly
//Thanks to Un-clouded:
//github.com/un-clouded/rpi-stuff/blob/master/i2cf/i2cf.c
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define  CORE_CLK  (250*1000*1000)
// "BSC" is Broadcom Serial Controller, which means I2C controller
#define  BSC_SIZE  0x20
#define  BSC1_BASE  0x20804000
#define  BSC_DIV_OFFSET   0x14
extern uint32_t  *registers;
#define  DIV  (registers [BSC_DIV_OFFSET / sizeof (*registers)])


#define I2C_MIN_FREQ 1000
#define I2C_MAX_FREQ 400000


//optional print
#define  I2C_DEBUG_PRINT(x)  printf(x) 


extern int i2c_file;
extern uint8_t I2C_LastCrc;
extern bool I2C_crc_ok;
extern bool I2C_initialized;

extern struct i2c_msg i2c_msgs[2];
extern struct i2c_rdwr_ioctl_data i2c_msgset;
extern uint8_t i2c_obuff[32];
extern uint8_t i2c_ibuff[32];

bool I2C_init(void);
bool I2C_close(void);
bool I2C_Read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len, bool use_crc);
bool I2C_Write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len, bool use_crc);
//
bool I2C_FreqSet(uint32_t f);//UNTESTED
bool I2C_IsInit(void);
bool I2C_WasCrcOk(void);
uint8_t I2C_GetLastCrc(void);
//
uint8_t I2C_crc_calculate(uint8_t address, uint8_t reg, uint8_t len, uint8_t* data);
void I2C_crc8byte(uint8_t* crc_val, uint8_t val);

#endif
