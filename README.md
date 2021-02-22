# hans
Hans is a 5 button battery powered USB/MIDI footswitch

<img src=https://raw.githubusercontent.com/hunked/hans/main/images/front.jpg width=600><br>
<img src=https://raw.githubusercontent.com/hunked/hans/main/images/back.jpg width=300><img src=https://raw.githubusercontent.com/hunked/hans/main/images/inside.jpg width=300>

This is a continuation of a couple of previous projects of mine [1](https://github.com/hunked/eightbuttonMIDIfootswitch)[2](https://github.com/hunked/footie). My goal is to make a durable, 3D printed, battery powered, MIDI footswitch that outputs over USB and 5 pin MIDI that can be assembled from cheap parts available to anyone. Why did I name it Hans? No good reason at all.

Hans has the following hardware/features:
- 5 momentary foot switches with status LEDs
- 1 external footswitch input for acting as a "shift" button (with small backup shift button on the back in case an external switch is not attached)
- 5 virtual "banks" of buttons that can be switched between while holding shift+the left most or right most button
- USB-C charging with an 18650 on board for portable power
- USB (mini) and 5-pin MIDI cable connections for MIDI data
- [Teensy 2.0++](https://www.pjrc.com/store/teensypp.html) brains running at a blazing 16Mhz
- a 4 character screen

Hans is capable of the following operations:
- Send MIDI Note (on & off), Control Change, or Program Change messages
- Switch between momentary or toggle operation

You can hook Hans up with a mini-USB (it shows up as a class-compliant MIDI device) or a standard MIDI cable and control whatever you damn well please. 

Parts list:
| Name                                                          | Quantity |
|---------------------------------------------------------------|----------|
| Micro Slide Switch 3PIN 2 Position 1P2T 5mm Handle            | 1        |
| SPST Momentary Soft Touch Push Button Stomp Foot Pedal Switch | 5        |
| 6.35mm Mono Female Jack With Switch Socket Panel Adapter      | 1        |
| 5 Pin Panel Mount Female Jack DIN Adapter MIDI Connector      | 1        |
| 7mm Momentary Push button Switch                              | 2        |
| TM1637 4 Bits Digital LED Display Module                      | 1        |
| Flat Top 5V 5mm Red LED Lamp w/integrated resistor            | 5        |
| Type-C 5V 1A 18650 Lithium Battery Charger Module             | 1        |
| Battery Spring and Plate Set                                  | 1        |
| Teensy 2.0++ USB AVR Development Board                        | 1        |
| 1000uF+ 6.3V+ Capacitor                                       | 1        |
| 220 Ohm Resistor 1/4W                                         | 2        |
| M3 Thread Knurled Brass Threaded Heat Set                     | 10       |
| M3x10 Stainless Countersunk Hex Head Bolt                     | 10       |
| M2x6mm Stainless Countersunk Self-Tapping Screw               | 6        |
| M2x10mm Stainless Hex Head Bolt                               | 2        |
| M2 Stainless Nut                                              | 2        |
