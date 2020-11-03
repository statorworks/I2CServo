//
//Statorworks 2020
//I2C Servo Example. Cpp on Raspbian
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
//
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
//if you dont want to use I2C as an administrator
//give everyone read-write access to the devices:
//sudo chmod o+rw /dev/i2c-*
//
//-Check i2c bus present on system
//i2cdetect -y 0        <<or -y 1
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
//
#include <sys/ioctl.h>
#include <fcntl.h>
//
#include "i2c_raspy.h"
#include "i2cfreq.h"
#include "I2CServo.h"

void MyuSleep(uint32_t us);
void MyDelay(uint32_t ms);
volatile u_int32_t i;  
byte data[16];  
byte i2c_address = 0x00;
bool result;

 
int main (int argc, char *argv[]){


  printf("start\n");
  
  //I2C
  I2C_init();
  MyuSleep(100000);

  
  //Loop
  while (1) {//uncomment the examples as necessary

   //----------------------------------------
	//EXAMPLE #1
	//Read I2C address just to confirm communication. 
	
	result = I2CServo_GetI2cAddress(i2c_address, &data[0]);
	if(result==1){ 
	Serial.println("Responded Ok\n");
	Serial.println(data[0]);
	Serial.println("\n");
	else{ 
	printf("Did not respond\n");
	MyDelay(1000); 	
 
 
	//----------------------------------------
	//EXAMPLE #2
	//Change i2c address. Change is immediate and address is saved to servo's non-volatile memory.
	//Further access needs to be to the new address.
	//Make sure only one servo is on i2c bus
	/*
	I2CServo_SetI2cAddress(i2c_address, 0x25,0);  //0-127
	i2c_address = 0x25;
	MyDelay(1000); 
	*/
 
	//----------------------------------------
	//EXAMPLE #3
	//Scan i2c address space in case you forgot the servo's address.
	//Make sure only one servo is on i2c bus
	/*
	for(byte i =0; i<=127;i++){
		if(I2CServo_GetI2cAddress(i, &data[0])==1){
			printf(i);
			printf(" responded.\n");
			break;
		}
	  }
	MyDelay(2000);  
	*/
 
	//----------------------------------------
	//EXAMPLE #4
	//Stop motion immediately in current position without ramping, and enter park state.
	/*
	I2CServo_Stop(i2c_address);
	MyDelay(1000);
	*/
 
	//----------------------------------------
	//EXAMPLE #5
	//Set running parameter(s)
	//User can set one item at a time or all simultaneously, see available functions.
	//I2CServo_SetParkType(i2c_address, 0); //0=coast, 1= brake, 2=hold with MAX_POWER setting, 3-100=hold with this number as power.
	//I2CServo_SetMaxPower(i2c_address, 50, 0); //0-100% cap
	//I2CServo_SetDeadband(i2c_address, 2, 0);
	//I2CServo_Setup(i2c_address, 100, 1, 0, 100, 3000, 1, 0, 0); //address, park_type, motor_polarity, continuous_rotation, max_power, max_speed, ramp_time ms, ramp_curve, use_crc
	//MyDelay(1);

	//----------------------------------------
	//EXAMPLE #6
	//Simple motion, back and forth
 
	//I2CServo_SetTargetPosition(i2c_address, 300, 0);
	//MyDelay(3000);
 
	//I2CServo_SetTargetPosition(i2c_address, 700, 0);
	//MyDelay(3000);
 

	//----------------------------------------
	//EXAMPLE #7
	//You can push new positions (up to 8) without waiting for current motion to end
	//Note that these get cancelled by a regular TARGET_POSITION write.
	//The power setting must be sufficient to overcome the load in order to reach each of the positions. 
	/*
	I2CServo_SetNextTargetPosition(i2c_address, 300, 0);
	MyDelay(1);
	I2CServo_SetNextTargetPosition(i2c_address, 700, 0);
	MyDelay(1);
	I2CServo_SetNextTargetPosition(i2c_address, 500, 0);
	MyDelay(1);  
	MyDelay(5000);
	*/

	//----------------------------------------
	//EXAMPLE #8
	//Continuous Rotation
	//Servo free to rotate. Potentiometer fixed, or free rotating. Control speed only by power setting. ramp time slope is time to to 100%power in ms
	//If there is a free-rotating pot, you can still consult its current position. 
	/*
	I2CServo_Setup(i2c_address, 1, 1, 2, 0, 0, 4000, 0, 0); //park_type, motor_polarity, multiturn_type(2), max_power, max_speed, ramp_time ms, ramp_curve, use_crc
	MyDelay(2);
	//
	I2CServo_SetMaxPower(i2c_address, 50, 0);
	MyDelay(6000);
	I2CServo_SetMaxPower(i2c_address, 0, 0);
	MyDelay(6000); 
	I2CServo_SetMaxPower(i2c_address, -50, 0);
	MyDelay(6000);
	I2CServo_SetMaxPower(i2c_address, 0, 0);
	MyDelay(6000);
	*/

	//----------------------------------------
	//EXAMPLE #9
	//Read status register(s)
	//user can get one item at a time or all simultaneously, see available functions.
	/*
	byte state, temp, lastcrc8;
	int8_t power;
	int16_t pos, vel;
	//i2cServo_GetCurrentTemp(i2c_address, &temp, 0);
	//i2cServo_GetCurrentPosition(i2c_address, &pos, 0);
	i2cServo_GetAllStatus(i2c_address, &state, &pos, &vel, &power, &temp, &lastcrc8, 0);  //provide zero pointer for the items you don't want
	// 
	printf(pos);
	printf(" ");
	printf(power); 
	printf(" ");
	printf(temp);
	//
	MyDelay(100);
	*/
 
	//----------------------------------------
	//EXAMPLE #10
	//Create stored program
	//User can set one item at a time
	//I2CServo_SetProgramSlot(i2c_address, 0, 300, 0);
	//MyDelay(10);
	//I2CServo_SetProgramSlot(i2c_address, 1, 700, 0);
	//MyDelay(10);
	//I2CServo_SetProgramSlot(i2c_address, 2, 500, 0);  
	//MyDelay(10);
	//I2CServo_SetProgramSlot(i2c_address, 2,   0, 0); //0 marks end of programmed positions
	//MyDelay(10);
	//I2CServo_SetProgramReps(i2c_address, 15, 0);      //255= infinite
	//
	//Or set up all at once:
	//int16_t pp[8] = { 300, 700, 500, 0, 0, 0, 0, 0 };
	//I2CServo_SetFullProgram(i2c_address, &pp[0], 1, 15, 1, 0);  //positions array, direction, reps, start, use crc
	//MyDelay(1);

	//I2CServo_SaveAll(i2c_address, 0);
	//MyDelay(5);
 
	//Options to stop the running program (choose one):
	//I2CServo_SetProgramReps(i2c_address, 0, 0);           //reps=0,1,2 etc: reduce the number of reps left
	//I2CServo_SetNextTargetPosition(i2c_address, 800, 0);  //program ends at completion of current move and servo goes to new defined position 
	//I2CServo_SetTargetPosition(i2c_address, 800, 0);      //program ends right away and servo ramps towards new defined position 
	//I2CServo_Stop(i2c_address, 0);                        //program and motion end immediately on current position without any ramp
	//MyDelay(1000);

  
	//----------------------------------------
	//EXAMPLE #11
	//Save all registers. 
	//Current motion settings become new defaults, loaded on power up.
	//If PROGRAM_START register is set to option 2, the stored program motion will start on power up.
	//I2CServo_SaveAll(i2c_address, 0);
	//MyDelay(5000);


	//----------------------------------------
	//EXAMPLE #12
	//Read and write with crc-8 checksum. 
	//Smbus-type crc8 (0x07 polynomial) on the i2c packets adds robustness and safety to implementation.
	//The I2C wrapper implementation in this project acommodates for this, making things much simpler.
	//Crc8 success can be checked with the I2CWrapper_WasCrcOk() function, and if failed, you can decide to retry or do something else.
	/*
	//Read
	byte power;
	i2cServo_GetCurrentPosition(i2c_address, &power, 1);
	if(I2CWrapper_WasCrcOk()==1){  Serial.print("Crc Ok\n"); }
	else{ Serial.print("Crc  failed\n"); }
	MyDelay(1000);
  
	//Write, if checksum fails, servo discards the whole write transaction.
	I2CServo_SetMaxPower(i2c_address, 75, 1); 
	//
	//The servo stores the last succesful write crc, so you can consult that to double check.
	byte last_crc;
	i2cServo_GetLastcrc8(i2c_address, &last_crc, 1);//Note that this itself is done using crc for integrity. 
	Serial.print(last_crc);
	//MyDelay(1000);
	*/
      
  }//loop
  
  //Don't forget to park and turn servo(s) off on program exit 
  //for(i=0;i<5;i++){ I2CServo_Stop(servo_ids[i], 0); }
  //for(i=0;i<5;i++){ I2CServo_Setup(servo_ids[i], 1, 1, 0, 0, 0, 0, 0, 0); }
  
  MyuSleep(5000);
  I2C_close();
  return(0);
}


void MyDelay(uint32_t ms){
 MyuSleep(ms * 1000);
}

void MyuSleep(uint32_t us){
 
 usleep(us); 
 //for usleep over 100ms on Raspbian, need some additional time to wake up before next i2c read. Otherwise i2c timing/data is incorrect.
 usleep(50); 
 //for(i=0;i<200000;i++){ i=i; } 	
}



 



