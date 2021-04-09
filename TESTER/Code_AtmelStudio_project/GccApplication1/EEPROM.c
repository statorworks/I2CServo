//
//EEPROM.h
//NVMCTRL ATTINY416
//Based on samd51 driver
//

#include "EEPROM.h"


bool EEPROM_Write(uint16_t position, uint8_t* data, uint16_t length){

  //check
  if(EEPROM_START  + position + length > EEPROM_END){ return KO; }
  if(data==0){return KO;}
  if(length==0){return KO;}

  //confirm eeprom ready
  if((NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm) !=0){ return KO; }
	  	  	
  DISABLE_INTERRUPTS; 


  //clear page buffer
  protected_write_io((void *)&NVMCTRL.CTRLA, CCP_SPM_gc, NVMCTRL_CMD_PAGEBUFCLR_gc);
  
  //CC 6-20 wait finished but have a way out
  volatile uint16_t t;
  for(t=0;t<1000;t++){ if((NVMCTRL.STATUS&NVMCTRL_EEBUSY_bm)==0){break;} } //takes 0x00 counts @10mhz

  for(uint8_t i=0; i<length; i++){

	//write page buffer. Attiny416 has byte-granularity on eeprom. Seems each byte takes a 'page write'
	*(uint8_t *)(EEPROM_START + position +i) = data[i];  
    
	//Actual byte copy to eeprom. command requires <4-cycles after key. Special assembly function from Atmel Start example.
	protected_write_io((void *)&NVMCTRL.CTRLA, CCP_SPM_gc, NVMCTRL_CMD_PAGEERASEWRITE_gc);  
	
    //CC 6-20 wait finished but have a way out
	for(t=0;t<1000;t++){ if((NVMCTRL.STATUS&NVMCTRL_EEBUSY_bm)==0){break;} }  
	//takes about 0xc8 counts @10mhz. which doesn't make sense since the datasheet says 4ms eeprom write.
	//there's no other flag to check so it seems to be done in the background.
  }
	  
  ENABLE_INTERRUPTS;
  
  return OK;
}


bool EEPROM_Read(uint16_t position, uint8_t* data, uint16_t data_length){
  
  //check
  if(EEPROM_START  + position + data_length > EEPROM_END){ return KO; }
  if(data==0){return KO;}
  if(data_length==0){return KO;}
	
  //read is done normally from the address. no eeprom command necessary
  for(uint8_t i=0; i<data_length; i++)
  {  data[i] = *(uint8_t*)(EEPROM_START + position +i);  }	

  return OK;
}

