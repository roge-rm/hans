/*
   5 buttons and an external switch walk into a bar.
*/

#include <TM1637TinyDisplay.h>
#include <MIDI.h>
#include <OneButton.h>
//#include <EEPROM.h>

elapsedMillis ledCount;
elapsedMillis screenWipe;

// create MIDI instance for 5 pin MIDI output
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#define buildRev "20210218"

// PIN assignments
// for the display
const int wipeTime = 500; // max time to display anything on the screen
const int CLK_PIN = 0;
const int DIO_PIN = 1;

// for buttons and external switch (acts as an extra button)
const byte buttonPin[6] = {10, 11, 12, 13, 14, 17};
// for LEDs
const byte ledPin[5] = {28, 29, 30, 31, 32};

// initialize buttons with OneButton
OneButton button1(buttonPin[0], true);
OneButton button2(buttonPin[1], true);
OneButton button3(buttonPin[2], true);
OneButton button4(buttonPin[3], true);
OneButton button5(buttonPin[4], true);
OneButton extswitch(buttonPin[5], true);

// supports up to five Banks of switches - 3 enabled by default
int switchBank = 0; // currently selected Bank of switches
int numBanks = 3;

// default switch notes - edit this later when editing/EEPROM storage is added
int switchNotes[5][5] = {{60, 61, 62, 63, 64}, {65, 66, 67, 68, 69}, {70, 71, 72, 73, 74}, {75, 76, 77, 78, 79}, {80, 81, 82, 83, 84}};

int midiChan[3] = {16, 16, 16}; // note, CC, PC MIDI channel


/* shift mode is triggered by the external switch input - this either add or subtract
    one from the bank number while the switch is held down to give quick access to extra
    buttons/functions
*/
bool shift = false;

// use mode setting to switch between note, cc, pc, edit modes (4 states)
// hold buttons 1, 2, 3, or 5 on startup to select mode
// alternatively show a mode select on bootup
// long press button 3 any time to return to mode select
int mode = 0;
int defaultMode = 0;

// LED status for tracking/enabling LEDs when switching between banks
bool ledStatus[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
int ledFlag[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
int ledInterval = 50; // time to decrease LED flash flag count

// track switch states for when toggling is enabled
bool switchStates[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};

// initialize display
TM1637TinyDisplay display(CLK_PIN, DIO_PIN);

void setup() {

  // Display setup and greeting
  display.setBrightness(0x01);
  display.setScrolldelay(100);
  display.showString("hans");
  delay(500);
  //display.showString(buildRev);
  display.showString("Ready");
  delay(250);
  display.clear();

  // initialize MIDI
  MIDI.begin();

  // initialize LEDs
  pinMode(ledPin[0], OUTPUT);
  pinMode(ledPin[1], OUTPUT);
  pinMode(ledPin[2], OUTPUT);
  pinMode(ledPin[3], OUTPUT);
  pinMode(ledPin[4], OUTPUT);

  // initialize button 1 functions
  button1.attachClick(click1);
  button1.attachLongPressStart(longPressStart1);

  // initialize button 2 functions
  button2.attachClick(click2);

  // initialize button 2 functions
  button3.attachClick(click3);

  // initialize button 2 functions
  button4.attachClick(click4);

  // initialize button 1 functions
  button5.attachClick(click5);
  button5.attachLongPressStart(longPressStart5);

  // initialize external button functions
  // single click will flip the shift state
  // holding will flip the shift state while held then flip back when released
  extswitch.attachClick(clickExt);
  extswitch.attachLongPressStart(longPressStartExt);
  extswitch.attachLongPressStop(longPressStopExt);

  // this is the part where the mode select will happen
  // holding buttons 1, 2, 3 while starting will select note, cc, pc modes
  // holding button 5 will start edit mode
  // holding nothing will start the default mode

}

void loop() {
  // Update LEDs
  ledUpdate();

  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
  button1.tick();
  button2.tick();
  button3.tick();
  button4.tick();
  button5.tick();
  extswitch.tick();

  // Ignore incoming MIDI
  while (usbMIDI.read())
  {
  }
}

// button press functions
void click1() {
  display.showNumber(switchNotes[switchBank][0]);
  screenWipe = 0;
  ledFlag[switchBank][0] = 5;
  usbMIDI.sendNoteOn(switchNotes[switchBank][0], 127, midiChan[0]);
  MIDI.sendNoteOn(switchNotes[switchBank][0], 127, midiChan[0]);
  usbMIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
  MIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
}

void longPressStart1() {
  if (switchBank > 0) switchBank--;
  else if (switchBank == 0) switchBank = numBanks - 1;
  displayBank();
}

void click2() {
  display.showNumber(switchNotes[switchBank][1]);
  screenWipe = 0;
  ledFlag[switchBank][1] = 5;
  usbMIDI.sendNoteOn(switchNotes[switchBank][1], 127, midiChan[0]);
  MIDI.sendNoteOn(switchNotes[switchBank][1], 127, midiChan[0]);
  usbMIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
  MIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
}

void click3() {
  display.showNumber(switchNotes[switchBank][2]);
  screenWipe = 0;
  ledFlag[switchBank][2] = 5;
  usbMIDI.sendNoteOn(switchNotes[switchBank][2], 127, midiChan[0]);
  MIDI.sendNoteOn(switchNotes[switchBank][2], 127, midiChan[0]);
  usbMIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
  MIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
}

void click4() {
  display.showNumber(switchNotes[switchBank][3]);
  screenWipe = 0;
  ledFlag[switchBank][3] = 5;
  usbMIDI.sendNoteOn(switchNotes[switchBank][3], 127, midiChan[0]);
  MIDI.sendNoteOn(switchNotes[switchBank][3], 127, midiChan[0]);
  usbMIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
  MIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
}

void click5() {
  display.showNumber(switchNotes[switchBank][4]);
  screenWipe = 0;
  ledFlag[switchBank][4] = 5;
  usbMIDI.sendNoteOn(switchNotes[switchBank][4], 127, midiChan[0]);
  MIDI.sendNoteOn(switchNotes[switchBank][4], 127, midiChan[0]);
  usbMIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
  MIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
}

void longPressStart5() {
  if (switchBank < (numBanks - 1)) switchBank++;
  else if (switchBank == (numBanks - 1)) switchBank = 0;
  displayBank();
}

//not sure how to implement external button yet
//perhaps use shift mode to access different other modes eg note/cc/pc/edit mode without having to turn the device off/on
void clickExt() {
  shift = !shift;
  display.showString("Shift");
  screenWipe = 0;
}

void longPressStartExt () {
  shift = !shift;
  display.showString("Shift");
  screenWipe = 0;
}

void longPressStopExt () {
  shift = !shift;
  display.showString("Shift");
  screenWipe = 0;
}


// update LED status
void ledUpdate() {
  // check for any pending LED flash flags - this is used to flash an LED for a period of time
  // afer a button has been pressed
  for (int i = 0; i < 5; i++) { // if an LED flash flag is present, set the LED to light
    if (ledFlag[switchBank][i] > 0) {
      ledStatus[switchBank][i] = 1;
    } else ledStatus[switchBank][i] = 0;
  }
  if (ledCount >= ledInterval) { // reduce any pending LED flash flags every ledInterval
    for (int i = 0; i < 5; i++) {
      if (ledFlag[switchBank][i] > 0) {
        ledFlag[switchBank][i] --;
      }
    }
    ledCount = 0;
  }


  for (int i = 0; i < 5; i++) {
    if (ledStatus[switchBank][i] == 1) {
      digitalWrite(ledPin[i], HIGH);
    }
    else digitalWrite(ledPin[i], LOW);
  }

  if (screenWipe >= wipeTime) {
    display.clear();
    screenWipe = 0;
  }
}

void ledReset() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      ledFlag[i][j] = 0;
      ledStatus[i][j] = 0;
    }
  }
}

// flash LEDs a certain number of times (eg to confirm button Bank selection)
void ledFlash(int numLEDs, int numFlashes) {
  for (int h = 0; h < numFlashes; h++) {
    for (int i = 0; i < numLEDs; i++) {
      digitalWrite(ledPin[i], HIGH);
    }
    delay(100);
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPin[i], LOW);
    }
    delay(100);
  }
}

void displayBank() {
  ledReset();
  display.showString("Bank");
  delay(250);
  display.showNumber(switchBank + 1);
  delay(250);
  display.clear();
  ledFlash(switchBank + 1, 2);
}
