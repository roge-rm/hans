# hans
### Hans is a 5 button battery powered USB/MIDI footswitch<br>

Check Hans out on: [Youtube](https://www.youtube.com/watch?v=nbIn7SEke0U) | [Thingiverse](https://www.thingiverse.com/thing:4768840)

<img src=https://raw.githubusercontent.com/hunked/hans/main/images/front.jpg width=600><br>
<img src=https://raw.githubusercontent.com/hunked/hans/main/images/back.jpg width=300><img src=https://raw.githubusercontent.com/hunked/hans/main/images/inside.jpg width=300><br>

This is a continuation of a couple of previous projects of mine [[1]](https://github.com/hunked/eightbuttonMIDIfootswitch)[[2]](https://github.com/hunked/footie). 

My goal is to make a durable, 3D printed, battery powered, MIDI footswitch that outputs over USB and 5 pin MIDI. <br>
For cheap.<br>

Why did I name it Hans? No good reason at all.

### Hans has the following hardware/features:
- 5 momentary foot switches with status LEDs
- 1 external footswitch input for acting as a "shift" button (with small backup shift button on the back in case an external switch is not attached)
- Up to 5 virtual "banks" of buttons that can be switched between (3 enabled by default but can be changed in the edit menu)
- Full editing of all footswitch functions from the switch itself - you don't need to use your computer at all to change any settings. 
- Changes can be saved to EEPROM and will be loaded automatically on startup.
- USB-C charging with an 18650 on board for portable power
- USB (mini) and 5-pin MIDI cable connections for MIDI data
- [Teensy 2.0++](https://www.pjrc.com/store/teensypp.html) brains running at a blazing 16Mhz
- a 4 character screen

When Hans is turned on you can choose between four modes: 
- send MIDI note on/off messages 
- send MIDI control change (CC) messages
- send MIDI program change (PC) messages
- an edit mode to customize all features of the footswitch

The first three modes are self-explanatory. Both the note on/off and CC modes can be toggled between momentary and toggle operations (done by holding shift + pressing button 2).
The edit mode allows changing most of the settings on the device. You can change the midi notes sent (and the velocity sent with them), the control change numbers and values, and program change numbers. You can also edit the default mode (in case you want to start in CC or PC mode), the timeout before the default mode is selected, and various other settings like screen and LED brightness.

### How to use Hans:
- Turn Hans on.
- You will be presented with a mode selection screen. Press a button to choose a mode. <br>If no mode choice is made the default will be chosen after a timeout (5 seconds).
  * Button 1 - Note On/Off
  * Button 2 - Control Change
  * Button 3 - Program Change
  * Button 5 - Edit Mode
- If you selected modes 1 through 3:
  * Press the footswitch buttons to send the note/CC/PC message associated with that button.
  * Hold the shift button and press one of the front switches for additional functions:
    * Button 1/5 - Change button bank (-/+)
    * Button 2 - Switch between momentary/toggle modes
    * Button 3 - Return to mode select menu
    * Button 4 - Panic! Stop all playing notes
- If you selected edit mode:
  * Navigate through menus/submenus using the following buttons:
    * Button 1 - Go back to previous menu
    * Button 2 - Previous item (in menu) OR Reduce value by 1 (when editing)
    * Button 3 - Select item (in menu) or accept value (when editing)
    * Button 4 - Next item (in menu) or Increase value by 1 (when editing)
  * **Changes will not be saved to EEPROM (to be loaded on next startup) unless you select SAVE from the either the main menu or under the DATA submenu.** If you make a change you do not like you can either restart the unit, load the settings from EEPROM (under DATA submenu), or load the factory defaults (also under DATA submenu).

### Pinout:<br>
<img src=https://raw.githubusercontent.com/hunked/hans/main/images/pinout.png>

### Parts list:
| Name                                                          | Quantity |
|---------------------------------------------------------------|----------|
| [Toggle Switch 2 Position 6 Pins](https://www.aliexpress.com/item/32800053774.html)            | 1        |
| [SPST Momentary Soft Touch Push Button Stomp Foot Pedal Switch](https://www.aliexpress.com/item/32918205335.html) | 5        |
| [6.35mm Mono Female Jack With Switch Socket Panel Adapter](https://www.aliexpress.com/item/32614113278.html)      | 1        |
| [5 Pin Panel Mount Female Jack DIN Adapter MIDI Connector](https://www.aliexpress.com/item/32972269819.html)      | 1        |
| [7mm Momentary Push button Switch](https://www.aliexpress.com/item/32790920961.html)                              | 2        |
| [TM1637 4 Bits Digital LED Display Module](https://www.aliexpress.com/item/32387190376.html)                      | 1        |
| [Flat Top 5V 5mm Red LED Lamp w/integrated resistor](https://www.aliexpress.com/item/32904653006.html)            | 5        |
| [Type-C 5V 1A 18650 Lithium Battery Charger Module](https://www.aliexpress.com/item/32670803042.html)             | 1        |
| [Battery Spring and Plate Set](https://www.aliexpress.com/item/32881285245.html)                                  | 1        |
| [Teensy 2.0++ USB AVR Development Board](https://www.pjrc.com/store/teensypp.html) or [clone](https://www.aliexpress.com/item/32975352827.html)          | 1        |
| [1000uF+ 6.3V+ Capacitor](https://www.aliexpress.com/item/4000092084550.html)                                       | 1        |
| [220 Ohm Resistor 1/4W](https://www.aliexpress.com/item/32952657927.html)                                         | 2        |
| [M3 Thread Knurled Brass Threaded Heat Set](https://www.aliexpress.com/item/4001185849382.html)                     | 10       |
| [M3x10 Stainless Countersunk Hex Head Bolt](https://www.aliexpress.com/item/4000020967604.html)                     | 10       |
| [M2x6mm Stainless Countersunk Self-Tapping Screw](https://www.aliexpress.com/item/4001074924245.html)               | 6        |
| [M2x10mm Stainless Hex Head Bolt](https://www.aliexpress.com/item/32810852732.html)                               | 2        |
| [M2 Stainless Nut](https://www.aliexpress.com/item/32977174437.html)                                              | 2        |

### Purchasing a Hans:

[Please see this page for purchasing info.](https://github.com/hunked/hans/blob/main/PURCHASE.md/)
