//
//Statorworks 2020
//Minimalistic Oled SSD1306 screen text writing and bitmap drawing. Use with small boards like the Digispark and Arduino Nano.
//No frame buffer, small 8x8 font. User can also draw small bitmap widgets defined by the application.
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

//Abstracted i2c interface here.
#define OLEDSSD1306_I2C_ADDRESS  0x3C  //address without any left shift (not 0x78)
//I2c assumed to be initalized by application, we just need the write reference here:
#define OLEDSSD1306_I2C_WRITE(add, reg, buff, len)  I2CWrapper_Write(add, reg, buff, len, 0)


//AVR progmem flash storage of character array 
#define AVR_PROGMEM 1  //comment this out for other systems


//Small 8x8 raw Font. Plus a few widgets at the end. 
//Font modified from William Greiman's glcd 8x8 font: 
//github.com/greiman/SSD1306Ascii/blob/master/src/fonts/font8x8.h
#define OLEDSSD1306_FONT_8x8_RAW_SIZE 1210  //1024
#define OLEDSSD1306_FONT_8x8_CHARCOUNT 150  //27
extern const byte OLEDSSD1306_FONT_8x8[OLEDSSD1306_FONT_8x8_RAW_SIZE];
//if you add  a different 8x8 font, reference it here once:
#define OLEDSSD1306_FONT  OLEDSSD1306_FONT_8x8 


//Command register space
#define OLEDSSD1306_CHARGE_PUMP_ON  0x14
#define OLEDSSD1306_CHARGE_PUMP_OFF 0x10
#define OLEDSSD1306_SET_CONTRAST    0x81
#define OLEDSSD1306_INVERTED_OFF    0xA6
#define OLEDSSD1306_INVERTED_ON     0xA7
#define OLEDSSD1306_UPRIGTH         0xC8
#define OLEDSSD1306_UPDOWN          0xC0
#define OLEDSSD1306_LEFTRIGHT       0xA1
#define OLEDSSD1306_RIGHTLEFT       0xA0
#define OLEDSSD1306_TEST_LIT_ON     0xA5
#define OLEDSSD1306_TEST_LIT_OFF    0xA4
#define OLEDSSD1306_DISPLAY_OFF     0xAE
#define OLEDSSD1306_DISPLAY_ON      0xAF
#define OLEDSSD1306_SETCOLUMN_L     0x00
#define OLEDSSD1306_SETCOLUMN_H     0x10
#define OLEDSSD1306_SETADDRESSINGMODE 0x20
#define OLEDSSD1306_SETCOLUMNADDRESS  0x21   
#define OLEDSSD1306_SETPAGEADDRESS    0x22
#define OLEDSSD1306_SETPAGESTART      0xB0
#define OLEDSSD1306_SCROLLSTOP      0x2E

//other
#define OLEDSSD1306_SIZE_X 128
#define OLEDSSD1306_SIZE_Y 8 //8 horizontal pages 



//-----Easy high level API -------------------------------------------


void OledSSD1306_Begin(){

 //I2c assumed to be initalized by application
 //Nothing here now. kept for consistency
}

void OledSSD1306_Setup(bool on, bool flip_h, bool flip_v, bool clear_screen, byte contrast, bool inverted, bool enable_charge_pump, bool test_all_lit){

  byte cmd[2];
 
  //charge pump on
  //mandatory for i2c displays running on 3.3v without separate screen vcc
  cmd[0]=0x8D; 
  if(enable_charge_pump==1){cmd[1]=OLEDSSD1306_CHARGE_PUMP_ON;}
  else{cmd[1]=OLEDSSD1306_CHARGE_PUMP_OFF;}
  OledSSD1306_WriteCommand(&cmd[0], 2);
 
  //set contrast
  if(contrast>0x7F){contrast=0x7F;}
  cmd[0]=OLEDSSD1306_SET_CONTRAST;
  cmd[1]=contrast; //0x50; //level 0 to 7F
  OledSSD1306_WriteCommand(&cmd[0], 2);
 
  //inverted
  cmd[0] = OLEDSSD1306_INVERTED_OFF;
  if(inverted==1){ cmd[0] = OLEDSSD1306_INVERTED_ON;}
  OledSSD1306_WriteCommand(&cmd[0], 1);

  //orientation
  cmd[0] = OLEDSSD1306_UPRIGTH;  //upright(yellow is top on yello/blue display)
  if(flip_v==1){ cmd[0] = OLEDSSD1306_UPDOWN; }  
  OledSSD1306_WriteCommand(&cmd[0], 1);
  //
  cmd[0] = OLEDSSD1306_LEFTRIGHT; //left to right orientation
  if(flip_h==1){ cmd[0] = OLEDSSD1306_RIGHTLEFT; }
  OledSSD1306_WriteCommand(&cmd[0], 1);

  //stop any scrolling going on 
  //or memory contents will get a bit scrambled when user prints something after setup
  cmd[0] = OLEDSSD1306_SCROLLSTOP;
  OledSSD1306_WriteCommand(&cmd[0], 1); 
 
  //clear screen memory to all zeros
  if(clear_screen==1){
    OledSSD1306_fill(0, 0, OLEDSSD1306_SIZE_X, OLEDSSD1306_SIZE_Y, 0x00);
   }
  
  //test, all pixels lit
  cmd[0] = OLEDSSD1306_TEST_LIT_OFF;
  if(test_all_lit==1){ cmd[0] = OLEDSSD1306_TEST_LIT_ON; }
  OledSSD1306_WriteCommand(&cmd[0], 1);

  //work with page addressing mode for simplicity. i.e. no auto row increment. cursor set manually on each page row drawing
  cmd[0] = OLEDSSD1306_SETADDRESSINGMODE;
  cmd[1] = 0x02;
  OledSSD1306_WriteCommand(&cmd[0],2);
  
  //display On/Off
  if(on==1){ cmd[0] = OLEDSSD1306_DISPLAY_ON; }
  else{ cmd[0] = OLEDSSD1306_DISPLAY_OFF; }
  OledSSD1306_WriteCommand(&cmd[0], 1);
}

void OledSSD1306_fill(byte x, byte y, byte w, byte h, byte pattern){
 //Fill page rows accross with 8bit column pattern

 //Check bounds
 if(x>=(OLEDSSD1306_SIZE_X)){return;}
 if(y>=(OLEDSSD1306_SIZE_Y)){return;}
 if((x+w)>(OLEDSSD1306_SIZE_X)){ w = OLEDSSD1306_SIZE_X - x; }
 if((y+h)>(OLEDSSD1306_SIZE_Y)){ h = OLEDSSD1306_SIZE_Y - y; }

 byte i,r,c,n;
 byte ccount=0;

 //Small buffer to send chunks of horizontal page
 //The whole 128 row would be faster, but this is for small devices so we save some stack.
 byte dt[16];
 for(i=0;i<16;i++){dt[i] = pattern;}
 
 //Horizontal pages
 for(i=y; i<(y+h);i++){
   
   //track cursor horizontally
   ccount=x;
   
   //start on left
   OledSSD1306_SetCursor(x, i);
    
   //fill row page, 1-pixel column at a time from left to right
   for(c=0; c<8;c++){//up to eight chunks

     if(ccount>=(x+w)){break;}//up to width
     if(ccount>=128){break;}//done if hit right side
     n = 16;
     if((128-ccount)<16){ n= 128-ccount; }
     //
     OledSSD1306_WriteData(&dt[0], n);
     ccount+=n;
   }
 }//i
//
}

byte OledSSD1306_PrintValue(int32_t value, byte x, byte y, byte alignment, bool width_double, bool height_double, byte* or_mask, byte* and_mask, bool inverted, byte max_len, byte leading_zero_option){

 //check
 if(x>=OLEDSSD1306_SIZE_X){return 0;}
 if(y>=OLEDSSD1306_SIZE_Y)  {return 0;}
  
 char str[16];
 byte len=0;
 byte i, digit;
 uint32_t v;
 bool lea=1;//track leading zeros until we find non zero

 //sign
 if(value<0){
  str[0]='-';
  len=1;
 } 
 uint32_t val = abs(value);
   
 //val to string
 if(max_len>10){max_len=10;}//32bit
 uint32_t den = 1000000000;
 for(i=10; i>max_len; i--){den /=10;} //prep handy divider
 if(value<0){ den /=10; }//if (-)has taken one slot above
 //
 for(i=0; i<10; i++){

   if(len>(max_len-1)){break;}
   if(den==0){break;}
   
   //single decimal value
   v = val/den;
   v = v%10;
   den /=10;
   
   //to digit
   if((den>0)&&(v==0)&&(lea==1)){
    if(leading_zero_option==1){ digit = 32; } //' 'space, this is useful when aligned to the right, clears previously printed characters
    else if(leading_zero_option==2){ digit = 48; } //'0'
    else{ continue; }//no leading zeros
    }
    else{
     lea=0;//no more leading zeros
     digit = v+48; //ascii base '0'
    }

   //to string
   str[len] = digit; 
   len++;
 }
 str[len] = 0;//null termination


 byte cwidth=8;
 if(width_double==1){cwidth=16;}
 
 //start position from alignment and length
 int8_t posx = x; //alignemnt =0 left
 if(alignment==1){ posx = x - (len*cwidth)/2; }  //centered
 if(alignment==2){ posx = x - (len*cwidth);   }  //right
 if(posx<0){ posx=0; }//stay inside left bound 
 
 //draw it  
 for(byte i=0; i<len; i++){

   //look up character data and write it
   OledSSD1306_PrintCharacter((byte)str[i], (posx+(i*cwidth)), y, width_double, height_double, 0, or_mask, and_mask, inverted);
  }

 //give user the length so they can compose on their end
 return len; 
}

byte OledSSD1306_PrintText(char* string, byte x, byte y, byte alignment, bool width_double, bool height_double, byte* or_mask, byte* and_mask, bool inverted){

 //check
 if(string==0){return 0;}
 if(x>=OLEDSSD1306_SIZE_X){return 0;}
 if(y>=OLEDSSD1306_SIZE_Y)  {return 0;} 
 
 //get length
 byte len=0;
 for(byte i=0;i<16;i++){
  //check for null termination
  if(string[i]==0){break;}
  len++;
 }

 byte cwidth=8;
 if(width_double==1){cwidth=16;} 
 
 //start position from alignment and length
 int8_t posx = x; //alignemnt =0 left
 if(alignment==1){ posx = x - (len*cwidth)/2; }  //centered
 if(alignment==2){ posx = x - (len*cwidth);   }  //right
 if(posx<0){ posx=0; }//stay inside left bound 
 
 //draw it 
 for(byte i=0; i<len; i++){

   //look up character data and write it
   OledSSD1306_PrintCharacter((byte)string[i], (posx+(i*cwidth)), y, width_double, height_double, 0, or_mask, and_mask, inverted);
  }

 //give user the length so they can compose on their end
 return len; 
}

void OledSSD1306_PrintCharacter(byte character, byte x, byte y, bool width_double, bool height_double, byte repeats, byte* or_mask, byte* and_mask, bool inverted){

 //check
 if(x>=OLEDSSD1306_SIZE_X) {return;}
 if(y>=OLEDSSD1306_SIZE_Y) {return;}
 if(repeats>16){return;}

 //table starts at ' ' space. Also character must be byte instead of char so it can go past 127
 if(character<32){character=32;}
 character -= 32;

 //Copy character
 byte i;
 byte cpy[8];
 uint16_t location = (uint16_t)character*8;  //on font table
 for(i=0; i<8;i++){

   //on AVR font is in flash via PROGMEM
   cpy[i] = pgm_read_byte(&OLEDSSD1306_FONT[location]+i);
   //other
   //...
  }

 //draw it
 OledSSD1306_DrawBitmap8x8(&cpy[0], x, y, width_double, height_double, repeats, or_mask, and_mask, inverted);
 //
}

void OledSSD1306_DrawBitmap8x8(byte* bitmap, byte x, byte y, bool width_double, bool height_double, byte repeats, byte* or_mask, byte* and_mask, bool inverted){
 byte i,c;
 byte col8;
 uint16_t col16;
 byte* ptr; //final place to print from at the end

 //check
 if(x>=OLEDSSD1306_SIZE_X) {return;}
 if(y>=OLEDSSD1306_SIZE_Y) {return;}
 if(repeats>16){return;}
 
 //Working copy
 byte b;
 byte cpy1[32];
 for(i=0; i<8;i++){
    cpy1[i] = bitmap[i];
  }
  
 //Default location
 ptr=&cpy1[0];
 byte width=8;  //in bytes
 byte height=1; //in bytes

 //Or mask
 if(or_mask!=0){
  for(i=0; i<8;i++){
   ptr[i] |= or_mask[i];
  }
 }

 //And mask
 if(and_mask!=0){
  for(i=0; i<8;i++){
   ptr[i] &= and_mask[i];
  }
 }

 //Invert
 if(inverted==1){
  for(i=0; i<8;i++){
   ptr[i] = ~ptr[i];
  }
 }
  
 //Double height
 byte cpy2[32];
 if(height_double==1){
  for(c=0; c<8;c++){ //stretch each colum down
    col8  = ptr[c];
    col16 = 0; 
    for(i=0;i<8;i++){
      if(((col8>>i)&0x01)==1){
        col16 |= (uint16_t)3<<(i*2); //expand each bit x2 into the 16 bit variable  
      }
     }
    //paste
    cpy2[c]   = col16&0xFF;  //display bits go down-up, so here we use lsB
    cpy2[c+16] = col16>>8;   //and here msB
   }
  ptr=&cpy2[0];
  height=2;
 }

 //Double width
 byte cpy3[32];
 if(width_double==1){
   byte nc=0;
   for(c=0; c<8; c++){//duplicate each column 
     col8 = ptr[c];
     cpy3[nc] = col8;   
     cpy3[nc+1] = col8;
     //
     if(height==2){
       col8 = ptr[c+16];
       cpy3[16+nc] = col8;   
       cpy3[16+nc+1] = col8;        
      }
     nc+=2;
   }
  ptr=&cpy3[0];
  width=16;
 } 
   
 //Send
 byte xx = x; //horizontal track 
 for(i=0; i<=repeats; i++){//1+n times horizontally
    if(xx>127){break;}//stay in bounds
    if((xx+width)>127){width =127-xx; repeats=0;}
    //
    OledSSD1306_SetCursor(xx, y);
    OledSSD1306_WriteData(ptr, width);
    //
    if((y<7)&&(height==2)){//second row
      OledSSD1306_SetCursor(xx, y+1);
      OledSSD1306_WriteData(ptr+16, width);  
     }
    xx+=width;
  }//
// 
}

void OledSSD1306_Scroll(bool enable, byte x, byte y, byte w, byte h, bool horizontal, bool vertical, int8_t rate){

 byte cmd[7];
 
 //stop scrolling
 if(enable==0){
  cmd[0] = OLEDSSD1306_SCROLLSTOP;
  OledSSD1306_WriteCommand(&cmd[0], 1);   
  return;
 }

 //check
 if(x>=OLEDSSD1306_SIZE_X) {return;}
 if(y>=OLEDSSD1306_SIZE_Y) {return;}
 if(w==0){return;}
 if(h==0){return;}
 if((x+w)>=OLEDSSD1306_SIZE_X){w= OLEDSSD1306_SIZE_X-1-x;} 
 if((y+h)>=OLEDSSD1306_SIZE_Y){h= OLEDSSD1306_SIZE_Y-1-y;}     
 if((horizontal==0)&&(vertical==0)){return;}
  
 //direction
 bool dir=1;
 if(rate<0){ dir=0; rate = -rate; } 

 //rate needs a bit of translation. see datasheet page 28.
 if(rate>7){rate=7;}
 byte ratelut[8]={3,2,1,0,6,5,4,7};
 rate = ratelut[rate];
  
 //setup horizontal
 if((horizontal==1)&&(vertical==0)){
   cmd[0] = 0x26; //right
   if(dir==0){ cmd[0] = 0x27;} //left
   cmd[1] = 0;   //dummy
   cmd[2] = y;   //top 
   cmd[3] = rate;
   cmd[4] = y+h; //bottom 
   cmd[5] = x;   //left 
   cmd[6] = x+w; //right
   OledSSD1306_WriteCommand(&cmd[0], 7); 
 }//
 
 //setup vertical (more cumbersome)
 else{
   //Seems parameters #2 and #4 would control how many rows scroll fully vertical,
   //but appears buggy, thus only have diagonal scroll, and no vertical support, as per datasheet.
   cmd[0] = 0x29; //right
   if(dir==0){ cmd[0] = 0x2A;} //left
   cmd[1] = 0;//dummy
   cmd[2] = 0;//fixed diagonal   
   cmd[3] = rate;
   cmd[4] = 7;//fixed diagonal 
   cmd[5] = 0x01;//scrolling offset[??]
   OledSSD1306_WriteCommand(&cmd[0], 6); 
   //
   //Now the actual window that shows diagonal scrolling. Note that the parameters here are in pixel-rows not pages.
   //there doesn't seem to be a control of start column and end column window when doing this.
   //sometings' buggy here, if you tell it to scroll the center area, the top area is not fixed but scrolls sidewayas
   //so we leave it as whole screen area scrolling 
   //cmd[0] = 0xA3;//set top
   //cmd[1] = 00;//y;   0 and 64= whole screen (default anyway) 
   //cmd[2] = 64;//h;   
   //OledSSD1306_WriteCommand(&cmd[0], 3);    
  }//

 //start
 cmd[0] = 0x2F;
 OledSSD1306_WriteCommand(&cmd[0], 1);   
//  
}



//---Low level---------------------------------------------------------------------------


void OledSSD1306_WriteCommand(byte* cmd, byte len){

  //check
  if(cmd==0){return;}
  if(len==0){return;}
  
  //C/D byte: 0x00 = single command, 0x08=multiple commands, 0x04= data 
  OLEDSSD1306_I2C_WRITE(OLEDSSD1306_I2C_ADDRESS, 0x00, cmd, len); //define write implementation at the top only once 
}

void OledSSD1306_WriteData(byte* dat, uint16_t len){
  
  //Write data to screen ram, starting at current cursor position
  //Display automatically increments cursor column register for each byte.
  //
  //Do it in chunks of 16 in case the i2c library has a limit.
  //For example tinywirem has a limit of sending 16 bytes at a time.
  //this also helps in terms of not losing clocksync on long data

  //check
  if(dat==0){return;}
  if(len==0){return;}
  
  uint16_t n_chunks = len/16;
  byte rem = len%16;
  uint16_t count=0;

  for(uint16_t i=0; i<n_chunks;i++){
     
     //C/D byte: 0x00 = single command, 0x08=multiple commands, 0x40= data 
     OLEDSSD1306_I2C_WRITE(OLEDSSD1306_I2C_ADDRESS, 0x40, (dat+count), 16); //define write implementation at the top only once     
     count +=16;
  }
  //remainder
  OLEDSSD1306_I2C_WRITE(OLEDSSD1306_I2C_ADDRESS, 0x40, dat+count, rem);
//
}

void OledSSD1306_SetCursor(byte x, byte y){
  
  //check
  if(x>=OLEDSSD1306_SIZE_X) { return; }
  if(y>=OLEDSSD1306_SIZE_Y) { return; }  

  //X
  byte cmd = OLEDSSD1306_SETCOLUMN_L | (x & 0XF);
  OledSSD1306_WriteCommand(&cmd,1);
  cmd = OLEDSSD1306_SETCOLUMN_H | (x >> 4);
  OledSSD1306_WriteCommand(&cmd, 1);
  
  //Y
  cmd = OLEDSSD1306_SETPAGESTART  | y; 
  OledSSD1306_WriteCommand(&cmd,1);
}


//TODO: move one space from right side to leading left side
//Font modified from William Greiman's glcd 8x8 font: 
//github.com/greiman/SSD1306Ascii/blob/master/src/fonts/font8x8.h
//Removed header, shifted characters right by one pixel, and added extra widgets. 
const byte OLEDSSD1306_FONT_8x8[OLEDSSD1306_FONT_8x8_RAW_SIZE] PROGMEM= {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // <space>
0x00,0x00,0x00,0x00,0x00,0x5F,0x00,0x00,  // !
0x00,0x00,0x00,0x00,0x03,0x00,0x03,0x00,  // "
0x00,0x24,0x7E,0x24,0x24,0x7E,0x24,0x00,  // #
0x00,0x00,0x00,0x2E,0x2A,0x7F,0x2A,0x3A,  // $
0x00,0x46,0x26,0x10,0x08,0x64,0x62,0x00,  // %
0x00,0x20,0x54,0x4A,0x54,0x20,0x50,0x00,  // &
0x00,0x00,0x00,0x04,0x02,0x00,0x00,0x00,  // '
0x00,0x00,0x00,0x3C,0x42,0x00,0x00,0x00,  // (
0x00,0x00,0x00,0x42,0x3C,0x00,0x00,0x00,  // )
0x00,0x10,0x54,0x38,0x54,0x10,0x00,0x00,  // *
0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00,  // +
0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,  // ,
0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x00,  // -
0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,  // .
0x00,0x40,0x20,0x10,0x08,0x04,0x00,0x00,  // /

0x00,0x3C,0x62,0x52,0x4A,0x46,0x3C,0x00,  // 0
0x00,0x44,0x42,0x7E,0x40,0x40,0x00,0x00,  // 1
0x00,0x64,0x52,0x52,0x52,0x52,0x4C,0x00,  // 2
0x00,0x24,0x42,0x42,0x4A,0x4A,0x34,0x00,  // 3
0x00,0x30,0x28,0x24,0x7E,0x20,0x20,0x00,  // 4
0x00,0x2E,0x4A,0x4A,0x4A,0x4A,0x32,0x00,  // 5
0x00,0x3C,0x4A,0x4A,0x4A,0x4A,0x30,0x00,  // 6
0x00,0x02,0x02,0x62,0x12,0x0A,0x06,0x00,  // 7
0x00,0x34,0x4A,0x4A,0x4A,0x4A,0x34,0x00,  // 8
0x00,0x0C,0x52,0x52,0x52,0x52,0x3C,0x00,  // 9
0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00,  // :
0x00,0x00,0x00,0x80,0x64,0x00,0x00,0x00,  // ;
0x00,0x00,0x00,0x10,0x28,0x44,0x00,0x00,  // <
0x00,0x00,0x28,0x28,0x28,0x28,0x28,0x00,  // =
0x00,0x00,0x00,0x44,0x28,0x10,0x00,0x00,  // >
0x00,0x04,0x02,0x02,0x52,0x0A,0x04,0x00,  // ?

0x00,0x3C,0x42,0x5A,0x56,0x5A,0x1C,0x00,  // @
0x00,0x7C,0x12,0x12,0x12,0x12,0x7C,0x00,  // A
0x00,0x7E,0x4A,0x4A,0x4A,0x4A,0x34,0x00,  // B
0x00,0x3C,0x42,0x42,0x42,0x42,0x24,0x00,  // C
0x00,0x7E,0x42,0x42,0x42,0x24,0x18,0x00,  // D
0x00,0x7E,0x4A,0x4A,0x4A,0x4A,0x42,0x00,  // E
0x00,0x7E,0x0A,0x0A,0x0A,0x0A,0x02,0x00,  // F
0x00,0x3C,0x42,0x42,0x52,0x52,0x34,0x00,  // G
0x00,0x7E,0x08,0x08,0x08,0x08,0x7E,0x00,  // H
0x00,0x00,0x42,0x42,0x7E,0x42,0x42,0x00,  // I
0x00,0x30,0x40,0x40,0x40,0x40,0x3E,0x00,  // J
0x00,0x7E,0x08,0x08,0x14,0x22,0x40,0x00,  // K
0x00,0x7E,0x40,0x40,0x40,0x40,0x40,0x00,  // L
0x00,0x7E,0x04,0x08,0x08,0x04,0x7E,0x00,  // M
0x00,0x7E,0x04,0x08,0x10,0x20,0x7E,0x00,  // N
0x00,0x3C,0x42,0x42,0x42,0x42,0x3C,0x00,  // O

0x00,0x7E,0x12,0x12,0x12,0x12,0x0C,0x00,  // P
0x00,0x3C,0x42,0x52,0x62,0x42,0x3C,0x00,  // Q
0x00,0x7E,0x12,0x12,0x12,0x32,0x4C,0x00,  // R
0x00,0x24,0x4A,0x4A,0x4A,0x4A,0x30,0x00,  // S
0x02,0x02,0x02,0x7E,0x02,0x02,0x02,0x00,  // T
0x00,0x3E,0x40,0x40,0x40,0x40,0x3E,0x00,  // U
0x00,0x1E,0x20,0x40,0x40,0x20,0x1E,0x00,  // V
0x00,0x3E,0x40,0x20,0x20,0x40,0x3E,0x00,  // W
0x00,0x42,0x24,0x18,0x18,0x24,0x42,0x00,  // X
0x02,0x04,0x08,0x70,0x08,0x04,0x02,0x00,  // Y
0x00,0x42,0x62,0x52,0x4A,0x46,0x42,0x00,  // Z
0x00,0x00,0x00,0x7E,0x42,0x42,0x00,0x00,  // [
0x00,0x00,0x04,0x08,0x10,0x20,0x40,0x00,  // <backslash>
0x00,0x00,0x00,0x42,0x42,0x7E,0x00,0x00,  // ]
0x00,0x00,0x08,0x04,0x7E,0x04,0x08,0x00,  // ^
0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,  // _

0x3C,0x42,0x99,0xA5,0xA5,0x81,0x42,0x3C,  // `
0x00,0x00,0x20,0x54,0x54,0x54,0x78,0x00,  // a
0x00,0x00,0x7E,0x48,0x48,0x48,0x30,0x00,  // b
0x00,0x00,0x00,0x38,0x44,0x44,0x44,0x00,  // c
0x00,0x00,0x30,0x48,0x48,0x48,0x7E,0x00,  // d
0x00,0x00,0x38,0x54,0x54,0x54,0x48,0x00,  // e
0x00,0x00,0x00,0x00,0x7C,0x0A,0x02,0x00,  // f
0x00,0x18,0xA4,0xA4,0xA4,0xA4,0x7C,0x00,  // g
0x00,0x00,0x7E,0x08,0x08,0x08,0x70,0x00,  // h
0x00,0x00,0x00,0x00,0x48,0x7A,0x40,0x00,  // i
0x00,0x00,0x00,0x40,0x80,0x80,0x7A,0x00,  // j
0x00,0x00,0x7E,0x18,0x24,0x40,0x00,0x00,  // k
0x00,0x00,0x00,0x00,0x3E,0x40,0x40,0x00,  // l
0x00,0x00,0x7C,0x04,0x78,0x04,0x78,0x00,  // m
0x00,0x00,0x7C,0x04,0x04,0x04,0x78,0x00,  // n
0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,  // o

0x00,0x00,0xFC,0x24,0x24,0x24,0x18,0x00,  // p
0x00,0x18,0x24,0x24,0x24,0xFC,0x80,0x00,  // q
0x00,0x00,0x00,0x78,0x04,0x04,0x04,0x00,  // r
0x00,0x00,0x48,0x54,0x54,0x54,0x20,0x00,  // s
0x00,0x00,0x00,0x04,0x3E,0x44,0x40,0x00,  // t
0x00,0x00,0x3C,0x40,0x40,0x40,0x3C,0x00,  // u
0x00,0x00,0x0C,0x30,0x40,0x30,0x0C,0x00,  // v
0x00,0x00,0x3C,0x40,0x38,0x40,0x3C,0x00,  // w
0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00,  // x
0x00,0x00,0x1C,0xA0,0xA0,0xA0,0x7C,0x00,  // y
0x00,0x00,0x44,0x64,0x54,0x4C,0x44,0x00,  // z
0x00,0x00,0x08,0x08,0x76,0x42,0x42,0x00,  // {
0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,  // |
0x00,0x00,0x42,0x42,0x76,0x08,0x08,0x00,  // }
0x00,0x00,0x00,0x04,0x02,0x04,0x02,0x00,  // ~

//Bonus non-ascii 8x8 symbols:
//these can be commented out if you want to save space.
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  //127 top bar  
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,  //128 right bar
0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //129 left bar
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  //130 solid block
0xA9, 0x29, 0xC9, 0x12, 0xE2, 0x04, 0x18, 0xE0,  //131 radio
0x0C, 0x1E, 0x3E, 0x7E, 0x7E, 0x3E, 0x1E, 0x0C,  //132 wifi
0x00, 0xFE, 0xFE, 0xFF, 0xFF, 0xFE, 0xFE, 0x00,  //133 batery full
0x00, 0xFE, 0x82, 0x83, 0x83, 0x82, 0xFE, 0x00,  //134 battery empty
0x00, 0xFE, 0x82, 0xF3, 0x9F, 0x82, 0xFE, 0x00,  //135 battery charging
0x20, 0x30, 0x38, 0x3C, 0x3C, 0x38, 0x30, 0x20,  //136 arrow up
0x00, 0x00, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,  //137 arrow right
0x04, 0x0C, 0x1C, 0x3C, 0x3C, 0x1C, 0x0C, 0x04,  //138 arrow down
0x00, 0x00, 0x18, 0x3C, 0x7E, 0xFF, 0x00, 0x00,  //139 arrow left
0x10, 0x60, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01,  //140 square root
0x10, 0x10, 0x10, 0x54, 0x54, 0x10, 0x10, 0x10,  //141 division
0x12, 0x19, 0x15, 0x12, 0x00, 0x00, 0x00, 0x00,  //142 squared
0x11, 0x15, 0x15, 0x0A, 0x00, 0x00, 0x00, 0x00,  //143 cubed
0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00,  //144 to x
0x80, 0x80, 0x40, 0x30, 0x0C, 0x02, 0x01, 0x01,  //145 integral
0x3C, 0x42, 0x89, 0xA1, 0xA1, 0x89, 0x42, 0x3C,  //146 smiley
0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C,  //147 circle
0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C,  //148 filled circle
0x18, 0x66, 0x5A, 0xA5, 0xA5, 0x5A, 0x66, 0x18   //149 gear
};

