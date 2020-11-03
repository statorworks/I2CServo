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
//#include "i2c_ardu.h"
#include "i2c_raspy.h"  //***TODO: implement crc functionality



int i2c_file;
uint8_t I2C_LastCrc;
bool I2C_crc_ok;
bool I2C_initialized;

struct i2c_msg i2c_msgs[2];
struct i2c_rdwr_ioctl_data i2c_msgset;
uint8_t i2c_obuff[10];
uint8_t i2c_ibuff[10];


bool I2C_IsInit(void){ return I2C_initialized; }

bool I2C_WasCrcOk(void){ return I2C_crc_ok; }

uint8_t I2C_GetLastCrc(void){ return I2C_LastCrc; }

bool I2C_init(){

  I2C_initialized=0;
  
  char filename[] = "/dev/i2c-1";  //i2c-1 on Rpi Zero   
  i2c_file = open(filename, O_RDWR);
  if(i2c_file<0){I2C_DEBUG_PRINT("Could not init i2c\n");  return 0;}
  else{ I2C_DEBUG_PRINT("i2c initialized Ok\n"); }

  I2C_initialized=1;  
 
  return 1;	
}

bool I2C_close(){

if(i2c_file==3){ close(i2c_file); }
return 1;
}

bool I2C_Read(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len, bool use_crc){
 
  if(address>0x7F){return 0;}
  if(data==0){return 0;}
  if(len==0){return 0;}
  if(len>64){return 0;}
  
  //read (combined i2c write-read type so we can access any register)
  i2c_msgs[0].addr  = address;
  i2c_msgs[0].flags = 0; 
  i2c_msgs[0].len   = 1;
  i2c_msgs[0].buf   = i2c_obuff;  
  i2c_obuff[0]      = reg; //first reg to read
  //
  i2c_msgs[1].addr  = address; //restart condition with address again
  i2c_msgs[1].flags = I2C_M_RD | I2C_M_STOP; 
  i2c_msgs[1].len   = 2;    //read two 
  i2c_msgs[1].buf   = data;    
  //
  i2c_msgset.msgs   = i2c_msgs;
  i2c_msgset.nmsgs  = 2;
  //
  ioctl(i2c_file, I2C_RDWR, &i2c_msgset);
  
  return 1;
}

bool I2C_Write(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len, bool use_crc){

  uint16_t i;
  
  if(address>0x7F){return 0;}
  if(data==0){return 0;}
  if(len==0){return 0;}
  if(len>64){return 0;}

  //Old Way
  //ioctl(file, I2C_SLAVE, 0x79); //setup address
  /*
  buff[0]=0x0D; //reg
  buff[1]=0x02;
  buff[2]=0x50;   
  write(file, buff, 3);  //must return 3
  printf("sent\n");
  usleep(100000);
  //
  buff[0]=0x0D; //reg
  buff[1]=0x02;
  buff[2]=0x80; 
  write(file, buff, 3);
  printf("sent\n");
  usleep(100000);
  */
  
  //New Way
  i2c_msgs[0].addr  = address;
  i2c_msgs[0].flags = 0; 
  i2c_msgs[0].len   = 1+len;   //reg+data
  i2c_msgs[0].buf   = i2c_obuff;
  i2c_obuff[0]      = reg;     //first reg to write    
  
  for(i=0;i<len;i++){
   i2c_obuff[1+i] = data[i];
  }
  
  i2c_msgset.msgs   = i2c_msgs;
  i2c_msgset.nmsgs  = 1;
  ioctl(i2c_file, I2C_RDWR, &i2c_msgset);
  return 1;
}

bool  I2C_FreqSet(uint32_t f){

  //check 
  if(f<I2C_MIN_FREQ){ f=I2C_MIN_FREQ;} 
  if(f>I2C_MAX_FREQ){ f=I2C_MAX_FREQ; }
  
  //Acces BCM2835 i2c register directly
  int  mem_fd = 0;
  if((mem_fd = open ("/dev/mem", O_RDWR)) < 0)
  { I2C_DEBUG_PRINT("could not change i2c freq\n"); }
  //  
  if ((registers = (uint32_t *) mmap (NULL, BSC_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, BSC1_BASE)) == MAP_FAILED)
  { I2C_DEBUG_PRINT("could not change i2c freq\n"); }
  
  //Even though DIV is a 32-bit register, the MS 16 bits should be zero, 
  //so the divider ranges from 65535 down to 0 (which is interpreted as 32,768).  
  //The range of frequencies should then be CORE_CLK / 1 down to CORE_CLK / 65535
  uint32_t  new_div = CORE_CLK / f;
  DIV = new_div;  //write register
  I2C_DEBUG_PRINT("f: %u  DIV: %08x\n", f, new_div);
  
  //close memory access	
  if(close (mem_fd) < 0)
  { I2C_DEBUG_PRINT("i2cfreq_set could not close memory access\n"); }
  if(munmap (registers, BSC_SIZE) < 0)
  { I2C_DEBUG_PRINT("i2cfreq_set could not close memory access\n"); }
 
 return 1;
 // 
}




