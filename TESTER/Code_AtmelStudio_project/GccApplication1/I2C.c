//
//I2C master or slave
//ATTINY416
//Created: 5/14/2019 12:48:46 PM
//Author: Camilo

#include "I2C.h"

bool i2c_Init(uint8_t i2c_n, bool master, uint8_t addr, uint32_t freq_hz){

 //Slave
 #if I2C_MASTER == 0
 if(master==0){
	
	TWI0.CTRLA   = 0;				//no sda hold, no sda setup. Normal and high speed default (up to 400Khz)
	TWI0.DBGCTRL = 1;				//keep running on debug
	TWI0.MCTRLA  = 0;				//master disabled
	//slave disabled before configuring, data interrupt, address interrupt, stop interrupt, use slave address, smart mode disabled.
	TWI0.SCTRLA = TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm;  
	TWI0.SADDR   = (addr<<1);		//7-bit address. shifted to match actual transmit format
	TWI0.SADDRMASK = 0x00;			//no mask, no secondary address. see datasheet page 324

    //CC 6 20 clear all interrupt flags. 
	//in case of a bus lock reset we need to not be stuck with an interrupt flag. 
    TWI0.SSTATUS = 0xFF;
	//
	TWI0.SCTRLA |= TWI_ENABLE_bm;		
	//command "Wait for Start Condition" ensure lines are released:
	TWI0.SCTRLB = (0<<TWI_ACKACT_bp) | (0x02<<TWI_SCMD_gp);	
 }
 #endif

 //Master
 #if I2C_MASTER == 1  //save on program memory
 if(master==1){
		
	TWI0.CTRLA   = 0;				//no sda hold, no sda setup. Normal and high speed default (up to 400Khz)
    //TWI0.CTRLA = TWI_SDAHOLD_500NS_gc | TWI_SDASETUP_8CYC_gc;
	TWI0.DBGCTRL = 1;				//keep running on debug
	TWI0.SCTRLA  = 0;				//slave disabled
	//master disabled for configuration, read int enable, write int enable, quick command disabled, timeout disabled, smart mode disabled.
	TWI0.MCTRLA  = TWI_RIEN_bm | TWI_WIEN_bm;
	TWI0.MCTRLB |= TWI_ACKACT_bm ;	//send ack when appropriate. No commands at this point.
	//TWI0.MADDR   = 0;				//as per errata don't write addres register until 2-clocks after enabling twi.
	
	////FastMode+
	if(freq_hz > 400000){ TWI0.CTRLA  |= 2; }
	
	//Baud: ((fclk/fscl)-(fclk*Trise)-10)/4  -> ((10M/100K)-(10M*0.1uS)-10)4 = 40. See datasheet
	if(freq_hz==0){ return 0;}
	//
	i2c_freq = freq_hz;
	uint16_t baud = (((uint32_t)I2C_CLOCK_FREQ/freq_hz)-((I2C_CLOCK_FREQ * 1)/1000000) -10  )/4; //as per datasheet
	if(baud<3)  { baud =3; }
	if(baud>255){ baud =255; }
	TWI0.MBAUD = baud;
	
	//Force bus state idle
	TWI0.MSTATUS = (TWI0.MSTATUS&0xFC)|I2C_MASTER_BUS_STATES_IDLE;
	
	//clear lines
	TWI0.MCTRLB |= TWI_FLUSH_bm;
	
	//todo: toggle clock some 16 times to allow slaves reset any previous state
	//...
	
	//enable
	TWI0.MCTRLA |= TWI_ENABLE_bm;
 }
  
 #endif	

	//initial state
	i2c_transfer_address = addr; //CC 6-20 	
	i2c_transaction_state = I2C_TRANSACTION_IDLE;
	i2c_initialized =1;
	i2c_busy_time=0;
		
 return 1;	
}

void i2c_Check(uint8_t lapse_ms){
//CC 6-20
//call this every 1ms to monitor bus state and reset on any lockout condition

 if(i2c_transaction_state!=I2C_TRANSACTION_IDLE)
 { i2c_busy_time += lapse_ms; }	 
 else{ i2c_busy_time = 0;}

 //Reset i2c if stuck for more than a few ms.
 if(i2c_busy_time>I2C_BUS_LOCKUP_TIMEOUT_MS){
   
   //Slave
   #if I2C_MASTER == 0 
   	//Force send ack condition (SDA up release bus). Up to master to recover at this point.
    TWI0.SCTRLB = (0<<TWI_ACKACT_bp) | (0x03<<TWI_SCMD_gp); 
	i2c_Init(0, 0, i2c_transfer_address, 0);
   
   //Master
   #else 
    i2c_Init(0, 1 , i2c_transfer_address, i2c_freq);
   #endif
  }//
//  
}

//I2C Slave interrupt handler
ISR(TWI0_TWIS_vect){

	#if I2C_MASTER == 0  //less code space usage

	uint8_t i, val;
	
	//Check state
	if((TWI0.SCTRLA & TWI_ENABLE_bm)==0){return;}
	
	//Check bus error case
	if((TWI0.SSTATUS & TWI_BUSERR_bm) ==TWI_BUSERR_bm){
		
		//clear
		TWI0.SSTATUS |= TWI_BUSERR_bm;
		
		//cancel anything going on
		TWI0.SCTRLB |= (0x02<<TWI_SCMD_gp);  //command "Wait for Start Condition"
		i2c_transaction_state = I2C_TRANSACTION_IDLE;
		return;
	}

	//Collision
	if((TWI0.SSTATUS & TWI_COLL_bm) == TWI_COLL_bm){
		
		TWI0.SSTATUS |= TWI_COLL_bm;
		//cancel anything going on
		TWI0.SCTRLB |= (0x02<<TWI_SCMD_gp);  //command "Wait for Start Condition"
		i2c_transaction_state = I2C_TRANSACTION_IDLE;
		return;	
	}
	
	//Address received
	//Address/stop interrupt + address/stop status bit set
	//This check is before the state check so the master can always reset the transfer with the start+address condition. this way we're never stuck. 
	if(((TWI0.SSTATUS & TWI_APIF_bm) ==TWI_APIF_bm)&&((TWI0.SSTATUS & TWI_AP_bm) ==TWI_AP_bm)){
	
		//State Idle, expect I2c address match
		//Don't check this state. allow start+address detection to always reset the state 
		//if(i2c_transaction_state == I2C_TRANSACTION_IDLE){
				
			//clear
			TWI0.SSTATUS |= TWI_APIF_bm;

			//Master write to slave operation
			if((TWI0.SSTATUS & TWI_DIR_bm) != TWI_DIR_bm){ 
				
				i2c_transaction_state = I2C_TRANSACTION_WRITE;
				
				//transaction begins
				i2c_transfer_step  = 0;
				i2c_transfer_count = 0;
				i2c_transfer_done  = 0;			
			    i2c_use_crc=0;

			}
			
			//Simple immediate read
			//not supported, does not include register write. Apparently used with single-register devices.
			
			//Combined master read from slave operation. happens after write of register value.
			else{ 
				i2c_transaction_state = I2C_TRANSACTION_READ_COMBINED; 
			}

			//ack the address. This triggers the next step
			TWI0.SCTRLB = (0<<TWI_ACKACT_bp) | (0x03<<TWI_SCMD_gp);
	
	}//address


	//Write by master (SaddressW, reg, data)
	else if(i2c_transaction_state == I2C_TRANSACTION_WRITE){
		
		//byte received
		if((TWI0.SSTATUS & TWI_DIF_bm) ==TWI_DIF_bm){
			
			//clear
			TWI0.SSTATUS |= TWI_DIF_bm;
			
			//first byte is register offset for transaction.
			if(i2c_transfer_step==0){
				
				val = TWI0.SDATA;  //read and send ack
				i2c_transfer_register = val &0x7F;
				
				//let msb of register offset byte indicate if crc is applicable
				if((val &0x80)==0x80){ 
				   i2c_use_crc =1; 
				   i2c_crc_calc= 0;
				   crc8byte(&i2c_crc_calc, i2c_transfer_address);
				   crc8byte(&i2c_crc_calc, val); //computation includes the 0x80 bit	
				}
				else{ i2c_use_crc=0; }
				
				i2c_transfer_step=1;
				
				//*if this is part of a combined read, then a repeated start condition with read follows.
			}
			//if crc applies there should be a second byte with the length of the data transfer
			else if((i2c_transfer_step==1)&&(i2c_use_crc==1)){
				
			  i2c_transfer_length = TWI0.SDATA;
			  crc8byte(&i2c_crc_calc, i2c_transfer_length);	

			  //*if this is part of a combined read, then a repeated start condition with read follows.			
			  i2c_transfer_step=2;
			}
			//continue master write, get bytes from master
			//Actual register write left to application as checks may be necessary.
			else{
				
				val = TWI0.SDATA;
				bool do_store_byte=1;
				
				//if crc applies add byte to the computation. skip the last actual crc byte
				if(i2c_use_crc==1){  
					
					if((i2c_transfer_count)<i2c_transfer_length)
					{ crc8byte(&i2c_crc_calc, val); }				
				    else
					{ i2c_crc_value = val; do_store_byte=0; }//crc byte is not saved with the payload data
				}				
				
				//store								
				if(do_store_byte==1){
									
					//constrain bounds
					if(i2c_transfer_in_data==0){}
					else if(i2c_transfer_count > I2C_MAX_TRANSFER_BYTES){}	
					else{  
					  i2c_transfer_in_data[i2c_transfer_count] = val; 
					  i2c_transfer_count++;
					}	
				}				
		    }//step
			
			//**No need to send ack here [?], it seems done automatically.
			//TWI0.SCTRLB = (0<<TWI_ACKACT_bp) | (0x03<<TWI_SCMD_gp); //send ack and wait for next byte			
		}//byte
		
		//Stop bit from master, finalize write
		//Address/stop interrupt + addres/stop status bit clear
		else if(((TWI0.SSTATUS & TWI_APIF_bm) ==TWI_APIF_bm)&&((TWI0.SSTATUS & TWI_AP_bm) != TWI_AP_bm)){
			
			//complete transaction, release bus
			TWI0.SCTRLB = (0<<TWI_ACKACT_bp) | (0x02<<TWI_SCMD_gp);	//command "Wait for Start Condition"

			//done
			i2c_transfer_done=1;
						
			//determine if crc ok			
			if(i2c_use_crc==1){
			 i2c_crc_ok=0;	
			 if(i2c_transfer_count>0){//avoid underflow
			   if(i2c_crc_value == i2c_crc_calc){ i2c_crc_ok=1; }
			  }
			}
						
			//invoke user callback
			if(i2c_receive_complete_callback!=0)
			  if((i2c_use_crc==0) || ((i2c_use_crc==1)&&(i2c_crc_ok==1))){
  			  { i2c_receive_complete_callback(); }			
		    }
			
			//done
			i2c_transaction_state = I2C_TRANSACTION_IDLE;
		}
		else{}
	}//write
	
	
	//Simple read
	//not supported in this implementation
	
	
	//Combined read by master  (SaddressW,reg,SaddressR,data)
	else if(i2c_transaction_state == I2C_TRANSACTION_READ_COMBINED){
		
		uint8_t dummy = TWI0.SSTATUS;
			
		//byte sent successfully
		if((TWI0.SSTATUS & TWI_DIF_bm) ==TWI_DIF_bm){

		  //clear
		  //NOT needed! this wrecks the receive TWI0.SSTATUS |= TWI_DIF_bm;
					  
		  //ack from master
		  //No need to check TWI_RXACK flag
						
		  //send bytes to master. register has already been taken on first part, now send data
		  if(i2c_transfer_step >=1){//confirm step
			   	
				//if crc applies, the last byte sent to master is the crc
			    if((i2c_use_crc==1)&&((i2c_transfer_count)==i2c_transfer_length))
			    {  
				  val = i2c_crc_calc;	
				}				
			    else{//byte

				  //constrain bounds
				  if(i2c_transfer_out_data==0){val =0;}
				  if((i2c_transfer_register + i2c_transfer_count) >= I2C_MAX_REG_COUNT){val=0;}
				  else{ val = i2c_transfer_out_data[ i2c_transfer_register + i2c_transfer_count]; }
					  				  
				  //if crc applies, add byte to the calculation
				  if(i2c_use_crc==1)
				  { crc8byte(&i2c_crc_calc, val); }

				  i2c_transfer_count++;
				}
				
				//actual send
				TWI0.SDATA = val; 
				TWI0.SCTRLB = (1<<TWI_ACKACT_bp)|(0x03<<TWI_SCMD_gp);  //send. required	
   			
			   }//step
	
		  //Nack from master. master done reading
		  //No need to check TWI_RXACK flag

		}//byte

		//Stop bit from master, finalize Read at any time
		//stop bit interrupt + address/stop status bit clear
		else if(((TWI0.SSTATUS & TWI_APIF_bm) ==TWI_APIF_bm)&&((TWI0.SSTATUS & TWI_AP_bm) != TWI_AP_bm)){
			
			//clear
			TWI0.SSTATUS |= TWI_APIF_bm;
			
			//complete transaction, release bus
			TWI0.SCTRLB = (0x02<<TWI_SCMD_gp);				//command "Wait for Start Condition"
			i2c_transaction_state = I2C_TRANSACTION_IDLE;	//done
			i2c_transfer_done=1;  //signal application
	  }
	}

	//If this point is reached in idle, all flags must be clear
	//No unhandled flags should be pending. this guards against external spurious stop conditions etc 
	else if(i2c_transaction_state==I2C_TRANSACTION_IDLE){
	  TWI0.SSTATUS |= 0xFF;
	}

 #endif
}

void I2C_SlaveSetAddress(uint8_t address){
  TWI0.SADDR = (address<<1);
  i2c_transfer_address = address;
}

//I2C Master interrupt handler
ISR(TWI0_TWIM_vect){
	
	#if I2C_MASTER == 1 //less code space usage

	uint16_t i;
	uint8_t dummy_read;
	
	//Check state
	if((TWI0.MCTRLA & TWI_ENABLE_bm)==0){return;}
	if(i2c_transaction_state == I2C_TRANSACTION_IDLE) {return;} 
	 
	//Check bus error case
	if((TWI0.MSTATUS & TWI_BUSERR_bm) ==TWI_BUSERR_bm){
		
		//clear
		TWI0.MSTATUS |= TWI_BUSERR_bm;
		
		//cancel anything going on
		TWI0.MCTRLB |= (0x03<<TWI_SCMD_gp);  //command "Send stop condition"
		i2c_transaction_state = I2C_TRANSACTION_IDLE;
		i2c_transfer_failed = 1;
		return;
	}

	//Arbitration lost (another master gained bus) user must start over
	else if((TWI0.MSTATUS & TWI_ARBLOST_bm) ==TWI_ARBLOST_bm){// (i2c->STATUS.bit.ARBLOST==1){
		
		//clear
		TWI0.MSTATUS |= TWI_ARBLOST_bm;
		//
		TWI0.MCTRLB |= (0x03<<TWI_SCMD_gp);  //command "Send stop condition"
		i2c_transaction_state = I2C_TRANSACTION_IDLE;
		i2c_transfer_failed = 1;
		return;
	}	

	//NACK + MB "master on bus" = no response. no device present etc. user must start over
	else if((TWI0.MSTATUS & TWI_RXACK_bm) ==TWI_RXACK_bm){//*NACK is high
		
		//send stop condition (recommended)
		TWI0.MCTRLB = (0x03<<TWI_SCMD_gp);  //command "Send stop condition"
		//
		i2c_transaction_state = I2C_TRANSACTION_IDLE;
		i2c_transfer_failed = 1;
		return;
	}	


 	//Write (addressW, reg, data)
 	else if(i2c_transaction_state == I2C_TRANSACTION_WRITE){

	    //Byte sent + Ack received
	    if(((TWI0.MSTATUS & TWI_WIF_bm) ==TWI_WIF_bm)&&((TWI0.MSTATUS & TWI_RXACK_bm) !=TWI_RXACK_bm)){//*ACK is low

			//address just sent
			if(i2c_transfer_step==0){		
				
				//send slave target register.
				TWI0.MDATA = i2c_transfer_register;
				i2c_transfer_step=1;
				return;			
			}
 			//send bytes
			else if(i2c_transfer_step==1){
	 	  
	 			//send next
	 			if(i2c_transfer_count<i2c_transfer_length){
		 	  
		 			TWI0.MDATA = i2c_transfer_out_data[i2c_transfer_count];
		 			i2c_transfer_count++;
	 			}
				//done
	 			else{
		 	  
		 			//send stop
		 			TWI0.MCTRLB |= (0x03<<TWI_SCMD_gp);  //command "Send stop condition"
		 			//
		 			i2c_transfer_done=1;
		 			i2c_transaction_state = I2C_TRANSACTION_IDLE;
	 			}
			}
 	    }//ack			
	
	}//write
	 
	
	//Read simple (addressR, reg, data)
	//Not supported in this implementation
	
	
	//Read combined (addressW, reg, addressR, data)
	else if(i2c_transaction_state == I2C_TRANSACTION_READ_COMBINED){
	    
		//Byte sent + Ack received
		if(((TWI0.MSTATUS & TWI_WIF_bm) ==TWI_WIF_bm)&&((TWI0.MSTATUS & TWI_RXACK_bm) !=TWI_RXACK_bm)){//*ACK is low
					
			//address just sent
			if(i2c_transfer_step==0){

				//send slave target register
				TWI0.MDATA = i2c_transfer_register;
				i2c_transfer_step=1;
				return;
			}
			//change to read		
			else if(i2c_transfer_step==1){

				//send stop
				//TWI0.MCTRLB = (1<<TWI_ACKACT_bp) | 0x03; //NACK +stop				
				//**As per I2c SPEC, stop not needed, the ads1114 device doc mistakenly states it is required, but not true.
									
				//send address again with read bit	
				TWI0.MADDR = (i2c_transfer_address<<1) | 0x01;
				i2c_transfer_step=2;
				return;			
			}
			else{}
		}//ack
		
		//Get bytes
		else if(((TWI0.MSTATUS & TWI_RIF_bm) ==TWI_RIF_bm)){	
			
			if(i2c_transfer_step==2){
			
			  //track
			  i2c_transfer_count++;
	
			  //save
			  if(i2c_transfer_count<i2c_transfer_length){
				  
				//read and trigger next
				i2c_transfer_in_data[i2c_transfer_count-1] = TWI0.MDATA;    //this is not sending ack action automatically
				TWI0.MCTRLB = (0<<TWI_ACKACT_bp) | 0x02; //required manual ACK + read
				return;
			  }
			  //done
			  else{
				//save last byte received triggering NACK and stop
				i2c_transfer_in_data[i2c_transfer_count-1] = TWI0.MDATA;  //this is not sending nack action automatically
				TWI0.MCTRLB = (1<<TWI_ACKACT_bp) | 0x03; //required manual NACK + stop
				//
				i2c_transfer_done=1;
				i2c_transaction_state = I2C_TRANSACTION_IDLE;
			  }			
			}
		}//rx
						
	}//read combined
	
 #endif
}

bool I2C_MasterWrite(uint8_t i2c_n, uint8_t address, uint8_t register_offset, uint8_t* data, uint8_t length){

#if I2C_MASTER == 1 //less code space usage
	if((TWI0.MCTRLA & TWI_ENABLE_bm)==0){return 0;}
	if(data==0){return 0;}
	if(length==0){return 0;}

	i2c_transfer_address = address;
	i2c_transfer_register = register_offset;
	i2c_transfer_out_data   = data;
	i2c_transfer_length = length;
	i2c_transfer_count  = 0;
	i2c_transfer_step	 = 0;
	i2c_transfer_done	 = 0;
	i2c_transfer_failed = 0;
	i2c_transaction_state = I2C_TRANSACTION_WRITE;
	
	//calculate crc if applicable
	//if(i2c_use_crc==1){
	//crc=..
	//i2c_transfer_register_offset |=0x80;  //msb tell compliant slave to use crc
	//}
	
	//send address to begin transmission
	TWI0.MADDR = (address<<1);  //bit0 =0 =Write
	
	//continued on interrupt routine. wait result here
	uint32_t timeout=0;
	while(i2c_transfer_done==0 && i2c_transfer_failed==0)
	{ if(timeout++ > 1000) { 
		return 0; } 
		}//say about 5ms

#endif	
	return 1;
}

bool I2C_MasterRead(uint8_t i2c_n, uint8_t address, uint8_t register_offset, uint8_t* data, uint8_t length){

#if I2C_MASTER == 1
	if((TWI0.MCTRLA & TWI_ENABLE_bm)==0){return 0;}
	if(data==0){return 0;}
	if(length==0){return 0;}
	
	i2c_transfer_address = address;
	i2c_transfer_register = register_offset;
	i2c_transfer_in_data   = data;
	i2c_transfer_length = length;
	i2c_transfer_count  = 0;
	i2c_transfer_step	= 0;
	i2c_transfer_done	= 0;
	i2c_transfer_failed = 0;
	
	//read type:
	i2c_transaction_state = I2C_TRANSACTION_READ_COMBINED;//addressW, reg, addressR, data
		
	//send address to begin transmission
	TWI0.MADDR = (address<<1);  //bit0 =0 =Write to begin with
	
	//continued on interrupt routine. wait result here
	uint32_t timeout=0;
	while(i2c_transfer_done==0 && i2c_transfer_failed==0)
	{ if(timeout++ > 1000) { return 0; } }//say about 5ms

#endif	
	return 1;
}

uint8_t crc8(uint8_t* data, uint8_t length){
//*perfomance measured on attiny416: 2.4uS for eight bytes.

 uint8_t crc = 0;
 for(uint8_t i = 0; i<length; i++){ 
  crc8byte(&crc, data[i]);
 }
 return crc;
}

void crc8byte(uint8_t* crc_val, uint8_t val){
//thanks to:
//www.mouser.com/pdfDocs/SFM4100_CRC_Checksum_Calculation.pdf
//stackoverflow.com/questions/51752284/how-to-calculate-crc8-in-c
//reveng.sourceforge.net/crc-catalogue/all.htm

 if(crc_val==0){return;}

 uint16_t POLYNOMIAL = 0x07; //SMbus type
	
 //calculates 8-Bit checksum with given polynomial
 *crc_val ^= val;
 //for (uint8_t bit = 8; bit > 0; --bit)
 for (uint8_t bit = 0; bit <8; bit++) 
 { 
   if((*crc_val & 0x80)!=0) {*crc_val = (*crc_val << 1) ^ POLYNOMIAL;}
   else {*crc_val = (*crc_val << 1);}
 }
}