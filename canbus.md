# CAN Bus Functionality

The CAN Bus allows for multiple synths to be connected together and communicate across devices. This is used in our synth in multiple ways, for example the Transmitter and Receiver modes which allow for seamless transmission of data along the different synths that have been connected together. 

## Main Implementation
CAN bus makes use of a uint8_t array of size 12. Different values needed are sent via this array. If a button is pressed it sends a char (P or R) to indicate press or release. 

Full breakdown: 0 is press or release
                1 is octave for the reciever to correctly play the required note
                2 is the note number (0 - 11 on the keyboard.)
                
                
If the board is configured as a sender it will send values on the CAN bus, whereas if it is a reciever it reads instead. For the special case of a solo board, the CAN_Init function gets a true argument passed to allow for looping. 
