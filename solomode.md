# Solo Mode

Upon startup, the synth provides the ability to choose the mode 'Solo Mode' using the joystick. It allows the user to operate a single synth on its own with full functionality and ability to switch octaves.

The addition of a solo option to a keyboard required careful consideration of a number of factors. One important consideration was the hardware of the keyboard itself. The keyboard must be able to function independently, without relying on other devices for data transmission. This requires the inclusion of additional hardware, such as a microcontroller, to enable the keyboard to operate independently.

## Main Implementation
We implemented solo mode by repurposing the CAN bus to be initialized to loop onto itself. This allows it to send messages to itself so that it can function on its own as an independent synth.
