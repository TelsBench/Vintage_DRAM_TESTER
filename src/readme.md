#DRAM TEST OVERVIEW 

Author : R. T. Burns
Date : 19th march 2023

#####Shown in the picture is an Arduino Nano AT328.

This project extends, refactors and builds upon an original effort , see original and new licence for this project. This is built on an Arduino Nano but should work for most Arduino's provided the pins are available in the number required.

![](../images/DRAM_Tester_Breadboard.jpg 'Breadboard Layout') 

![](../images/Schematic_Vintage%20DRAM%20Tester%20-%2016%20Pin%20DIP_2023-04-07.png 'Schematic-Wiring Diagram')

 

**LED'S**
WHITE RUN ( Looks Yellow on image for some reason )
RED   FAIL
GREEN PASS

**OLED Display I2C Interface**
128x32 ( its quite small but good enough for this simple project)



This Code and Hardware Supports Retro DRAM Chips of the same 16 pin DIL format supply voltages and pinouts BITD, example below.

####Examples of this DRAM format in 16pin DIP
Notice they all have the same pinout. But smaller storage capacity have `Not Connected` pins used by larger capacity DRAM's and they are marked with NC for not connected. The maximum size for this format is 266,144x1  (1/4 Megga bits).

Some DRAM's may use other voltages than 5v and Ground but this tester project cocentrates on those who only use 5v and 0v, so be aware and please check that the DRAM you will put into this configuration is applicable or damage could occur to an otherwise good chip potentially. VCC means 5v in the diagram below in the context of what we are doing here.  The Arduino Nano has a Mini-B usb connector and this is where it gets its power in this scenario, and the power for the components on the breadboard will take its power directly from the Nano power pins out +5v and GND.

![ ](../images/DRAM_Examples.png 'A Standard DRAM  16Pin Format BITD')


Featuers In this project

1.) OLED display, more detailed feeback than just LED's'
2.) Removal of existing Random Test method.
3.) Replaced with Marching One's and then Marching Zeros.
4.) Add Pushbutton Selector 7/8/9 Addr Bits - Used the OLED for this.
5.) Added a ReadMe(.)md ( this document ) with Graphics etc for detailed info.

#Switching Number of ADDR Bits

An onboard switch selects either 9 Bits ( 256Kx1 ) A0-A8 or anything lower Which would be 7,8 and 8 Bits  so 16kx1/64kx1 , there may also be variants for 4k, but I'm not sure about that.

The ADDR_LENGTH  ( Number of address bits ) is stored in the EEPROM, when first programed it will write the Default '8' and write to the EEprom. However this can be changed by Either poweing up the arduino or pressing and holding the reset button until you get 'Release Button' Message come up, then repeatedly press the Button and find the length you want. To register that value, save and continue just HOLD DOWN the button for a couple of seconds.

#Other Notes
I used a breadboard and 22gauge wiring and three LEDS + an OLED 128x32 Display. The Nano has a reset switch as to most Arduino's so that's handy. I was able to plug the chips for testing right into the breadboard, no ZIF sockets were used on this build, but it would be a great idea to do.


**Chips Tested on this so far are**
1. TMS4164   64k x1
2. M5M4256P 256k x1
3. MB8118    16k x1
4. TMS4816   16k x1

This is not at the maximum speed they would be run at and does not push the limits shown in the specs, but should provide a good deal of confidence in that the chips is at least able to address, write and read the contents of the DRAM.

During the work I tested my entire collection and ended up with a small pile of 
failures ( Happy or Sad ? - I choose Happy it would have save me time at some later point)












