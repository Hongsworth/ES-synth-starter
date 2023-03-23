# Octaves

## Main Implementation

We implemented the ability to switch between octaves 2 to 6 on the synth. The implementation involved storing the exact frequencies of each octave's keys into separate arrays; it is very easy to switch between the five modes and play different key frequencies. The octave is selected using a set of if statements to decide which of the 5 octave arrays it needs to index. The relevant octave frequencies are then heard on the keyboard.
