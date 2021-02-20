/*
   5 buttons and an external switch walk in to a bar.
*/

#include <TM1637TinyDisplay.h>
#include <AceButton.h>
#include <MIDI.h>
//#include <EEPROM.h>

using namespace ace_button;

elapsedMillis ledCount;
elapsedMillis screenWipe;

// create MIDI instance for 5 pin MIDI output
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

#define buildRev "20210219"

// PIN assignments
// for the display
const int wipeTime = 1000; // max time in ms to display anything on the screen
const int CLK_PIN = 0;
const int DIO_PIN = 1;

// for buttons and external switch (acts as an extra button)
const byte switchPin[6] = {10, 11, 12, 13, 14, 17};
// for LEDs
const byte ledPin[5] = {28, 29, 30, 31, 32};

// all buttons called using basic acebutton config
AceButton button1(switchPin[0]);
AceButton button2(switchPin[1]);
AceButton button3(switchPin[2]);
AceButton button4(switchPin[3]);
AceButton button5(switchPin[4]);
AceButton extswitch(switchPin[5]);

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
// set to true if your external pedal is closed in default state
// you will also need to have soldered the normally-connected contact on the TS jack to ground
// or you will have problems when the external swich is disconnected
bool inversepedal = true; 

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

// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);

// initialize display
TM1637TinyDisplay display(CLK_PIN, DIO_PIN);

void setup() {

  // Display setup and greeting
  display.setBrightness(0x01);
  display.setScrolldelay(100);
  display.showString("hans");
  delay(250);
  display.showString(buildRev);
  display.showString("Ready");
  delay(250);
  display.clear();

  // Initialize MIDI
  MIDI.begin();

  // Initialize built-in LED as an output.
  pinMode(ledPin[0], OUTPUT);
  pinMode(ledPin[1], OUTPUT);
  pinMode(ledPin[2], OUTPUT);
  pinMode(ledPin[3], OUTPUT);
  pinMode(ledPin[4], OUTPUT);

  // Buttons use the built-in pull up register.
  pinMode(switchPin[0], INPUT_PULLUP);
  pinMode(switchPin[1], INPUT_PULLUP);
  pinMode(switchPin[2], INPUT_PULLUP);
  pinMode(switchPin[3], INPUT_PULLUP);
  pinMode(switchPin[4], INPUT_PULLUP);
  pinMode(switchPin[5], INPUT_PULLUP);

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
}

void loop() {
  // wipe the screen, if needed
  screenCheck();

  // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
  button1.check();
  button2.check();
  button3.check();
  button4.check();
  button5.check();
  extswitch.check();

  // Ignore incoming MIDI
  while (usbMIDI.read())
  {
  }
}

// The event handler for both buttons.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  uint8_t pin = button->getPin();

  // If buttons 1-5 are pressed
  if ((pin == switchPin[0]) || (pin == switchPin[1]) || (pin == switchPin[2]) || (pin == switchPin[3]) || (pin == switchPin[4])) {
    switch (eventType) {
      case AceButton::kEventPressed:
        if (shift == false) { // regular usage when shift mode is disabled (footbutton is not pressed)
          if (pin == switchPin[0]) {
            display.showNumber(switchNotes[switchBank][0]);
            digitalWrite(ledPin[0], HIGH);
            screenWipe = 0;
            usbMIDI.sendNoteOn(switchNotes[switchBank][0], 127, midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][0], 127, midiChan[0]);
          }
          if (pin == switchPin[1]) {
            display.showNumber(switchNotes[switchBank][1]);
            digitalWrite(ledPin[1], HIGH);
            screenWipe = 0;
            usbMIDI.sendNoteOn(switchNotes[switchBank][1], 127, midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][1], 127, midiChan[0]);
          }
          if (pin == switchPin[2]) {
            display.showNumber(switchNotes[switchBank][2]);
            digitalWrite(ledPin[2], HIGH);
            screenWipe = 0;
            usbMIDI.sendNoteOn(switchNotes[switchBank][2], 127, midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][2], 127, midiChan[0]);
          }
          if (pin == switchPin[3]) {
            display.showNumber(switchNotes[switchBank][3]);
            digitalWrite(ledPin[3], HIGH);
            screenWipe = 0;
            usbMIDI.sendNoteOn(switchNotes[switchBank][3], 127, midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][3], 127, midiChan[0]);
          }
          if (pin == switchPin[4]) {
            display.showNumber(switchNotes[switchBank][4]);
            digitalWrite(ledPin[4], HIGH);
            screenWipe = 0;
            usbMIDI.sendNoteOn(switchNotes[switchBank][4], 127, midiChan[0]);
            MIDI.sendNoteOn(switchNotes[switchBank][4], 127, midiChan[0]);
          }
        }
        else if (shift == true) {
          if (pin == switchPin[0]) {
            digitalWrite(ledPin[0], HIGH);
            screenWipe = 0;
            if (switchBank > 0) switchBank--;
            else if (switchBank == 0) switchBank = numBanks - 1;
            displayBank();
          }
          if (pin == switchPin[1]) {
            digitalWrite(ledPin[1], HIGH);
            screenWipe = 0;
          }
          if (pin == switchPin[2]) {
            digitalWrite(ledPin[2], HIGH);
            screenWipe = 0;
          }
          if (pin == switchPin[3]) {
            digitalWrite(ledPin[3], HIGH);
            screenWipe = 0;
          }
          if (pin == switchPin[4]) {
            digitalWrite(ledPin[4], HIGH);
            screenWipe = 0;
            if (switchBank < (numBanks - 1)) switchBank++;
            else if (switchBank == (numBanks - 1)) switchBank = 0;
            displayBank();
          }
        }
        break;
      case AceButton::kEventReleased:
        if (shift == false) {
          if (pin == switchPin[0]) {
            digitalWrite(ledPin[0], LOW);
            usbMIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][0], 0, midiChan[0]);
          }
          if (pin == switchPin[1]) {
            digitalWrite(ledPin[1], LOW);
            usbMIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][1], 0, midiChan[0]);
          }
          if (pin == switchPin[2]) {
            digitalWrite(ledPin[2], LOW);
            usbMIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][2], 0, midiChan[0]);
          }
          if (pin == switchPin[3]) {
            digitalWrite(ledPin[3], LOW);
            usbMIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][3], 0, midiChan[0]);
          }
          if (pin == switchPin[4]) {
            digitalWrite(ledPin[4], LOW);
            usbMIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
            MIDI.sendNoteOff(switchNotes[switchBank][4], 0, midiChan[0]);
          }
        }
        else if (shift == true) {
          if (pin == switchPin[0]) {
            digitalWrite(ledPin[0], LOW);
          }
          if (pin == switchPin[1]) {
            digitalWrite(ledPin[1], LOW);
          }
          if (pin == switchPin[2]) {
            digitalWrite(ledPin[2], LOW);
          }
          if (pin == switchPin[3]) {
            digitalWrite(ledPin[3], LOW);
          }
          if (pin == switchPin[4]) {
            digitalWrite(ledPin[4], LOW);
          }
        }
        display.clear();
        break;
    }
  }


  // If the external switch is pressed, enable shift mode. Disable when released.
  // Possibly change shift to a toggle/momentary option in the future menu
  else if (pin == switchPin[5]) {
    int j;
    switch (eventType) {
      case AceButton::kEventPressed:
        if (inversepedal) {
          display.showString("SOff");
          shift = false;
        } else if (!inversepedal) {
          display.showString("S On");
          shift = true;
        }
        screenWipe = 0;
        break;
      case AceButton::kEventReleased:
        if (inversepedal) {
          display.showString("S On");
          shift = true;
        } else if (!inversepedal) {
          display.showString("SOff");
          shift = false;
        }
        screenWipe = 0;
        break;
    }
  }
}

// update LED status
void screenCheck() {
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
