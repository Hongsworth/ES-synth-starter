# ES-synth-starter

  Use this project as the starting point for your Embedded Systems labs and coursework.
  
  [Lab Part 1](doc/LabPart1.md)
  
  [Lab Part 2](doc/LabPart2.md)

## Additional Information
  [Handshaking and auto-detection](doc/handshaking.md)



# Angelo's Branch: Notes: 

22/02/23: 

Lab1 Part 2 is a simple setup exercise that was completed on 22/02/23.

General principle is reading the keyboard presses. What has been understood is the following: 

Using a cycle for read pins RA0, RA1 and RA2, we can check for key presses on the C0-3 columns.
C0 for example is connected to multiple keys on the keyboard, but the specified keypress value will
only go high when the correct read pins are enabled. 

For keys C - D#, read pins must all be low for example. This allows for the detection of keypresses on those. 

Cycling through readstates 0-2 (RA0-2 will be represented in binary form in ascending order from 000, 001, 010)
allows for the detection of many keystates sequentially with minimal delays. Delays are needed to stop interference 
as discussed in the Spec. 


