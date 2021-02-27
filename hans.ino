/*
   5 buttons and an external switch walk in to a bar.

   code is WIP combination of last project (footie) and current project
*/

#define buildRev "20210226"
bool intro = false; // set to false to disable intro name/build date

#include <Bounce2.h>
#include <MIDI.h>
#include <EEPROM.h>
#include <TM1637TinyDisplay.h>

// create MIDI instance for 5 pin MIDI output
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// for buttons and external switch (acts as an extra button)
const byte switchPin[7] = {38, 39, 40, 41, 42, 43, 44};

// output pin attached to normally-connected pin of TS port
// this is driven low (to simulate ground) for normally-closed external footswitches
// and driven high (to simulate open) for normally-open footswitches
// this is to avoid unwanted behaviour when the switch is not connected
const byte extDriver = 45;

// shift mode is enabled when external pedal switch is pushed down
bool shift = false;

// control whether button presses are momentary or toggle (only applies to note and CC modes)
bool toggle = false;

// set to true if your external pedal is closed in default state
// you also need to connect an output pin to the normally-connected pin of TS port (see above)
bool normallyClosed = true;

// LED settings
int ledBright = 1; //1-10 scale of how bright the LEDs are
const byte ledPin[5] = {14, 15, 16, 26, 25};
const int ledDelay = 125; //How long to light LEDs up for visual confirmation

//Default channel for MIDI notes, CC, program change, control change
int midiChan[3] = {16, 16, 16};

// can select from up to 5 switchBanks of notes/velocities/etc - defaults to 3
int switchBank = 0;
int numBanks = 3;

// arrays are declared here but loaded from EEPROM/defaults in eepromREAD() -- eeprom not implemented yet
int switchNotes[5][5] = {{60, 61, 62, 63, 64}, {65, 66, 67, 68, 69}, {70, 71, 72, 73, 74}, {75, 76, 77, 78, 79}, {80, 81, 82, 83, 84}};
int switchVels[5][5] = {{127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}};
int switchCCs[5][5] = {{64, 65, 66, 67, 68}, {69, 70, 71, 72, 73}, {74, 75, 76, 77, 78}, {79, 80, 81, 82, 83}, {84, 85, 86, 87, 88}};
int switchCCValOns[5][5] = {{127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}, {127, 127, 127, 127, 127}};
int switchCCValOffs[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
int switchPCs[5][5] = {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}, {11, 12, 13, 14, 15}, {16, 17, 18, 19, 20}, {21, 22, 23, 24, 25}};

bool ledStatus[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};

// switch states for runModes that require toggling
bool switchStates[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};

// create Bounce objects for each button and switch.

Bounce button1 = Bounce();
Bounce button2 = Bounce();
Bounce button3 = Bounce();
Bounce button4 = Bounce();
Bounce button5 = Bounce();
Bounce button6 = Bounce();
Bounce buttonExt = Bounce();

//Track runMode (function of footswitch) as well as pick default option if no change is made before timeout. Default timeout is 6000ms.
//0 = Startup screen (runMode select)
//1 = MIDI Note (default on/off as button pressed/released)
//2 = MIDI CC (default on/off as button pressed/released)
//3 = Program Change
//5 = Settings - not implemented y et
int runMode = 0;
int runModeDefault = 1; //will set by EEPROM - not setup yet

int runmodeTime = 5; // time in s to display runmode menu before selecting default
const int runmodeTimeLong = 60; // time in s to display runmode when you return to it manually during operation
elapsedMillis timeOut;

// for settings menu navigation
int menuPos[4] = {0, 0, 0, 0};
int menuSelect = 0;
char menuListMain[7][5] = {"CHAN", "NOTE", "CC", "PC", "SET", "DATA", "SAVE"};
char menuListChan[3][5] = {"NOTE", "CC", "PC"};
char menuListNote[2][5] = {"NOTE", "VEL"};
char menuListCC[3][5] = {"CC", "V ON", "VOFF"};
char menuListSettings[7][5] = {"DRUN", "TIME", "BANK", "EXSW", "LEDB", "SCRB", "BINF"};
char menuListData[3][5] = {"LOAD", "SAVE", "DEF"};

bool wipe = true; // enable/disable automatic screen wiping
const int wipeTime = 1000; // max time in ms to display anything on the screen
elapsedMillis screenWipe;

// initialize display
const int clkPin = 0;
const int dioPin = 1;
int screenBright = 1; // control how bright the screen is from 0-7
int scrollDelay = 100; // speed at which text scrolls on the screen
TM1637TinyDisplay display(clkPin, dioPin);

void setup() {
  // button setup
  button1.attach(switchPin[0], INPUT_PULLUP);
  button2.attach(switchPin[1], INPUT_PULLUP);
  button3.attach(switchPin[2], INPUT_PULLUP);
  button4.attach(switchPin[3], INPUT_PULLUP);
  button5.attach(switchPin[4], INPUT_PULLUP);
  button6.attach(switchPin[5], INPUT_PULLUP);
  buttonExt.attach(switchPin[6], INPUT_PULLUP);

  // button debounce interval in ms
  button1.interval(5);
  button2.interval(5);
  button3.interval(5);
  button4.interval(5);
  button5.interval(5);
  button6.interval(5);
  buttonExt.interval(5);

  //eepromREAD(); //Set variable values from EEPROM (or defaults if EEPROM has not been written)

  // display setup and greeting
  display.setBrightness(screenBright);
  display.setScrolldelay(scrollDelay);
  if (intro) {
    display.showString("hans");
    delay(250);
    display.showString(buildRev);
    delay(250);
  }
  display.clear();

  // serial debugging
  //Serial.begin(9600);

  // initialize MIDI
  MIDI.begin();

  // configure switch pins as for input mode with pullup resistors
  pinMode(switchPin[0], INPUT_PULLUP);
  pinMode(switchPin[1], INPUT_PULLUP);
  pinMode(switchPin[2], INPUT_PULLUP);
  pinMode(switchPin[3], INPUT_PULLUP);
  pinMode(switchPin[4], INPUT_PULLUP);
  pinMode(switchPin[5], INPUT_PULLUP);
  pinMode(switchPin[6], INPUT_PULLUP);

  // initialize built-in LED as an output.
  pinMode(ledPin[0], OUTPUT);
  pinMode(ledPin[1], OUTPUT);
  pinMode(ledPin[2], OUTPUT);
  pinMode(ledPin[3], OUTPUT);
  pinMode(ledPin[4], OUTPUT);

  // initialize PIN for normally-connected TS plug pin
  pinMode(extDriver, OUTPUT);

  // drive normally-connected TS pin low for normally-closed pedal and high for normally-open
  if (normallyClosed) digitalWrite(extDriver, LOW);
  else if (!normallyClosed) digitalWrite(extDriver, HIGH);

  updateButtons(); //Pull initial button status to prevent phantom MIDI messages from being sent once the loop starts

  display.clear();
  timeOut = 0;
}

void loop() {
  updateButtons(); // poll for button presses.

  switch (runMode) {
    case 0:
      runModeSelectMode();
      break;
    case 1:
      runModeNote();
      break;
    case 2:
      runModeCC();
      break;
    case 3:
      runModePC();
      break;
    case 5:
      runModeSettings();
      break;
  }

  while (usbMIDI.read()) // ignore incoming MIDI
  {
  }

  if (buttonExt.changed() || button6.changed()) {
    if (buttonExt.rose()) {
      if (normallyClosed) {
        shift = true;
        display.showString("Shft");
        screenWipe = 0;
      }
      else shift = false;
    } else if (buttonExt.fell()) {
      if (!normallyClosed) {
        shift = true;
        display.showString("Shft");
        screenWipe = 0;
      }
      else shift = false;
    }

    else if (button6.rose()) {
      shift = false;
    }
    else if (button6.fell()) {
      shift = true;
      display.showString("Shft");
      screenWipe = 0;
    }
  }

  // wipe the screen every wipeTime ms
  if (wipe) {
    if (screenWipe >= wipeTime) {
      display.clear();
      screenWipe = 0;
    }
  }

  updateLEDs(); // update LED status
}

void updateButtons() {
  button1.update();
  button2.update();
  button3.update();
  button4.update();
  button5.update();
  button6.update();
  buttonExt.update();
}

void runModeSelectMode() { // give choice between running modes, choose default mode after timeout if no option selected
  display.showString("Mode");

  ledStatus[switchBank][0] = 1;
  ledStatus[switchBank][1] = 1;
  ledStatus[switchBank][2] = 1;
  ledStatus[switchBank][4] = 1;

  if (button1.rose()) {
    resetLEDs();
    updateLEDs();
    runMode = 1;
    displayText(runMode - 1, 3);
  }
  else if (button2.rose()) {
    resetLEDs();
    updateLEDs();
    runMode = 2;
    displayText(runMode - 1, 3);
  }
  else if (button3.rose()) {
    resetLEDs();
    updateLEDs();
    runMode = 3;
    displayText(runMode - 1, 3);
  }
  else if (button5.rose()) {
    resetLEDs();
    updateLEDs();
    runMode = 5;
    displayText(runMode - 1, 3);
  }
  screenWipe = 0;

  if (timeOut >= (runmodeTime * 1000)) {
    resetLEDs();
    updateLEDs();
    runMode = runModeDefault;
    displayText(runMode - 1, 3);
    timeOut = 0;
  }
}

void runModeNote() {
  if (button1.changed() || button2.changed() || button3.changed() || button4.changed() || button5.changed()) {
    if (!shift) {
      if (!toggle) {
        if (button1.fell()) {
          display.showNumber(switchNotes[switchBank][0]);
          ledStatus[switchBank][0] = 1;
          usbMIDI.sendNoteOn(switchNotes[switchBank][0], switchVels[switchBank][0], midiChan[0]);
          MIDI.sendNoteOn(switchNotes[switchBank][0], switchVels[switchBank][0], midiChan[0]);
        }
        else if (button1.rose()) {
          display.showNumber(switchNotes[switchBank][0]);
          usbMIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
          MIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
          ledStatus[switchBank][0] = 0;
        }

        if (button2.fell()) {
          display.showNumber(switchNotes[switchBank][1]);
          ledStatus[switchBank][1] = 1;
          usbMIDI.sendNoteOn(switchNotes[switchBank][1], switchVels[switchBank][1], midiChan[0]);
          MIDI.sendNoteOn(switchNotes[switchBank][1], switchVels[switchBank][1], midiChan[0]);
        }
        else if (button2.rose()) {
          display.showNumber(switchNotes[switchBank][1]);
          usbMIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
          MIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
          ledStatus[switchBank][1] = 0;
        }

        if (button3.fell()) {
          display.showNumber(switchNotes[switchBank][2]);
          ledStatus[switchBank][2] = 1;
          usbMIDI.sendNoteOn(switchNotes[switchBank][2], switchVels[switchBank][2], midiChan[0]);
          MIDI.sendNoteOn(switchNotes[switchBank][2], switchVels[switchBank][2], midiChan[0]);
        }
        else if (button3.rose()) {
          display.showNumber(switchNotes[switchBank][2]);
          usbMIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
          MIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
          ledStatus[switchBank][2] = 0;
        }

        if (button4.fell()) {
          display.showNumber(switchNotes[switchBank][3]);
          ledStatus[switchBank][3] = 1;
          usbMIDI.sendNoteOn(switchNotes[switchBank][3], switchVels[switchBank][3], midiChan[0]);
          MIDI.sendNoteOn(switchNotes[switchBank][3], switchVels[switchBank][3], midiChan[0]);
        }
        else if (button4.rose()) {
          display.showNumber(switchNotes[switchBank][3]);
          usbMIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
          MIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
          ledStatus[switchBank][3] = 0;
        }

        if (button5.fell()) {
          display.showNumber(switchNotes[switchBank][4]);
          ledStatus[switchBank][4] = 1;
          usbMIDI.sendNoteOn(switchNotes[switchBank][4], switchVels[switchBank][4], midiChan[0]);
          MIDI.sendNoteOn(switchNotes[switchBank][4], switchVels[switchBank][4], midiChan[0]);
        }
        else if (button5.rose()) {
          display.showNumber(switchNotes[switchBank][4]);
          usbMIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
          MIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
          ledStatus[switchBank][4] = 0;
        }
      }
      else if (toggle) {
        if (button1.fell()) {
          display.showNumber(switchNotes[switchBank][0]);
          if (switchStates[switchBank][0] == 0) {
            switchStates[switchBank][0] = 1;
            usbMIDI.sendNoteOn(switchNotes[switchBank][0], switchVels[switchBank][0], midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][0], switchVels[switchBank][0], midiChan[0]);
            ledStatus[switchBank][0] = 1;
          } else {
            switchStates[switchBank][0] = 0;
            usbMIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
            ledStatus[switchBank][0] = 0;
          }
        }

        else if (button2.fell()) {
          display.showNumber(switchNotes[switchBank][1]);
          if (switchStates[switchBank][1] == 0) {
            switchStates[switchBank][1] = 1;
            usbMIDI.sendNoteOn(switchNotes[switchBank][1], switchVels[switchBank][1], midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][1], switchVels[switchBank][1], midiChan[0]);
            ledStatus[switchBank][1] = 1;
          } else {
            switchStates[switchBank][1] = 0;
            usbMIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
            ledStatus[switchBank][1] = 0;
          }
        }

        else if (button3.fell()) {
          display.showNumber(switchNotes[switchBank][2]);
          if (switchStates[switchBank][2] == 0) {
            switchStates[switchBank][2] = 1;
            usbMIDI.sendNoteOn(switchNotes[switchBank][2], switchVels[switchBank][2], midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][2], switchVels[switchBank][2], midiChan[0]);
            ledStatus[switchBank][2] = 1;
          } else {
            switchStates[switchBank][2] = 0;
            usbMIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
            ledStatus[switchBank][2] = 0;
          }
        }

        else if (button4.fell()) {
          display.showNumber(switchNotes[switchBank][3]);
          if (switchStates[switchBank][3] == 0) {
            switchStates[switchBank][3] = 1;
            usbMIDI.sendNoteOn(switchNotes[switchBank][3], switchVels[switchBank][3], midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][3], switchVels[switchBank][3], midiChan[0]);
            ledStatus[switchBank][3] = 1;
          } else {
            switchStates[switchBank][3] = 0;
            usbMIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
            ledStatus[switchBank][3] = 0;
          }
        }

        else if (button5.fell()) {
          display.showNumber(switchNotes[switchBank][4]);
          if (switchStates[switchBank][4] == 0) {
            switchStates[switchBank][4] = 1;
            usbMIDI.sendNoteOn(switchNotes[switchBank][4], switchVels[switchBank][4], midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][4], switchVels[switchBank][4], midiChan[0]);
            ledStatus[switchBank][4] = 1;
          } else {
            switchStates[switchBank][4] = 0;
            usbMIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
            ledStatus[switchBank][4] = 0;
          }
        }

      }
    }
    else if (shift) {
      shiftMode();
    }
  }
}

void runModeCC() {
  if (button1.changed() || button2.changed() || button3.changed() || button4.changed() || button5.changed()) {
    if (!shift) {
      if (!toggle) {
        if (button1.fell()) {
          display.showNumber(switchCCs[switchBank][0]);
          ledStatus[switchBank][0] = 1;
          usbMIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOns[switchBank][0], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOns[switchBank][0], midiChan[1]);
        }
        else if (button1.rose()) {
          display.showNumber(switchCCs[switchBank][0]);
          usbMIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOffs[switchBank][0], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOffs[switchBank][0], midiChan[1]);
          ledStatus[switchBank][0] = 0;
        }

        if (button2.fell()) {
          display.showNumber(switchCCs[switchBank][1]);
          ledStatus[switchBank][1] = 1;
          usbMIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOns[switchBank][1], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOns[switchBank][1], midiChan[1]);
        }
        else if (button2.rose()) {
          display.showNumber(switchCCs[switchBank][1]);
          usbMIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOffs[switchBank][1], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOffs[switchBank][1], midiChan[1]);
          ledStatus[switchBank][1] = 0;
        }

        if (button3.fell()) {
          display.showNumber(switchCCs[switchBank][2]);
          ledStatus[switchBank][2] = 1;
          usbMIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOns[switchBank][2], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOns[switchBank][2], midiChan[1]);
        }
        else if (button3.rose()) {
          display.showNumber(switchCCs[switchBank][2]);
          usbMIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOffs[switchBank][2], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOffs[switchBank][2], midiChan[1]);
          ledStatus[switchBank][2] = 0;
        }

        if (button4.fell()) {
          display.showNumber(switchCCs[switchBank][3]);
          ledStatus[switchBank][3] = 1;
          usbMIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOns[switchBank][3], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOns[switchBank][3], midiChan[1]);
        }
        else if (button4.rose()) {
          display.showNumber(switchCCs[switchBank][3]);
          usbMIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOffs[switchBank][3], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOffs[switchBank][3], midiChan[1]);
          ledStatus[switchBank][3] = 0;
        }

        if (button5.fell()) {
          display.showNumber(switchCCs[switchBank][4]);
          ledStatus[switchBank][4] = 1;
          usbMIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOns[switchBank][4], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOns[switchBank][4], midiChan[1]);
        }
        else if (button5.rose()) {
          display.showNumber(switchCCs[switchBank][4]);
          usbMIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOffs[switchBank][3], midiChan[1]);
          MIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOffs[switchBank][3], midiChan[1]);
          ledStatus[switchBank][4] = 0;
        }
      }

      else if (toggle) {

        if (button1.fell()) {
          display.showNumber(switchCCs[switchBank][0]);
          if (switchStates[switchBank][0] == 0) {
            switchStates[switchBank][0] = 1;
            usbMIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOns[switchBank][0], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOns[switchBank][0], midiChan[1]);
            ledStatus[switchBank][0] = 1;
          } else {
            switchStates[switchBank][0] = 0;
            usbMIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOffs[switchBank][0], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][0], switchCCValOffs[switchBank][0], midiChan[1]);
            ledStatus[switchBank][0] = 0;
          }
        }

        else if (button2.fell()) {
          display.showNumber(switchCCs[switchBank][1]);
          if (switchStates[switchBank][1] == 0) {
            switchStates[switchBank][1] = 1;
            usbMIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOns[switchBank][1], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOns[switchBank][1], midiChan[1]);
            ledStatus[switchBank][1] = 1;
          } else {
            switchStates[switchBank][1] = 0;
            usbMIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOffs[switchBank][1], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][1], switchCCValOffs[switchBank][1], midiChan[1]);
            ledStatus[switchBank][1] = 0;
          }
        }

        else if (button3.fell()) {
          display.showNumber(switchCCs[switchBank][2]);
          if (switchStates[switchBank][2] == 0) {
            switchStates[switchBank][2] = 1;
            usbMIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOns[switchBank][2], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOns[switchBank][2], midiChan[1]);
            ledStatus[switchBank][2] = 1;
          } else {
            switchStates[switchBank][2] = 0;
            usbMIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOffs[switchBank][2], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][2], switchCCValOffs[switchBank][2], midiChan[1]);
            ledStatus[switchBank][2] = 0;
          }
        }

        else if (button4.fell()) {
          display.showNumber(switchCCs[switchBank][3]);
          if (switchStates[switchBank][3] == 0) {
            switchStates[switchBank][3] = 1;
            usbMIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOns[switchBank][3], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOns[switchBank][3], midiChan[1]);
            ledStatus[switchBank][3] = 1;
          } else {
            switchStates[switchBank][3] = 0;
            usbMIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOffs[switchBank][3], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][3], switchCCValOffs[switchBank][3], midiChan[1]);
            ledStatus[switchBank][3] = 0;
          }
        }

        else if (button5.fell()) {
          display.showNumber(switchCCs[switchBank][4]);
          if (switchStates[switchBank][4] == 0) {
            switchStates[switchBank][4] = 1;
            usbMIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOns[switchBank][4], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOns[switchBank][4], midiChan[1]);
            ledStatus[switchBank][4] = 1;
          } else {
            switchStates[switchBank][4] = 0;
            usbMIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOffs[switchBank][4], midiChan[1]);
            MIDI.sendControlChange(switchCCs[switchBank][4], switchCCValOffs[switchBank][4], midiChan[1]);
            ledStatus[switchBank][4] = 0;
          }
        }

      }
    }
    else if (shift) {
      shiftMode();
    }
  }
}

void runModePC() {
  if (button1.changed() || button2.changed() || button3.changed() || button4.changed() || button5.changed()) {
    if (!shift) {
      if (button1.rose()) {
        display.showNumber(switchPCs[switchBank][0]);
        if (switchStates[switchBank][0] != 1) {
          resetSwitches();
          switchStates[switchBank][0] = 1;
          ledStatus[switchBank][0] = 1;
        }
        usbMIDI.sendProgramChange(switchPCs[switchBank][0], midiChan[2]);
        MIDI.sendProgramChange(switchPCs[switchBank][0], midiChan[2]);
      }
      if (button2.rose()) {
        display.showNumber(switchPCs[switchBank][1]);
        if (switchStates[switchBank][1] != 1) {
          resetSwitches();
          switchStates[switchBank][1] = 1;
          ledStatus[switchBank][1] = 1;
        }
        usbMIDI.sendProgramChange(switchPCs[switchBank][1], midiChan[2]);
        MIDI.sendProgramChange(switchPCs[switchBank][1], midiChan[2]);
      }
      if (button3.rose()) {
        display.showNumber(switchPCs[switchBank][2]);
        if (switchStates[switchBank][2] != 1) {
          resetSwitches();
          switchStates[switchBank][2] = 1;
          ledStatus[switchBank][2] = 1;
        }
        usbMIDI.sendProgramChange(switchPCs[switchBank][2], midiChan[2]);
        MIDI.sendProgramChange(switchPCs[switchBank][2], midiChan[2]);
      }
      if (button4.rose()) {
        display.showNumber(switchPCs[switchBank][3]);
        if (switchStates[switchBank][3] != 1) {
          resetSwitches();
          switchStates[switchBank][3] = 1;
          ledStatus[switchBank][3] = 1;
        }
        usbMIDI.sendProgramChange(switchPCs[switchBank][3], midiChan[2]);
        MIDI.sendProgramChange(switchPCs[switchBank][3], midiChan[2]);
      }
      if (button5.rose()) {
        display.showNumber(switchPCs[switchBank][4]);
        if (switchStates[switchBank][4] != 1) {
          resetSwitches();
          switchStates[switchBank][4] = 1;
          ledStatus[switchBank][4] = 1;
        }
        usbMIDI.sendProgramChange(switchPCs[switchBank][4], midiChan[2]);
        MIDI.sendProgramChange(switchPCs[switchBank][4], midiChan[2]);
      }
    }
    else if (shift) {
      shiftMode();
    }
  }
}

void displayText(int textNum, int blinkNum) {
  switch (textNum) {
    case 0:
      display.showString("Note");
      blinkLED(runMode, blinkNum);
      break;
    case 1:
      display.showString("CC");
      blinkLED(runMode, blinkNum);
      break;
    case 2:
      display.showString("PC");
      blinkLED(runMode, blinkNum);
      break;
    case 4:
      display.showString("Edit");
      blinkLED(runMode, blinkNum);
      break;
    case 5:
      resetLEDs();
      display.showString("Bank");
      delay(150);
      display.showNumber(switchBank + 1);
      delay(150);
      blinkLED(switchBank + 1, 2);
      break;
    case 6:
      display.showString("Menu");
      blinkLED(3, blinkNum);
      break;
    case 7:
      if (toggle) display.showString("Toggle");
      if (!toggle) display.showString("Momentary");
      break;
    case 8:
      display.showString("Bank");
      break;
    case 9:
      display.showString("Btn");
      break;
    case 10:
      display.showString("Channel");
      break;
    case 11:
      display.showString("Note");
      delay(100);
      displayText(8, 0);
      break;
    case 12:
      display.showString("CC");
      delay(100);
      displayText(8, 0);
      break;
    case 13:
      display.showString("PC");
      delay(100);
      displayText(8, 0);
      break;
    case 20:
      display.showString("Saved to EEPROM");
      break;
    case 21:
      display.showString("Loaded from EEPROM");
      break;
    case 22:
      display.showString("Loaded factory defaults");
      break;
  }
  screenWipe = 0;
  delay(250);
}

void shiftMode() {
  if (button1.rose()) { // move down one bank of switches
    if (switchBank > 0) switchBank--;
    else if (switchBank == 0) switchBank = numBanks - 1;
    displayText(5, 0);
  }
  else if (button2.rose()) { // switch between toggle and momentary mode
    resetNotes();
    resetLEDs();
    resetSwitches();
    toggle = !toggle;
    displayText(7, 1);
  }
  else if (button3.rose()) { // return to Menu
    resetNotes();
    resetLEDs();
    resetSwitches();
    displayText(6, 3);
    updateButtons();
    runMode = 0;
    runmodeTime = runmodeTimeLong;
    timeOut = 0;
  }
  else if (button4.rose()) { // panic! stop all notes
    resetSwitches();
    resetLEDs();
    panic();
    blinkLED(4, 5);
    display.showString("All notes stopped");
  }
  else if (button5.rose()) { // move up one bank of switches
    if (switchBank < (numBanks - 1)) switchBank++;
    else if (switchBank == (numBanks - 1)) switchBank = 0;
    displayText(5, 0);
  }

}

void resetSwitches() {
  for (int i = 0; i < 5; i++) { //Set all switch states to off
    for (int j = 0; j < 5; j++) {
      switchStates[i][j] = 0;
    }
  }
  resetLEDs();
}

void resetLEDs() {
  for (int i = 0; i < 5; i++) { //Set all LED states to off
    for (int j = 0; j < 5; j++) {
      ledStatus[i][j] = 0;
    }
  }
}

void blinkLED(int i, int j) {
  digitalWrite(ledPin[i - 1], LOW);
  for (int k = 0; k < j - 1; k++) {
    analogWrite(ledPin[i - 1], ledBright * 25.5);
    delay(ledDelay);
    analogWrite(ledPin[i - 1], 0);
    delay(ledDelay / 3);
  }
}

void updateLEDs() {
  for (int i = 0; i < 5; i++) {
    if ((ledStatus[switchBank][i] == 1) || (switchStates[switchBank][i] == 1)) {
      analogWrite(ledPin[i], ledBright * 25.5);
    }
    else analogWrite(ledPin[i], 0);
  }
}

// stop all outgoing notes
void resetNotes() {
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++) {
      if (switchStates[i][j] > 0) {
        usbMIDI.sendNoteOff(switchNotes[i][j], 0, midiChan[0]);
        MIDI.sendNoteOff(switchNotes[i][j], 0, midiChan[0]);
      }
    }
}

void panic() {
  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 5; j++) {
      usbMIDI.sendNoteOff(switchNotes[i][j], 0, midiChan[0]);
      MIDI.sendNoteOff(switchNotes[i][j], 0, midiChan[0]);
    }
}


/*
   Menu beyond this point, work in progress
*/

void resetMenu() {
  for (int i = 0; i < 4; i++) menuPos[i] = 0;
  menuSelect = 0;
}

void runModeSettings() {
  switch (menuPos[0]) {
    // main menu level - allow choices between channel, note, cc, pc, settings, data, and save/exit
    // must choose save/exit (or save under data) or else changes will be lost on reboot
    case 0:
      display.showString(menuListMain[menuSelect]);
      if (button1.rose()) { // button1 is back whenever possible - in this case it exist the menu without saving
        resetNotes();
        resetLEDs();
        resetSwitches();
        resetMenu();
        runmodeTime = runmodeTimeLong;
        runMode = 0;
        timeOut = 0;
        blinkLED(1, 3);
      }
      else if (button2.rose()) {
        if (menuSelect > 0) menuSelect--;
        else menuSelect = 6;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[0] = menuSelect + 1;
        blinkLED(3, 3);
        displayText(menuSelect + 10, 0);
        menuSelect = 0;
      }
      else if (button4.rose()) {
        if (menuSelect < 6) menuSelect++;
        else menuSelect = 0;
        blinkLED(4, 3);
      }
      screenWipe = 0;
      break;

    // channel submenu - change note/cc/pc channel
    case 1:
      backButton();
      switch (menuPos[1]) {
        case 0: // nothing selected
          display.showString(menuListChan[menuSelect]);
          if (button2.rose()) {
            if (menuSelect > 0) menuSelect--;
            else menuSelect = 2;
            blinkLED(2, 3);
          }
          else if (button3.rose()) {
            menuPos[1] = menuSelect + 1;
            blinkLED(3, 3);
          }
          else if (button4.rose()) {
            if (menuSelect < 2) menuSelect++;
            else menuSelect = 0;
            blinkLED(4, 3);
          }
          break;
        case 1: // change note channel
          display.showNumber(midiChan[0]);
          changeValue(0);
          break;
        case 2: // change CC channel
          display.showNumber(midiChan[1]);
          changeValue(0);
          break;
        case 3: // change PC channel
          display.showNumber(midiChan[2]);
          changeValue(0);
          break;
      }
      break;

    // note submenu - change note values and velocities
    case 2:
      backButton();
      switch (menuPos[1]) {
        case 0: // switch between banks
          display.showNumber(switchBank + 1);
          changeValue(1);
          break;
        case 1: // pick which button
          display.showNumber(menuSelect + 1);
          changeValue(2);
          break;
        case 2: // choose/change note or velocity
          switch (menuPos[2]) {
            case 0: // choose note or velocity
              display.showString(menuListNote[menuSelect]);
              changeValue(3);
              break;
            case 1: // change note
              display.showNumber(switchNotes[switchBank][menuPos[3]]);
              changeValue(4);
              break;
            case 2: // change velocity
              display.showNumber(switchVels[switchBank][menuPos[3]]);
              changeValue(5);
              break;
          }
          break;
      }
      break;

    // cc submenu - change cc numbers and on/off values
    case 3:
      backButton();
      switch (menuPos[1]) {
        case 0: // switch between banks
          display.showNumber(switchBank + 1);
          changeValue(1);
          break;
        case 1: // pick which button
          display.showNumber(menuSelect + 1);
          changeValue(2);
          break;
        case 2: // choose/change note or velocity
          switch (menuPos[2]) {
            case 0: // choose note or velocity
              display.showString(menuListCC[menuSelect]);
              changeValue(6);
              break;
            case 1: // change CC
              display.showNumber(switchCCs[switchBank][menuPos[3]]);
              changeValue(7);
              break;
            case 2: // change on value
              display.showNumber(switchCCValOns[switchBank][menuPos[3]]);
              changeValue(8);
              break;
            case 3: // change off value
              display.showNumber(switchCCValOffs[switchBank][menuPos[3]]);
              changeValue(9);
              break;
          }
          break;
      }
      break;

    // pc submenu - chance pc numbers
    case 4:
      backButton();
      switch (menuPos[1]) {
        case 0: // switch between banks
          display.showNumber(switchBank + 1);
          changeValue(1);
          break;
        case 1: // pick which button
          display.showNumber(menuSelect + 1);
          changeValue(2);
          break;
        case 2: // change pc number
          display.showNumber(switchPCs[switchBank][menuPos[3]]);
          changeValue(10);
          break;
      }
      break;

    // settings submenu - change system systems
    case 5:
      if (button1.rose()) { // button1 is back whenever possible
        if (menuPos[1] > 0) {
          menuSelect = menuPos[1] - 1;
          menuPos[1] = 0;
        }
        else {
          menuSelect = menuPos[0] - 1;
          menuPos[0] = 0;
        }
        blinkLED(1, 3);
      }
      switch (menuPos[1]) {
        case 0: // nothing selected
          display.showString(menuListSettings[menuSelect]);
          if (button2.rose()) {
            if (menuSelect > 0) menuSelect--;
            else menuSelect = 6;
            blinkLED(2, 3);
          }
          else if (button3.rose()) {
            menuPos[1] = menuSelect + 1;
            menuSelect = 0;
            blinkLED(3, 3);
          }
          else if (button4.rose()) {
            if (menuSelect < 6) menuSelect++;
            else menuSelect = 0;
            blinkLED(4, 3);
          }
          break;
        case 1: // default run mode
          display.showNumber(runModeDefault);
          changeValue(11);
          break;
        case 2: // startup timeout
          display.showNumber(runmodeTime);
          changeValue(12);
          break;
        case 3: //  number of banks
          display.showNumber(numBanks);
          changeValue(13);
          break;
        case 4: // ext footswitch default state
          if (normallyClosed) display.showString("NC");
          else if (!normallyClosed) display.showString("NO");
          changeValue(14);
          break;
        case 5: // led intensity
          display.showNumber(ledBright);
          changeValue(15);
          break;
        case 6: // screen brightness
          display.showNumber(screenBright);
          changeValue(16);
          break;
        case 7: // intro text on/off
          if (intro) display.showString("YES");
          else if (!intro) display.showString("NO");
          changeValue(17);
          break;
      }
      break;

    // data submenu - EEPROM management
    case 6:
      if (button1.rose()) { // button1 is back whenever possible
        if (menuPos[1] > 0) {
          menuSelect = menuPos[1] - 1;
          menuPos[1] = 0;
        }
        else {
          menuSelect = menuPos[0] - 1;
          menuPos[0] = 0;
        }
        blinkLED(1, 3);
      }
      switch (menuPos[1]) {
        case 0: // nothing selected
          display.showString(menuListData[menuSelect]);
          if (button2.rose()) {
            if (menuSelect > 0) menuSelect--;
            else menuSelect = 2;
            blinkLED(2, 3);
          }
          else if (button3.rose()) {
            menuPos[1] = menuSelect + 1;
            blinkLED(3, 3);
          }
          else if (button4.rose()) {
            if (menuSelect < 2) menuSelect++;
            else menuSelect = 0;
            blinkLED(4, 3);
          }
          break;
        case 1: // load from EEPROM
          displayText(21, 0);
          blinkLED(3, 3);
          menuPos[1] = 0;
          break;
        case 2: // save to EEPROM
          displayText(20, 0);
          blinkLED(3, 3);
          menuPos[1] = 0;
          break;
        case 3: // restore defaults (does not save to EEPROM automatically)
          displayText(22, 0);
          blinkLED(3, 3);
          menuPos[1] = 0;
          break;
      }
      break;

    // save to EEPROM and exit
    case 7:
      resetNotes();
      resetLEDs();
      resetSwitches();
      resetMenu();
      eepromSave();
      displayText(20, 0);
      runmodeTime = runmodeTimeLong;
      runMode = 0;
      timeOut = 0;
      blinkLED(1, 5);
      break;

  }
  screenWipe = 0;
  checkValues(); // verify changes are valid and correct if not
}

void backButton() {
  if (button1.rose()) { // button1 is back whenever possible
    if (menuPos[2] > 0) {
      menuSelect = menuPos[2] - 1;
      menuPos[2] = 0;
    }
    else if (menuPos[1] > 0) {
      menuSelect = menuPos[1] - 1;
      menuPos[1]--;
    }
    else {
      menuSelect = menuPos[0] - 1;
      menuPos[0] = 0;
    }
    blinkLED(1, 3);
  }
}

// handles functions from settings menu that change values
void changeValue(int value) {
  switch (value) {
    // change midi channels
    case 0:
      if (button2.rose()) {
        midiChan[menuPos[1] - 1]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        if (menuPos[1] > 0) menuPos[1] = 0;
        else resetMenu();
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        midiChan[menuPos[1] - 1]++;
        blinkLED(4, 3);
      }
      break;

    // change bank
    case 1:
      if (button2.rose()) {
        switchBank--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 1;
        menuSelect = 0;
        blinkLED(3, 3);
        displayText(9, 0);
      }
      else if (button4.rose()) {
        switchBank++;
        blinkLED(4, 3);
      }
      break;

    // choose button
    case 2:
      if (button2.rose()) {
        if (menuSelect > 0) menuSelect--;
        else menuSelect = 4;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 2; // position 2 is for editing note/velocity
        menuPos[3] = menuSelect; //menuPos[3] holds the selected button for editing
        blinkLED(3, 3);
        menuSelect = 0;
      }
      else if (button4.rose()) {
        if (menuSelect < 4) menuSelect++;
        else menuSelect = 0;
        blinkLED(4, 3);
      }
      break;

    // choose note or velocity edit
    case 3:
      if (button2.rose()) {
        if (menuSelect > 0) menuSelect--;
        else menuSelect = 1;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = menuSelect + 1;
        blinkLED(3, 3);
        menuSelect = 0;
      }
      else if (button4.rose()) {
        if (menuSelect < 1) menuSelect++;
        else menuSelect = 0;
        blinkLED(4, 3);
      }
      break;

    // change note
    case 4:
      if (button2.rose()) {
        switchNotes[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchNotes[switchBank][menuPos[3]]++;
        blinkLED(4, 3);
      }
      break;

    // change velocity
    case 5:
      if (button2.rose()) {
        switchVels[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchVels[switchBank][menuPos[3]]--;
        blinkLED(4, 3);
      }
      break;

    // choose cc number, on value, off value
    case 6:
      if (button2.rose()) {
        if (menuSelect > 0) menuSelect--;
        else menuSelect = 2;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = menuSelect + 1;
        blinkLED(3, 3);
        menuSelect = 0;
      }
      else if (button4.rose()) {
        if (menuSelect < 2) menuSelect++;
        else menuSelect = 0;
        blinkLED(4, 3);
      }
      break;

    // change cc number
    case 7:
      if (button2.rose()) {
        switchCCs[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchCCs[switchBank][menuPos[3]]++;
        blinkLED(4, 3);
      }
      break;

    // change cc on value
    case 8:
      if (button2.rose()) {
        switchCCValOns[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchCCValOns[switchBank][menuPos[3]]++;
        blinkLED(4, 3);
      }
      break;

    // change cc off value
    case 9:
      if (button2.rose()) {
        switchCCValOffs[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[2] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchCCValOffs[switchBank][menuPos[3]]++;
        blinkLED(4, 3);
      }
      break;

    // change pc number
    case 10:
      if (button2.rose()) {
        switchPCs[switchBank][menuPos[3]]--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1]--;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        switchPCs[switchBank][menuPos[3]]++;
        blinkLED(4, 3);
      }
      break;

    // change default run mdoe
    case 11:
      if (button2.rose()) {
        runModeDefault--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        runModeDefault++;
        blinkLED(4, 3);
      }
      break;

    // change default run mode timeout
    case 12:
      if (button2.rose()) {
        runmodeTime--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        runmodeTime++;
        blinkLED(4, 3);
      }
      break;

    // change number of banks
    case 13:
      if (button2.rose()) {
        numBanks--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        numBanks++;
        blinkLED(4, 3);
      }
      break;

    // change between normally closed/normally open external switch
    case 14:
      if (button2.rose()) {
        normallyClosed = !normallyClosed;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        normallyClosed = !normallyClosed;
        blinkLED(4, 3);
      }
      break;

    // change LED brightness
    case 15:
      if (button2.rose()) {
        ledBright--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        ledBright++;
        blinkLED(4, 3);
      }
      break;

    // change screen brightness
    case 16:
      if (button2.rose()) {
        screenBright--;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        screenBright++;
        blinkLED(4, 3);
      }
      break;

    // disable intro text with build date
    case 17:
      if (button2.rose()) {
        intro = !intro;
        blinkLED(2, 3);
      }
      else if (button3.rose()) {
        menuPos[1] = 0;
        blinkLED(3, 3);
      }
      else if (button4.rose()) {
        intro = !intro;
        blinkLED(4, 3);
      }
      break;
  }
}

void checkValues() {
  // MIDI channels
  for (int i = 0; i < 3; i++) {
    if (midiChan[i] > 16) midiChan[i] = 1;
    else if (midiChan[i] < 1) midiChan[i] = 16;
  }

  // bank of switches
  if (switchBank > (numBanks - 1)) switchBank = 0;
  else if (switchBank < 0) switchBank = numBanks - 1;

  // notes, CC, PC
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      if (switchNotes[i][j] > 127) switchNotes[i][j] = 0;
      else if (switchNotes[i][j] < 0) switchNotes[i][j] = 127;
      if (switchVels[i][j] > 127) switchVels[i][j] = 0;
      else if (switchVels[i][j] < 0) switchVels[i][j] = 127;
      if (switchCCs[i][j] > 127) switchCCs[i][j] = 0;
      else if (switchCCs[i][j] < 0) switchCCs[i][j] = 127;
      if (switchCCValOns[i][j] > 127) switchCCValOns[i][j] = 0;
      else if (switchCCValOns[i][j] < 0) switchCCValOns[i][j] = 127;
      if (switchCCValOffs[i][j] > 127) switchCCValOffs[i][j] = 0;
      else if (switchCCValOffs[i][j] < 0) switchCCValOffs[i][j] = 127;
      if (switchPCs[i][j] > 127) switchPCs[i][j] = 0;
      else if (switchPCs[i][j] < 0) switchPCs[i][j] = 127;
    }
  }

  // customizeable settings
  if (runModeDefault > 3) runModeDefault = 1;
  else if (runModeDefault < 1) runModeDefault = 3;
  if (runmodeTime > 60) runmodeTime = 0;
  else if (runmodeTime < 0) runmodeTime = 60;
  if (numBanks > 5) numBanks = 2; // max 5 banks, minimum 2
  else if (numBanks < 2) numBanks = 5;
  if (ledBright > 10) ledBright = 1;
  else if (ledBright < 1) ledBright = 10;
  if (screenBright > 7) screenBright = 1;
  else if (screenBright < 1) screenBright = 7;
}

/*
   EEPROM management functions
*/

void eepromLoad() {

}

void eepromSave() {

}

void eepromDefaults() {

}
