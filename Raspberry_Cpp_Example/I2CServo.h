#ifndef I2CSERVO_H
#define I2CSERVO_H
//
//Statorworks 2020
//I2C C servo library
//Requires I2CWrapper module. Servo function calls assume application initialized the i2c port.
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

#include <stdint.h>

//I2C abstraction here
//#include "i2c_ardu.h"
#include "i2c_raspy.h"
#define I2C_SERVO_WRITE(add, reg, dat, len, crc)  I2C_Write(add, reg, dat, len, crc )
#define I2C_SERVO_READ(add, reg, dat, len, crc)   I2C_Read(add, reg, dat, len, crc )


//Servo register space
#define I2CS_REG_FIRMWARE_VERSION       0x00
#define I2CS_REG_ADDRESS                0x01
#define I2CS_REG_STOP                   0x02
#define I2CS_REG_PARK_TYPE              0x03
#define I2CS_REG_MOTOR_POLARITY         0x04
#define I2CS_REG_MULTITURN_TYPE         0x05
#define I2CS_REG_MAX_POWER              0x06
#define I2CS_REG_MAX_SPEED_H            0x07
#define I2CS_REG_MAX_SPEED_L            0x08
#define I2CS_REG_RAMP_TIME_H            0x09
#define I2CS_REG_RAMP_TIME_L            0x0A
#define I2CS_REG_RAMP_CURVE             0x0B
#define I2CS_REG_DEADBAND               0x0C
//
#define I2CS_REG_TARGET_POSITION_H      0x0D
#define I2CS_REG_TARGET_POSITION_L      0x0E
#define I2CS_REG_TARGET_POSITION_NEXT_H 0x0F  
#define I2CS_REG_TARGET_POSITION_NEXT_L 0x10
//
#define I2CS_REG_CURRENT_STATE          0x11
#define I2CS_REG_CURRENT_POSITION_H     0x12
#define I2CS_REG_CURRENT_POSITION_L     0x13
#define I2CS_REG_CURRENT_VELOCITY_H     0x14
#define I2CS_REG_CURRENT_VELOCITY_L     0x15
#define I2CS_REG_CURRENT_POWER          0x16
#define I2CS_REG_CURRENT_TEMPERATURE    0x17
#define I2CS_REG_LAST_CRC8              0x18
//
#define I2CS_REG_PROGRAM_POSITION_0_H   0x19
#define I2CS_REG_PROGRAM_POSITION_0_L   0x1A  //...
#define I2CS_REG_PROGRAM_REPS           0x29  //+0x10
#define I2CS_REG_SAVE                   0x2A


void I2CServo_Begin(void);
bool I2CServo_GetI2cAddress(uint8_t address, uint8_t* readadd, bool use_crc);
bool I2CServo_SetI2cAddress(uint8_t address, uint8_t new_address, bool use_crc);
bool I2CServo_Stop(uint8_t address, bool use_crc);
bool I2CServo_SetParkType(uint8_t address, uint8_t park_type, bool use_crc);
bool I2CServo_SetMotorPolarity(uint8_t address, uint8_t motor_polarity, bool use_crc);
bool I2CServo_SetMultiturnType(uint8_t address, uint8_t multiturn_type, bool use_crc);
bool I2CServo_SetMaxPower(uint8_t address, uint8_t max_power, bool use_crc);
bool I2CServo_SetMaxSpeed(uint8_t address, uint16_t max_speed, bool use_crc);
bool I2CServo_SetRampTime(uint8_t address, uint16_t ramp_time, bool use_crc);
bool I2CServo_SetRampCurve(uint8_t address, uint8_t ramp_curve, bool use_crc);
bool I2CServo_SetDeadband(uint8_t address, uint8_t deadband, bool use_crc);
bool I2CServo_Setup(uint8_t address, uint8_t park_type, uint8_t motor_polarity, uint8_t multiturn_type, uint8_t max_power, uint16_t max_speed, uint16_t ramp_time, uint8_t ramp_curve, bool use_crc);
//
bool i2cServo_GetCurrentState(uint8_t address, uint8_t* state, bool use_crc);
bool i2cServo_GetCurrentPosition(uint8_t address, uint16_t* pos, bool use_crc);
bool i2cServo_GetCurrentVelocity(uint8_t address, int16_t* vel, bool use_crc);
bool i2cServo_GetCurrentPower(uint8_t address, uint8_t* power, bool use_crc);
bool i2cServo_GetCurrentTemp(uint8_t address, uint8_t* temp, bool use_crc);
bool i2cServo_GetLastcrc8(uint8_t address, uint8_t* lastcrc8, bool use_crc);
bool i2cServo_GetAllStatus(uint8_t address, uint8_t* state, uint16_t* pos, int16_t* vel, int8_t* power, uint8_t* temp, uint8_t* lastcrc8, bool use_crc);
//
bool I2CServo_SetTargetPosition(uint8_t address, int16_t target_position, bool use_crc);
bool I2CServo_SetNextTargetPosition(uint8_t address, int16_t next_position, bool use_crc);
//
bool I2CServo_SetProgramSlot(uint8_t address, uint8_t slot_number, int16_t target_position, bool use_crc);
bool I2CServo_SetProgramReps(uint8_t address, uint8_t reps, bool use_crc);
bool I2CServo_SetFullProgram(uint8_t address, int16_t* target_positions, uint8_t reps, bool use_crc);
bool I2CServo_SaveAll(uint8_t address, bool use_crc);

#endif




























