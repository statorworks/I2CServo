//
//I2C servo control
//ATtiny816
//Based on servo project

#include "main.h"


int main(void)
{
	//peripherals
	init();

    while(1) 
    {
      
	  //Rtos 50hz
   	  if(timeslice==1){
		timeslice=0;	  
								
		//Update
	    StateManager();
		  
		//testing();
	    
		//watchdog reset
		wdt_reset();
	  
	 }//timeslice
   		wdt_reset();
   }//while
}

void StateManager(void){
 //Only one simple state needed 
 
 //Flick red and green Leds as necessary 
 if(led_blink_period_count>=LED_BLINK_PERIOD){
  if(red_led_blink_count>0){ RED_LED_TOGGLE; red_led_blink_count--; } else{RED_LED_OFF;}
  if(green_led_blink_count>0){ GREEN_LED_TOGGLE; green_led_blink_count--; } else{GREEN_LED_ON;} 
  led_blink_period_count=0;
 }
 else{led_blink_period_count++;};
  
  
 //Get desired address value from dip switch
 volatile uint8_t address = GetDipSwitch();
  
 //Check address set button
 if((PORTC_IN & PIN1_bm) == 0){  if(button_down_time<250){button_down_time++;} button_up_time=0; }
 else{ if(button_up_time<250){button_up_time++;} button_down_time=0; }
  
 //Get pot values
 volatile uint16_t pot_pow = GetAD(8);
 pot_pow = ((uint32_t)pot_pow*100)/1024;  //0-100
 volatile uint16_t pot_pos = GetAD(9); 
 
 
 
 //Set servo i2c address when button pressed for one second. Servo must be only one on bus for this.
 if(button_down_time==50){
   
   //only let user set address within valid range
   if((address<I2C_ADDRESS_RANGE_START)||(address>I2C_ADDRESS_RANGE_END)){
		red_led_blink_count=2; green_led_blink_count=0; 
	}
	
   else{//Ok
		data_out[0] = address;
		//Current address unknown, so send it to every address
		uint8_t add;
		for(add = 0x00; add<=0x7F; add++){			
		
			I2C_MasterWrite(0, add, REG_I2C_ADDRESS, data_out, 1);	 	 
			if(i2c_transfer_failed==0){ green_led_blink_count=6; red_led_blink_count= 0; break; }
		}
		//No servo responded
		if(add>I2C_ADDRESS_RANGE_END){ red_led_blink_count=6; green_led_blink_count=0; }
	}
  }
 
 
 //Check change in power pot
 else if(pot_pow!=old_pot_pow){
  
  uint8_t  max_power  = 20 + (((uint32_t)pot_pow*80)/100);   //20-100
  uint16_t max_speed  = 2500;				//units/s 
  uint16_t ramp_time  = (100-pot_pow) * 15; //0-1500 ms
  uint8_t  ramp_curve = 100-pot_pow;		//linear/sine
  //
  data_out[0] = max_power;
  data_out[1] = max_speed>>8;
  data_out[2] = max_speed;
  data_out[3] = ramp_time>>8;
  data_out[4] = ramp_time;      
  data_out[5] = ramp_curve; 
  I2C_MasterWrite(0, address, REG_MAX_POWER, data_out, 6);	 
  if(i2c_transfer_failed==1){ red_led_blink_count = 2; }
  else{ green_led_blink_count = 2; }
 }
  
 //Check change in position pot
 else if(pot_pos!=old_pot_pos){
	
  data_out[0] = pot_pos>>8;
  data_out[1] = pot_pos;
  I2C_MasterWrite(0, address, REG_TARGET_POSITION_H, data_out, 2);   
  if(i2c_transfer_failed==1){ red_led_blink_count = 2; }
  else{ green_led_blink_count = 2; } 
 } 
   
   
 //save old pot values
 old_pot_pow = pot_pow;
 old_pot_pos = pot_pos;

 
 //Also send pulse to test a regular servo on aux output:
 uint16_t per = 1000 + pot_pos; //standard servos range around 1000-2000us 
 SendPulse(per);    

 state_time++;
}

uint8_t GetDipSwitch(void){

//Pcb Layout favored for two layers so we have to format the bits here:
	
//invert
volatile uint8_t inv = ~PORTA_IN;

//remap
volatile uint8_t remap = 0;
remap |=  (inv>>3)&0x01;
remap |=  (inv>>1)&0x02;
remap |=  (inv<<1)&0x04;
remap |=  (inv>>4)&0x08;
remap |=  (inv>>2)&0x10;
remap |=  (inv>>0)&0x20;
remap |=  (inv<<2)&0x40;

//flip bits left right
volatile uint8_t ds=0;
for(uint8_t i=0;i<7;i++){ ds |= ((remap>>i)&0x01)<<(6-i); }

return ds;	
}


uint16_t GetAD(uint8_t n){

if(n>=MAX_AD_CHANNELS){ return 0; }
	
uint16_t val;	


//Save relevant registers so we can restore them below
volatile uint8_t adr[4];
adr[0] = VREF.CTRLA;
adr[1] = ADC0.CTRLA;
adr[2] = ADC0.CTRLC;
adr[3] = ADC0.MUXPOS;

//setup
ADC0.CTRLA = 0x81;       //single measurement, stay enabled

//select adc temp channel
ADC0.MUXPOS = n;  //errata says free running mode must be changed to single measurement before this.
volatile uint16_t i;
for(i=0; i<1000;i++){;}//needs some time to take effect.

//start one conversion
ADC0.COMMAND = 0x01;

//wait result. but have a way out.
//volatile uint16_t timeout=0;
for(i=0;i<1000;i++){;}
//while( ADC0.MUXPOS != 0x1E){} //none of these checks hold
//while((ADC0.COMMAND&0x01) ==0x01){}
//while((ADC0.INTFLAGS&0x01)!=0x01){}

val = ADC0.RES;


//restore adc settings. and begin sampling again
VREF.CTRLA  = adr[0];
ADC0.CTRLA  = adr[1];
ADC0.CTRLC  = adr[2];
ADC0.MUXPOS = adr[3];
ADC0.COMMAND = 0x01; //start
for(i=0;i<1000;i++){;} //wait
//while((ADC0.COMMAND&0x01) ==0x01){}//none of these checks hold
//while((ADC0.INTFLAGS&0x01)!=0x01){}

return val;	
}


void SendPulse(uint16_t time_us){
 //Can test a regular servo with this on auxiliary header terminal

 //Up
 asm("cli"); //no interrupts
 SERVO_PULSE_UP;
 
 //wait
 uint16_t per;
 uint16_t adj_us = time_us/2; //tweak it here. good enough.
 //
 for(per=0; per<adj_us; per++){
  asm("nop");
  //asm(nop);
 }
  
 //Down
 SERVO_PULSE_DOWN;
 asm("sei"); 	
}

uint32_t myabs(int32_t v){
	
 if(v<0){ v = -v; }
 return (uint32_t)v;	 	
}

//Timer interrupt handler
ISR(TCA0_OVF_vect){

  timeslice =1;
	
  //clear flag
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm;
}

void init(void){

 volatile uint32_t i;
 
 DISABLE_INTERRUPTS;

 //Wait a fraction of a second for stable power, to protect eeprom contents.
 for(i=0;i<30000;i++){ i=i; wdt_reset(); }
	 
  
 //Main clock control, 16/20Mhz internal 
 //special Atmel Start assembly function to write register value under 4-cycles after key 
 protected_write_io((void *)&CLKCTRL.MCLKCTRLA, CCP_IOREG_gc, 0);  
 //div=x2: 10Mhz max for 3.3V supply:
 protected_write_io((void *)&CLKCTRL.MCLKCTRLB, CCP_IOREG_gc, (CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm));
  
  
 //GPIO
 //PORTA_DIRSET = PIN0_bm;    //UPDI programming (fuse setting) 
 //Red LED
 //Green Led
 //Set switch
 PORTA_DIRSET = ~(0xFE);	//1-7 used for dip switch  (pa0 is updi line)
 PORTA_PIN1CTRL = 0x08;		//pull-ups on
 PORTA_PIN2CTRL = 0x08;	
 PORTA_PIN3CTRL = 0x08;	
 PORTA_PIN4CTRL = 0x08;	
 PORTA_PIN5CTRL = 0x08;	
 PORTA_PIN6CTRL = 0x08;	
 PORTA_PIN7CTRL = 0x08;	      
 //
 PORTB_OUTCLR = PIN2_bm;    //Off
 PORTB_DIRCLR = PIN0_bm;	//I2C SCL
 PORTB_DIRCLR = PIN1_bm;	//I2C SDA
 PORTB_DIRCLR = PIN4_bm;	//POT 1 (power) read
 PORTB_PIN4CTRL = 0x08;     //POT pull-up on
 PORTB_DIRCLR = PIN5_bm;	//POT 2 (position) read
 PORTB_PIN5CTRL = 0x08;     //POT pull-up on
 PORTB_OUTSET = PIN5_bm;    //On 
 //
 PORTC_DIRSET = PIN0_bm;    //PWM for regular servo test on auxiliary header
 PORTC_OUTCLR = PIN0_bm;    //Off
 PORTC_DIRCLR = PIN1_bm;	//Set swtich
 PORTC_PIN1CTRL = 0x08;     //pull-up on
 PORTC_DIRSET = PIN2_bm;	//Green LED
 PORTC_OUTSET = PIN2_bm;    //On
 PORTC_DIRSET = PIN3_bm;	//Red LED
 PORTC_OUTCLR = PIN3_bm;    //Off


	
 //Timer A: Rtos 50Hz
 TCA0.SINGLE.CTRLA = 0; //disable to begin with
 TCA0.SINGLE.CTRLD = 0; //Always simple 16-bit mode (not split) for simplicity
 TCA0.SINGLE.CTRLECLR = 0x01; //bit0=0: count always up
 //
 //Prescaler 10Mhz/256 = 39Khz.   
 TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV256_gc;
 TCA0.SINGLE.PER = 780; //50hz
 //
 TCA0.SINGLE.CTRLB   = 0<<TCA_SINGLE_WGMODE0_bp;	//normal count
 TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;			//timeslice interrupt
 TCA0.SINGLE.CTRLA  |= TCA_SINGLE_ENABLE_bm;		//start 



 //ADC INPUTS, PB5(AD8) PB4(AD9)
 ADC0.CTRLA  = ADC_FREERUN_bm;	//10-bit resolution, free running mode. disabled to begin with.
 ADC0.CTRLB  = 0;				//no result accumulation
 ADC0.CTRLC  = (1<<6) | (1<<4) | 0x02;   //reduced sample capacitance, vdd as reference, prescaler divider=x8 (2.5Mhz/8= 300Khz)
 ADC0.CTRLD  = (0<<5) | (0<<4) | (0<<0); //no init delay, no sampling delay variation, no sampling delay
 ADC0.CTRLE  = 0;				//no window comparison
 //ADC0.MUXPOS = 0x08;			//ADIN-8  pin-9 (PB5)
 ADC0.MUXPOS = 0x09;			//ADIN-9  pin-10 (PB4)
 //
 ADC0.CTRLA   |= ADC_ENABLE_bm;	//enable
 ADC0.COMMAND |= ADC_ENABLE_bm; //start converting right away
 //
 //Test first sample
 //while((ADC0.INTFLAGS & 0x01)==0){;}
 //uint16_t testres = ADC0.RES;


 //I2C Master 
 i2c_Init(0, 1, 0, 100000);		
 i2c_transfer_in_data  = i2cservo_in_data;       //receive buffer. no direct write to registers so values can be checked first.
 //i2c_transfer_out_data = (void*)&regs_read_copy; //user can freely read registers directly.
 //i2c_receive_complete_callback = &RegisterWriteCallback; //received data processed immediately so user packets can be sent one right after the other 
      
	  
 //Watchdog timer, 0.256Sec period. Set with the fuse, so we save some code space here
 //protected_write_io((void *)&WDT.CTRLA, CCP_IOREG_gc, 0x06);  
 //wdt_reset(); 
 	  
	  
 //initial state
 timeslice=0;
 state_time=0;
  
  
 //Ok 
 ENABLE_INTERRUPTS;

}

void testing(void){

 // uint32_t i;

  //toggle test pin
 // toggle_test_pin_time++;
 // if(toggle_test_pin_time>=125){
  //  toggle_test_pin_time = 0;
  //  PORTB_OUTTGL |= PIN5_bm;
  //}
		
  //Test pwm ramp  
  //TCD0.CTRLA  &= ~(0x01); //must disable for change
  //TCD0.CMPACLR = PWM_PERIOD_COUNT/2;
  //TCD0.CMPACLR++;
  //if(TCD0.CMPACLR>PWM_PERIOD_COUNT){TCD0.CMPACLR=0;}  //ramp up then zero
  //TCD0.CTRLA |= TCD_ENABLE_bm; //enable 
  
  //Test adc with pot
  //uint16_t adc_res = ADC0.RES;

  //Test motor speed with pot
  //TCD0.CTRLA   &= ~(0x01);		//must disable for change
  //TCD0.CMPACLR  = 50+ ((uint32_t)adc_res * PWM_PERIOD_COUNT)/1024;	//map 1024 to 250
  //if(TCD0.CMPACLR>PWM_PERIOD_COUNT) { TCD0.CMPACLR=PWM_PERIOD_COUNT; }
  //TCD0.CMPBSET  = PWM_PERIOD_COUNT-(adc_res>>2);
  //TCD0.CTRLA |= TCD_ENABLE_bm;	//enable 
  //while((TCD0.STATUS & TCD_ENRDY_bm)!=TCD_ENRDY_bm){}//wait sync
  
   //Quick test of servo back and forth
   //if(adc_res<400){
   /// TCD0.CTRLA   &= ~(0x01);		//must disable for change
   //  TCD0.CMPACLR = ((uint32_t)PWM_PERIOD_COUNT *70)/100; //TEST%  
   //  TCD0.CMPBSET = PWM_PERIOD_COUNT;
   //  TCD0.CTRLA |= TCD_ENABLE_bm;	//enable 
   // }
   //else if(adc_res>500){  
   // TCD0.CTRLA   &= ~(0x01);
   // TCD0.CMPACLR = 0;
   // TCD0.CMPBSET = PWM_PERIOD_COUNT - (((uint32_t)PWM_PERIOD_COUNT *70)/100); //%
   //  TCD0.CTRLA |= TCD_ENABLE_bm;	//enable 
   //}
   

 //Test I2C as master
 //uint8_t data[16]={ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 //if(i2c_initialized==0){
	
 //	i2c_Init(0, 1, 0, 100000);  //Tests: Read/Write Ok up to 200Khz, even with long cables. 250K starts losing the first data byte.
	//i2c_use_crc =1; //optional 
	    
	//test ads1115 ad breakout
    //#chn 0 single ended + continuous conversion + 4V scale range + 128sps + no comparator stuff
    //data[0] = 0x42;
	//data[1] = 0x80;
    //I2C_MasterWrite(0,0x48, 1, data, 2);
  //}
  //
  //while(1){
	
	//read ads1115
	//data[0]=5;
	//data[1]=7;
    //I2C_MasterRead(0,0x48, 0, data, 2); 
	//volatile uint16_t val = ((uint16_t)data[0]<<8) | data[1];

	//for(i=0;i<2000;i++){ ; }
 // }
  
 
  
  //Test I2C as slave 
  //uint8_t data[8]; //= {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
  //i2c_transfer_data = data;
  //i2c_transfer_data = i2cservo_in_data;
  //if(i2c_initialized==0)
  //{i2c_Init(0, 0, 0x25, 0); }
  //
  //while(1){//just wait here for master transfer  
   
   //  if(i2c_transfer_done ==1 ){
     // uint8_t val= data[0];	 
	//  i2c_transfer_done=0;
	// }
   //}
  
 //test crc8
 //uint8_t crcdata[8]={ 0x74, 0x23, 0x66,0x98,0x18,0x73,0x15,0x01 };
 //uint8_t crcdata[8]={ 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88 };
 //uint8_t crc_val=0x00;
 //for(i=0;i<8;i++){
 //  crc8byte(&crc_val, crcdata[i]);
 // }
 //crc_val = crc8(crcdata, 4);  //measured performance: 2.4us for eight bytes


 //1 sec	 
 //if(testing_time>=600){//1400){ 	

 // testing_time=0;

  //Test Eeprom
  //uint8_t ewdata[64];// = //{ 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88 };
  //uint8_t erdata[64];
  //for(i=0;i<64;i++){ ewdata[i]= i; erdata[i]=0; }
  //EEPROM_Write(0, ewdata, 64);
  //EEPROM_Read(0, erdata, 64);
  //for(i=0;i<10000;i++){ ; }
  //}
    
  //test temperature reading
  //volatile uint8_t degc = get_temperatureC();
  
 // if(testing_step==0){

 
    //fake i2c write
	//regs.max_speed=BE16(3600);
	//regs.ramp_time=BE16(200);
	//regs.ramp_curve=90;
	//regs.deadband=5;
	//regs.park_type=100;
	//regs.max_power=80;
	//i2c_transfer_register = REG_TARGET_POSITION_H;
	//i2cservo_in_data[0] = 0x00;
	//i2cservo_in_data[1] = 0xBC;
	//i2c_transfer_count = 2;
	//RegisterWriteCallback();
	//ProcessRegisterWrite();
	
	//simple change
    //regs.target_position=BE16(200);//200);	
    //front_end_target_position=300;
	//new_target_position=1;
 // }
  //if(testing_step==1){
	
    //fake i2c write
	//i2c_transfer_register = REG_TARGET_POSITION_H;	
	//i2cservo_in_data[0] = 0x03;
	//i2cservo_in_data[1] = 0xBC;
	//i2c_transfer_count = 2;
	//RegisterWriteCallback();
	//ProcessRegisterWrite();

	//i2c_transfer_register = REG_PROGRAM_REPS;	
	//i2cservo_in_data[0] = 0x04; //
	//i2cservo_in_data[1] = 0x01; //start
	//i2c_transfer_count = 2;
	//ProcessRegisterWrite();

	//simple change
    //regs.target_position=BE16(850);//800);
    //front_end_target_position = 600;
	//new_target_position=1;
 // }
  //testing_step++;
 // if(testing_step>=2){testing_step=0;}
 
// }
 
// testing_time ++;
}
 
int16_t BE16(int16_t val){
 	
 //switch endiannes	on 16-bit data
 volatile uint16_t temp = (uint16_t)val;
 temp = (temp<<8)|(temp>>8);
 return (int16_t)temp;  	
}
