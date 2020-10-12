//
//Statorworks 2020
//Simple wrappers for Arduino I2C. 
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
//Select the library for your board with Sketch->Include library
//Note that Wire and TinyWireM Arduino libraries block execution when there
//is no response if the bus is disconnected. Those libraries need to be fixed in the future.
//
//This wrapper also supports an optional SMbus type crc8 checksum (polynomial 0x07) for the i2c packet.
//With the crc option, the packet is automatically formatted in this fashion: address, (0x80|reg), datalength, data, crc8
//The crc calculation includes all these bytes except the crc spot itself.


//Select your board here:
#define I2CWRAPPER_ARDUINO_NANO_WIRE 1
//#define I2CWRAPPER_DIGISPARK_TINYWIREM 1



bool I2CWrapper_initialized;
bool I2CWrapper_IsInit(void){ return I2CWrapper_initialized; }
bool I2CWrapper_crc_ok;
bool I2CWrapper_WasCrcOk(void){ return I2CWrapper_crc_ok; }
byte I2CWrapper_LastCrc;
bool I2CWrapper_GetLastCrc(void){ return I2CWrapper_LastCrc; }


/////////////////////////////////////////////////////////////////////
#ifdef I2CWRAPPER_ARDUINO_NANO_WIRE

#include <Wire.h>     //Nano etc

void I2CWrapper_Begin(void){
  Wire.begin();
  I2CWrapper_initialized=1;
}

bool I2CWrapper_Write(byte address, byte reg, byte* data, byte len, bool use_crc){

 //check
 if(I2CWrapper_initialized==0){return 0;}
 if(data==0){return 0;}
 if(len==0){return 0;}
 
 Wire.beginTransmission(address);
 if(use_crc ==0){ 
  Wire.write(reg);
 }
 else{//for crc scheme we need to set msb on reg, and add data length byte 
  Wire.write(reg|0x80);
  Wire.write(len); 
 }

 
 for(byte i=0; i<len; i++){ 
   Wire.write(data[i]); //fill
 }

 //and crc byte if needed
 I2CWrapper_crc_ok =1;
 if(use_crc==1){
  byte crc_calc = I2CWrapper_crc_calculate(address, (reg|0x80), len, data);
  Wire.write(crc_calc); 
  I2CWrapper_LastCrc = crc_calc;
 }


 //send
 if(Wire.endTransmission(true)!=0){return 0;};
 
 return 1;//Ok
}


bool I2CWrapper_Read(byte address, byte reg, byte* data, byte len, bool use_crc){
 //Standard i2c combined-read

 //check
 if(I2CWrapper_initialized==0){return 0;} 
 if(data==0){return 0;}
 if(len==0){return 0;}
 
 //Write access reg
 Wire.beginTransmission(address);
 if(use_crc ==0){ 
  Wire.write(reg);
 }
 else{//for crc scheme we need to set msb on reg, and add data length byte 
  Wire.write(reg|0x80);
  Wire.write(len);  
 }
 //send
 if(Wire.endTransmission(false)!=0){return 0;}; //send, no stop bit. return zero on failure.

 //Read the data
 byte nbytes = len;
 if(use_crc==1){ nbytes++; }//for crc option we'll actually read an extra byte 
 Wire.requestFrom(address, nbytes);
 // 
 if(Wire.available()==0){return 0;} //no response
 for(byte i=0; i<len; i++){ 
   data[i] = Wire.read();
  }

 //and crc byte if needed
 I2CWrapper_crc_ok =1;
 if(use_crc==1){
  byte crc_recv = Wire.read();
  //
  byte crc_calc = I2CWrapper_crc_calculate(address, (reg|0x80), len, data);
  I2CWrapper_LastCrc = crc_calc;
  if(crc_recv != crc_calc){ I2CWrapper_crc_ok =0; return 0; }
 }

 return 1;//Ok
}

#endif


//////////////////////////////////////////////////////////////////
#ifdef I2CWRAPPER_DIGISPARK_TINYWIREM

#include <TinyWireM.h> //i2c Attiny85 Digispark
#include "USI_TWI_Master.h"

void I2CWrapper_Begin(void){
  TinyWireM.begin();
  I2CWrapper_initialized=1;
}

bool I2CWrapper_Write(byte address, byte reg, byte* data, byte len, bool use_crc){
 
 //check
 if(I2CWrapper_initialized==0){return 0;}
 if(data==0){return 0;}
 if(len==0){return 0;}
 
 //if(TinyWireM.beginTransmission(address)!=0){return 0;}
 TinyWireM.beginTransmission(address);
 
 if(use_crc ==0){ 
  TinyWireM.send(reg);
 }
 else{//for crc scheme we need to set msb on reg, and add data length byte 
  TinyWireM.send(reg|0x80);
  TinyWireM.send(len); 
 }
  
 for(byte i=0; i<len; i++){ 
   TinyWireM.send(data[i]); //fill
 }

 //and crc byte if needed
 I2CWrapper_crc_ok =1;
 if(use_crc==1){
  byte crc_calc = I2CWrapper_crc_calculate(address, (reg|0x80), len, data);
  TinyWireM.send(crc_calc); 
  I2CWrapper_LastCrc = crc_calc;
 }
   
 //if(TinyWireM.endTransmission(1)!=0){USI_TWI_Master_Stop(); return 0;} //send data and stop bit
 TinyWireM.endTransmission(true); //<has bug, doesn't send stop bit
 USI_TWI_Master_Stop();//<do it manually
 
 return 1;//Ok
}


bool I2CWrapper_Read(byte address, byte reg, byte* data, byte len, bool use_crc){
 //Standard i2c combined-read

 //check
 if(I2CWrapper_initialized==0){return 0;} 
 if(data==0){return 0;}
 if(len==0){return 0;}
 
 //Write access reg
 //if(TinyWireM.beginTransmission(address)!=0){return 0;}
 TinyWireM.beginTransmission(address);
 if(use_crc ==0){ 
  TinyWireM.send(reg);
 }
 else{//for crc scheme we need to set msb on reg, and add data length byte 
  TinyWireM.send(reg|0x80);
  TinyWireM.send(len);  
 }
  
 TinyWireM.endTransmission(false); //send, no stop bit, followed by restart condition

 //Read the data
 TinyWireM.requestFrom(address, len);
 for(byte i=0; i<len; i++){ 
   data[i] = TinyWireM.receive();
   //if(TinyWireM.available()==0){break;}
   TinyWireM.available();
 }

 //and crc byte if needed
 I2CWrapper_crc_ok =1;
 if(use_crc==1){
  byte crc_recv = TinyWireM.receive();
  USI_TWI_Master_Stop();
  //
  byte crc_calc = I2CWrapper_crc_calculate(address, (reg|0x80), len, data);
  I2CWrapper_LastCrc = crc_calc;
  if(crc_recv != crc_calc){ I2CWrapper_crc_ok =0; return 0; }
 }
 else{ 
   //stop bit/nack has to be manual  
   USI_TWI_Master_Stop();
 }

 return 1;//Ok
}

#endif


byte I2CWrapper_crc_calculate(byte address, byte reg, byte len, byte* data){
  
  byte crc8=0;
  I2CWrapper_crc8byte(&crc8, address);
  I2CWrapper_crc8byte(&crc8, reg);
  I2CWrapper_crc8byte(&crc8, len); 
  for(byte i = 0; i<len; i++){ 
   I2CWrapper_crc8byte(&crc8, data[i]);
  }
  return crc8;
}


void I2CWrapper_crc8byte(uint8_t* crc_val, uint8_t val){
//thanks to:
//mouser.com/pdfDocs/SFM4100_CRC_Checksum_Calculation.pdf
//stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
//reveng.sourceforge.net/crc-catalogue/all.htm

 if(crc_val==0){return;}

 uint16_t POLYNOMIAL = 0x07; //SMbus type
 
 //calculates 8-Bit checksum with given polynomial
 *crc_val ^= val;
 //for (uint8_t b = 8; b > 0; --b)
 for (uint8_t b = 0; b <8; b++) 
 { 
   if((*crc_val & 0x80)!=0) {*crc_val = (*crc_val << 1) ^ POLYNOMIAL;}
   else {*crc_val = (*crc_val << 1);}
 }
}

