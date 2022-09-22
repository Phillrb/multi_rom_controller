# ATTiny85 Multi-ROM Controller

Perform bank-switching on target EPROMs with an ATTiny85.

* Control 8 / 16 banks: 0x000 to 0b111 / 0b1111
* Change bank with button press (active LOW)
* Reset a target PCB 
* Non-volatile storage tracks current bank between power-cycles 

```
       ATTINY85
         _  _
 (A3*) -| \/ |- VCC
ACTION -|    |- A2  
 RESET -|    |- A1
   GND -|____|- A0
```
*Enabling A3 will increase control to 16 banks but limit reprogramming ATTiny85 to devices that can set fuse bits (TL866 or similar)

## Setup

Connect RESET (pin 3) to target device reset line. This will enable the ATTiny85 to pulse the reset line when it has switched banks.

Connect ACTION (pin 2) to external button. On momentarily pulling the line LOW, the ATTiny85 will increment the bank number. This line is already debounced.

Connect address lines (A0 to A3 - pins 5,6,7 & 1) to target EPROMs' upper address lines to perform bank-switching on those EPROMs. 

## Enabling A3 (Pin 1)

Set 'ENABLE_A3' to true