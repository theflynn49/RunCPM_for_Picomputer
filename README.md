
# RunCPM for Picomputer - Z80 CP/M emulator for Bobricius' Picomputer.
<br>
RunCPM was written by Marcelo Dantas, then ported to RaspberryPi Pico by GUIDO LEHWALDER.<br>
This is a port of the latter to Bobricius' Picomputer, for which I had to add the ST77XX Adafruit screen driver and my own keyboard and sound drivers.<br><br>
The original work can be found at these locations : 
<br>
<br>    (c) Marcelo Dantas https://github.com/MockbaTheBorg/RunCPM
<br>    (c) GUIDO LEHWALDER https://github.com/guidol70/RunCPM_RPi_Pico/tree/main/v6_7
<br>

## Some details

- The screen is 53x30 characters. As it is pretty slow and doesn't support effectively scrolling, I use the same trick that is used by PicoMite for such screens : erase the screen and go to the top when you reach the bottom.
- The last line is used to display the status of the fake-scrolling operations
- A minimal subset of VT100 codes is supported, just enough so TurboPascal 3.0 barely works, that is EEOL, CURPOS and colors (Esc[\<n\>m Esc[38;5;\<n\>m and Esc[48;5;\<n\>m) 
- You will find my TurboPascal parameter file in the support_files directory with a 'PICOMPUTER' choice in the screen section, if that helps.
- The CPU is overclocked, running at 250Mhz, giving you about 7 times the speed of a 4Mhz Z80.
<br>

## Keyboard

- SHIFT-DOWN : select LowerCase (then the shift key acts more like an ALT one, to select symbols)
- SHIFT-UP : select UpperCase (then the shift key acts more like an ALT one, to select symbols)
- SHIFT-RIGHT : select Symbols (then the shift key acts more like an CTRL one, to select control characters)
<br>

## Installation
<br>
Set your pico in boot mode, then copy the uf2 file (found in build/rp2040.rp2040.rpipico/) to the device.<br>
Note that if the firmware is already loaded, you can hold the "Q" button during reset to enter the boot mode.
<br>
You need to prepare a SD card with CP/M on it, found here : https://github.com/guidol70/RunCPM_RPi_Pico/tree/main/SDCard_content.zip
I suggest you add on that SD card some CP/M content you can find here : https://obsolescence.wixsite.com/obsolescence/multicomp-fpga-cpm-demo-disk (choice: Download Tool Set, not the Disk Image)
<br>
<br>Instruction for creating the SD card may be found here : https://github.com/guidol70/RunCPM_RPi_Pico/tree/main
