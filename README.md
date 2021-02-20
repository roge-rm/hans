# hans
hans is a 5 button battery powered USB/MIDI footswitch

<img src=https://raw.githubusercontent.com/hunked/hans/main/images/front.jpg width=600><br>
<img src=https://raw.githubusercontent.com/hunked/hans/main/images/back.jpg width=300><img src=https://raw.githubusercontent.com/hunked/hans/main/images/inside.jpg width=300>

This is a continuation of a couple of previous projects of mine [1](https://github.com/hunked/eightbuttonMIDIfootswitch)[2](https://github.com/hunked/footie). My goal is to make a durable, 3D printed, battery powered, MIDI footswitch that outputs over USB and 5 pin MIDI that can be assembled from cheap parts available to anyone. Why did I name it Hans? No good reason at all.

Hans has the following features:
- 5 momentary foot switches with status LEDs
- 3 banks of buttons that can be switched between using the far left/right buttons (for a total of 15 assignable functions)
- 1 external footswitch input (with no function.. at the moment)
- USB-C charging with an 18650 on board for portable power (this is so that it can be used with only a MIDI cable)
- [Teensy 2.0++](https://www.pjrc.com/store/teensypp.html) brains running at a blazing 16Mhz
- a 4 character screen

You can hook Hans up with a mini-USB (it shows up as a class-compliant MIDI device) or a standard MIDI cable and control whatever you damn well please. 

Currently only MIDI note messages are able to be sent but soon additional modes will be added to allow for sending of MIDI CC (control change) or PC (program change) messages.

More to come.
