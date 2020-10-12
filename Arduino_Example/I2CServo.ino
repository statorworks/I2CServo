//
//Statorworks 2020
//I2C servo library
//Requires I2CWrapper.ino
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




void I2CServo_Begin(){
 //Nothing here, assumes i2c initlized by application
}


bool I2CServo_GetI2cAddress(byte address, byte* readadd, bool use_crc){

 return I2CWrapper_Read(address, I2CS_REG_ADDRESS, readadd, 1, use_crc);  //return 0 on failure

}


bool I2CServo_SetI2cAddress(byte address, byte new_address, bool use_crc){
 //Change i2c address. Change is immediate and address is saved to servo's non-volatile memory.
 //Further access needs to be to the new address.
 //Make sure only one servo is on i2c bus 
 
 return I2CWrapper_Write(address, I2CS_REG_ADDRESS, &new_address, 1, use_crc);
}

bool I2CServo_Stop(byte address, bool use_crc){
 //Stop motion immediately in current position without ramping, and enter park state.

 byte b=0x01;//stop 
 return I2CWrapper_Write(address, I2CS_REG_STOP, &b, 1, use_crc);
}


bool I2CServo_SetParkType(byte address, byte park_type, bool use_crc){
 //0=coast 
 //1= brake 
 //2=hold with MAX_POWER setting
 //3-100=hold with this number as power. This can be used if you want a different force after achieving position.
  
 return I2CWrapper_Write(address, I2CS_REG_PARK_TYPE, &park_type, 1, use_crc);   
}


bool I2CServo_SetMotorPolarity(byte address, byte motor_polarity, bool use_crc){
 //0=reverse 1=forward
 //Use this bit if the motor wires have been soldered backwards. normally should be 1.
 //If you have doubts, test with a low power setting, and watch if the servo avoids instead of seeking the target position.
 
 return I2CWrapper_Write(address, I2CS_REG_MOTOR_POLARITY, &motor_polarity, 1, use_crc);
}


bool I2CServo_SetMultiturnType(byte address, byte multiturn_type, bool use_crc){
 //0= no multiturn, normal operation, 0-1024 range.  
 //1=servo mechanicaly free, pot still senses angle. bits 10-15 of current position hold turn count
 //2=Servo mechanicaly free, pot fixed. in this case MAX_POWER controls speed. 
  
 return I2CWrapper_Write(address, I2CS_REG_MULTITURN_TYPE, &multiturn_type, 1, use_crc);
}


bool I2CServo_SetMaxPower(byte address, byte max_power, bool use_crc){
 //0-100% limit power applied trying to seek or hold position.
  
 return I2CWrapper_Write(address, I2CS_REG_MAX_POWER, &max_power, 1, use_crc);
}


bool I2CServo_SetMaxSpeed(byte address, uint16_t max_speed, bool use_crc){
 //Speed cap in position units per second. typical 2000 for a normal 0.15S/60* servo.

 byte b[2];
 b[0] = (max_speed>>8);    //make sure to break down 16-bit value like this, so endianness on your system doesn't reverese the order.
 b[1] = (max_speed&0xFF);  //
 return I2CWrapper_Write(address, I2CS_REG_MAX_SPEED_H, &b[0], 2, use_crc);
}


bool I2CServo_SetRampTime(byte address, uint16_t ramp_time, bool use_crc){
 //Ramp time in milliseconds.
 //Time for 0-MAX_SPEED, and same for MAX_SPEED-0. Assuming load and MAX_POWER permit it.
 
 byte b[2];
 b[0] = (ramp_time>>8);    //make sure to break down 16-bit value like this, so endianness on your system doesn't reverese the order.
 b[1] = (ramp_time&0xFF);  //
 
 return I2CWrapper_Write(address, I2CS_REG_RAMP_TIME_H, &b[0], 2, use_crc);
}


bool I2CServo_SetRampCurve(byte address, byte ramp_curve, bool use_crc){
 //0=linear speed ramp
 //100=sine speed profile 
 
 return I2CWrapper_Write(address, I2CS_REG_RAMP_CURVE, &ramp_curve, 1, use_crc);
}


bool I2CServo_SetDeadband(byte address, byte deadband, bool use_crc){
 //0-10
 if(deadband<1){deadband=1;}
 if(deadband>20){deadband=20;} 
 return I2CWrapper_Write(address, I2CS_REG_DEADBAND, &deadband, 1, use_crc);
}


bool I2CServo_SetupAll(byte address, byte park_type, byte motor_polarity, byte multiturn_type, byte max_power, uint16_t max_speed, uint16_t ramp_time, byte ramp_curve, bool use_crc){

 byte b[9];
 b[0] = park_type;
 b[1] = motor_polarity;
 b[2] = multiturn_type;
 b[3] = max_power;
 b[4] = (max_speed>>8);    //make sure to break down 16-bit value like this, so endianness on your system doesn't reverese the order.
 b[5] = (max_speed&0xFF);  //
 b[6] = (ramp_time>>8);    //''
 b[7] = (ramp_time&0xFF);  //
 b[8] = ramp_curve;
   
 return I2CWrapper_Write(address, I2CS_REG_PARK_TYPE, &b[0], 9, use_crc);
}



bool i2cServo_GetCurrentState(byte address, byte* state, bool use_crc){

 if(state==0){return 0;}
 
 return I2CWrapper_Read(address, I2CS_REG_CURRENT_STATE, state, 1, use_crc);
}


bool i2cServo_GetCurrentPosition(byte address, int16_t* pos, bool use_crc) {

 if(pos==0){return 0;}
 
 byte dat[2];
 bool res = I2CWrapper_Read(address, I2CS_REG_CURRENT_POSITION_H, &dat[0], 2, use_crc);
 if(res==0){return 0;}
 
 *pos = (int16_t)dat[0]<<8 | dat[1];
 return 1;//Ok
}

bool i2cServo_GetCurrentVelocity(byte address, int16_t* vel, bool use_crc) {

 if(vel==0){return 0;}

 byte dat[2];
 bool res = I2CWrapper_Read(address, I2CS_REG_CURRENT_VELOCITY_H, &dat[0], 2, use_crc);
 if(res==0){return 0;}
 
 *vel = (int16_t)dat[0]<<8 | dat[1];
 return 1;//Ok
}


bool i2cServo_GetCurrentPower(byte address, int8_t* power, bool use_crc){

 if(power==0){return 0;}
 
 return I2CWrapper_Read(address, I2CS_REG_CURRENT_POWER, power, 1, use_crc);
}

bool i2cServo_GetCurrentTemp(byte address, byte* temp, bool use_crc){

 if(temp==0){return 0;}
 
 return I2CWrapper_Read(address, I2CS_REG_CURRENT_TEMPERATURE, temp, 1, use_crc);
}

bool i2cServo_GetLastcrc8(byte address, byte* lastcrc8, bool use_crc){

 if(lastcrc8==0){return 0;}
 
 return I2CWrapper_Read(address, I2CS_REG_LAST_CRC8, lastcrc8, 1, use_crc);
}

bool i2cServo_GetAllStatus(byte address, byte* state, int16_t* pos, int16_t* vel, int8_t* power, byte* temp, byte* lastcrc8, bool use_crc){
 
 byte dat[8];
 bool res = I2CWrapper_Read(address, I2CS_REG_CURRENT_STATE, &dat[0], 8, use_crc);
 if(res==0){return 0;}// no response. user should make sure to check this

 //fill the ones the user has provided pointers to
 if(state!=0){ *state = dat[0];}
 //if(pos!=0)  { *pos = (int16_t)((uint16_t)dat[1]<<8 | dat[2]); }
 if(pos!=0)  { *pos = (int16_t)dat[1]<<8 | dat[2]; } 
 //if(vel!=0)  { *vel = (int16_t)((uint16_t)dat[3]<<8 | dat[4]); }
 if(vel!=0)  { *vel = (int16_t)dat[3]<<8 | dat[4]; }
 if(power!=0){ *power = (int8_t)dat[5];}
 if(temp!=0) { *temp  = dat[6];}
 if(lastcrc8!=0){ *lastcrc8 = dat[7];}

 return 1; //Ok
}


bool I2CServo_SetTargetPosition(byte address, int16_t target_position, bool use_crc){
 //for normal mode, range is 0-1024  
 //for multiturn range is -32000-32000
 //Note that this cancels previously pushed 'next' positions and stops running stored program 
    
 byte b[2];
 b[0] = (uint16_t)(target_position)>>8;    
 b[1] = (uint16_t)(target_position)&0xFF; 
 
 return I2CWrapper_Write(address, I2CS_REG_TARGET_POSITION_H, &b[0], 2, use_crc);
}


bool I2CServo_SetNextTargetPosition(byte address, int16_t next_position, bool use_crc){
 //You can push new positions (up to 8) without waiting for current motion to end
 //Note that these get cancelled by a regular TARGET_POSITION write.
    
 byte b[2];
 b[0] = (uint16_t)(next_position)>>8;    
 b[1] = (uint16_t)(next_position)&0xFF; 
 
 return I2CWrapper_Write(address, I2CS_REG_TARGET_POSITION_NEXT_H, &b[0], 2, use_crc); 
}


bool I2CServo_SetProgramSlot(byte address, byte slot_number, int16_t target_position, bool use_crc){
 //the slot we do check here to make sure we don't write past the alloted register
 if(slot_number>7){ slot_number=7; }
     
 byte b[2];
 b[0] = (uint16_t)(target_position)>>8;    
 b[1] = (uint16_t)(target_position)&0xFF; 
 
 return I2CWrapper_Write(address, (I2CS_REG_PROGRAM_POSITION_0_H + (slot_number*2)), &b[0], 2, use_crc);
}


bool I2CServo_SetProgramReps(byte address, byte reps, bool use_crc){
//0=stop 255=infinite
 byte b=reps;
 return I2CWrapper_Write(address, I2CS_REG_PROGRAM_REPS, &b, 1, use_crc);
}


bool I2CServo_SetFullProgram(byte address, int16_t* target_positions, byte reps, bool use_crc){
 //please set the unused position slots to zero

 //check
 if(target_positions==0){return 0;}

 //we make sure to break down the 16-bit position values explicitly, 
 //so endianness on your system doesn't reverse the byte order.     
 byte b[19];
 byte n=0;
 for(byte i=0; i<8; i++){
   b[n] = (uint16_t)(target_positions[i])>>8;    
   b[n+1] = (uint16_t)(target_positions[i])&0xFF; 
   n+=2;
 }
 //
 b[16]=reps;  //how many times
 
 return I2CWrapper_Write(address, I2CS_REG_PROGRAM_POSITION_0_H, &b[0], 17, use_crc);
}

bool I2CServo_SaveAll(byte address, bool use_crc){

 byte b=1; 
 return I2CWrapper_Write(address, I2CS_REG_SAVE, &b, 1, use_crc); 
}




