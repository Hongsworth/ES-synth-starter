# Main Menu Functionality

Upon startup, the Synth screen will display an initial graphical animation and then a menu with four main  options: Solo, Receiver, Transmitter, and octave change. The octave of the board can be pre specified using the joystick. We select which mode we would like to enter using the joystick.

### Method of Implementation
We implemented a while loop within the setup() function which uses if-statements to decide which mode is to be entered. There is an arrow which indicates which option we are currently at and the joystick controls navigate it. The selection is then confirmed on the screen.
