//
//Statorworks 2020
//Arduino example for SWI2C board servo control
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
//Requirements:
//-I2CServo.ino, I2CWrapper.ino, OledSSD1306_Nano.ino on the sketch folder
//-Select your board on the i2CWrapper.ino file
//-If your Arduino board runs on 5V, don't forget to put an i2c level 
// shifter or repeater to interface the servo with 3.3V

#define LED_PIN 13
byte data[16];  
byte i2c_address = 0x58; //modify address as needed here. servo default is 0x00
bool result;

byte count=0;

void setup(){

 //Heartbeat LED
 pinMode(LED_PIN, OUTPUT); 
 digitalWrite(LED_PIN, HIGH);

 //I2C
 I2CWrapper_Begin();  //please select your board type in I2CWrapper.ino

 //console
 Serial.begin(9600); //Nano
 //SoftSerial.begin(9600);//Digispark (requires special wiring)
 
 //Alternatively, show data on small 0.93" i2c Oled display. 
 //This is an easier option for the Digispark board, and it also works with the others.
 //OledSSD1306_Setup(1, 0, 0, 1, 0x7F, 0, 1, 0); //display on, clear screen, charge pump on
 //OledSSD1306_PrintText("SERVO TEST", 64, 0, 1, 0, 1, 0, 0, 0);
 //OledSSD1306_PrintText("Pos:      ",  0, 2, 0, 0, 1, 0, 0, 0);
 //OledSSD1306_PrintText("Power:    ",  0, 4, 0, 0, 1, 0, 0, 0);
 //OledSSD1306_PrintText("Temp:     ",  0, 6, 0, 0, 1, 0, 0, 0);

 //Servo
 I2CServo_Begin();

}


void loop(){

 //Flash heartbeat LED
 digitalWrite(LED_PIN, HIGH);//  //PORTB |= 1<<LED_PIN;
 delay(20); 
 digitalWrite(LED_PIN, LOW);//PORTB &= ~(1<<LED_PIN);
 delay(20);
    

 //----------------------------------------
 //EXAMPLE #1
 //Read I2C address just to confirm communication. 
 
 result = I2CServo_GetI2cAddress(i2c_address, &data[0],0);
 if(result==1){ 
  Serial.println("Responded Ok");
  Serial.println(data[0]);
  Serial.println("\n");
  //OledSSD1306_PrintText("Responded Ok    ",  0, 2, 0, 0, 1, 0, 0, 0); 
 }
 else{ 
  Serial.println("Did not respond ");
  //OledSSD1306_PrintText("Did not respond ",  0, 2, 0, 0, 1, 0, 0, 0); 
 }
 delay(1000);
 
 
 //----------------------------------------
 //EXAMPLE #2
 //Change i2c address. Change is immediate and address is saved to servo's non-volatile memory.
 //Further access needs to be to the new address.
 //Make sure only one servo is on i2c bus
 /*
  I2CServo_SetI2cAddress(0, 0x58,0);  //0-127
  i2c_address = 0x58;
  delay(1000); 
 */
 
 //----------------------------------------
 //EXAMPLE #3
 //Scan i2c address space in case you forgot the servo's address.
 //Make sure only one servo is on i2c bus
 /*
 for(byte i =0; i<=127;i++){
   if(I2CServo_GetI2cAddress(i, &data[0], 0)==1){
     Serial.print(i);
     Serial.print(" responded.\n");
     //OledSSD1306_PrintValue(i, 0, 2, 0, 0, 1, 0, 0, 0, 3, 2);
     //OledSSD1306_PrintText(" responded.      ",  24, 2, 0, 0, 1, 0, 0, 0); 
     break;
   }
  }
 delay(2000);  
 */
 
 //----------------------------------------
 //EXAMPLE #4
 //Stop motion immediately in current position without ramping, and enter park state.
 /*
 I2CServo_Stop(i2c_address,0);
 delay(1000);
 */
 
 //----------------------------------------
 //EXAMPLE #5
 //Set running parameter(s)
 //User can set one item at a time or all simultaneously, see available functions.
 //I2CServo_SetParkType(i2c_address, 0); //0=coast, 1= brake, 2=hold with MAX_POWER setting, 3-100=hold with this number as power.
 //I2CServo_SetMaxPower(i2c_address, 50, 0); //0-100% cap
 //I2CServo_SetDeadband(i2c_address, 12, 0);
 //I2CServo_Setup(i2c_address, 2, 1, 0, 100, 1200, 1000, 80, 0); //address, park_type, motor_polarity, continuous_rotation, max_power, max_speed, ramp_time ms, ramp_curve, use_crc
 //delay(1);

 //----------------------------------------
 //EXAMPLE #6
 //Simple motion, back and forth
 /*
 I2CServo_SetTargetPosition(i2c_address, 200, 0);
 delay(1000);
 
 I2CServo_SetTargetPosition(i2c_address, 900, 0);
 delay(1000);
 */

 //----------------------------------------
 //EXAMPLE #7
 //You can push new positions (up to 8) without waiting for current motion to end
 //Note that these get cancelled by a regular TARGET_POSITION write.
 //The power setting must be sufficient to overcome the load in order to reach each of the positions. 
 /*
 I2CServo_SetNextTargetPosition(i2c_address, 300, 0);
 delay(1);
 I2CServo_SetNextTargetPosition(i2c_address, 700, 0);
 delay(1);
 I2CServo_SetNextTargetPosition(i2c_address, 500, 0);
 delay(1);  
 delay(5000);
 */

 //----------------------------------------
 //EXAMPLE #8
 //Continuous Rotation
 //Servo free to rotate. Potentiometer fixed, or free rotating. Control speed only by power setting. ramp time slope is time to to 100%power in ms
 //If there is a free-rotating pot, you can still consult it's current position. 
 /*
 I2CServo_Setup(i2c_address, 1, 1, 2, 0, 0, 2000, 0, 0); //park_type, motor_polarity, multiturn_type(2), max_power, max_speed, ramp_time ms, ramp_curve, use_crc
 delay(2);
 //
 I2CServo_SetMaxPower(i2c_address, 50, 0);
 delay(5000);
 I2CServo_SetMaxPower(i2c_address, 0, 0);
 delay(5000); 
 I2CServo_SetMaxPower(i2c_address, -50, 0);
 delay(5000);
 I2CServo_SetMaxPower(i2c_address, 0, 0);
 delay(5000);
 */

 //----------------------------------------
 //EXAMPLE #9
 //Read status register(s)
 //user can get one item at a time or all simultaneously, see available functions.
 //You can move the servo by hand and observe the line on the serial plotter graph, or the value on the Oled display. 
 
 byte state, temp, lastcrc8;
 int8_t power;
 int16_t pos, vel;
 //i2c_address = 0x00;
 //i2cServo_GetCurrentTemp(i2c_address, &temp, 0);
 //i2cServo_GetCurrentPosition(i2c_address, &pos, 0);
 //i2cServo_GetAllStatus(i2c_address, &state, &pos, &vel, &power, &temp, &lastcrc8, 0);  //provide zero pointer for the items you don't want
 // 
 //Serial.print(pos);
 //Serial.print(" ");
 //Serial.print(vel); 
 //Serial.print(" ");
 //Serial.println(temp);//println=print these on IDE serial plotter
 //
 //values on the oled screen right side
 //OledSSD1306_PrintValue(pos,   127, 2, 2, 0, 1, 0, 0, 0, 5, 1);
 //OledSSD1306_PrintValue(power, 127, 4, 2, 0, 1, 0, 0, 0, 5, 1);
 //OledSSD1306_PrintValue(temp,  127, 6, 2, 0, 1, 0, 0, 0, 5, 1);
 //
 //delay(100);
 
 
 //----------------------------------------
 //EXAMPLE #10
 //Create stored program
 //User can set one item at a time
 //I2CServo_SetProgramSlot(i2c_address, 0, 300, 0);
 //delay(10);
 //I2CServo_SetProgramSlot(i2c_address, 1, 700, 0);
 //delay(10);
 //I2CServo_SetProgramSlot(i2c_address, 2, 500, 0);  
 //delay(10);
 //I2CServo_SetProgramSlot(i2c_address, 2,   0, 0); //0 marks end of programmed positions
 //delay(10);
 //I2CServo_SetProgramReps(i2c_address, 15, 0);      //255= infinite
 //
 //Or set up all at once:
 //int16_t pp[8] = { 300, 700, 500, 0, 0, 0, 0, 0 };
 //I2CServo_SetFullProgram(i2c_address, &pp[0], 5, 0);  //address, positions array, reps, use crc
 //delay(1);

 //I2CServo_SaveAll(i2c_address, 0);
 //delay(50000);
 
 //Options to stop the running program (choose one):
 //I2CServo_SetProgramReps(i2c_address, 0, 0);           //reps=0,1,2 etc: reduce the number of reps left
 //I2CServo_SetNextTargetPosition(i2c_address, 800, 0);  //program ends at completion of current move and servo goes to new defined position 
 //I2CServo_SetTargetPosition(i2c_address, 800, 0);      //program ends right away and servo ramps towards new defined position 
 //I2CServo_Stop(i2c_address, 0);                        //program and motion end immediately on current position without any ramp
 //delay(1000);

  
 //----------------------------------------
 //EXAMPLE #11
 //Save all registers. 
 //Current motion settings become new defaults, loaded on power up.
 //If PROGRAM_START register is set to option 2, the stored program motion will start on power up.
 //I2CServo_SaveAll(i2c_address, 0);
 //delay(5000);


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
 delay(1000);
  
 //Write, if checksum fails, servo discards the whole write transaction.
 I2CServo_SetMaxPower(i2c_address, 75, 1); 
 //
 //The servo stores the last succesful write crc, so you can consult that to double check.
 byte last_crc;
 i2cServo_GetLastcrc8(i2c_address, &last_crc, 1);//Note this read can itself be done using crc for integrity. 
 Serial.print(last_crc);
 //delay(1000);
 */
 
}//loop








