Here you can find the files to have the servo tester PCB made, as well as the source code if you want to improve or repurpose the board.

To power the board, provide 5-9v (5-6.5v recommended) on the XH connector.

Left hand pot controls power and ramp time (although they can be controlled independently on actual application)

Right hand pot controls position. Note that range might go past mechanical range of your particular servo, test carefully.

To program the servo address, set the binary value on the dip switch (Down=0, Up=1), then press the tactile switch for one second. The green Led will flash momentarily.

If you see the Red Led flash, it means there is no response to the I2C communication. Check the physical connection, the supply voltage, and the Dip switch value.

You can also test a regular servo with the three-pin terminal on the right side. A 1-2ms pulse is provided, controlled by the right hand pot as is usual.

The two-pin terminal expose the uC PB2 and PB3 lines for those who want to do something else with the board.

To reprogram the board, use the VPG terminals (Voltage sample, UPDI line, Ground) underneath with a J-link or Atmel ICE programmer. Note that you still have to supply power via the XH connector.


BOM:

-Schottky Diode SOD-123

-AP2210 3.3v regulator SOT-23

-Atmel Attiny 416 or 816

-Potentiomenters   DK# PTV09A-40-20U-B103-ND

-Dip Switch 7-pos  DK# GH7175-ND

-6mm tactile switch

-PH connector 4-position right angle

-XH connector 2-position right angle

-Resistors 33, 2.2K   0603 

-Caps 0.1u, 10u  0603

-Led green, red   0603



Have fun!!
