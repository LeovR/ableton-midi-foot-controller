#include "Button.h"
#include "base64/Base64.h"

//#define DEBUG_BUTTONS true

byte counter;
const byte CLOCK = 248;
const byte START = 250;
const byte CONTINUE = 251;
const byte STOP = 252;

// LEDs
const byte ledPin = 13;
const byte bpmLed = 17;
const byte ledPins[] = {10, 11, 12, 14, 15, 16};
const byte bankDownLed = 22;
const byte bankUpLed = 23;
const byte playLed = 20;
const byte stopLed = 21;

const byte allLeds[] = {ledPin, bpmLed, ledPins[0], ledPins[1], ledPins[2],
                        ledPins[3], ledPins[4], ledPins[5], bankDownLed, bankUpLed, playLed, stopLed
                       };

const byte numberOfLeds = 12;

// Buttons
const byte channelButtonStart = 0;
const byte numberOfChannelButtons = 6;
const byte numberOfAllButtons = 10;

const byte bankDownPin = 6;
const byte bankUpPin = 7;

const byte stopPin = 8;
const byte playPin = 9;

Button channelButtons[numberOfChannelButtons];

Button bankDownButton = Button();
Button bankUpButton = Button();
boolean bankDownState;
boolean bankDownDown;
boolean bankUpState;
boolean bankUpDown;

Button stopButton = Button();
Button playButton = Button();
boolean stopState;
boolean stopDown;
boolean playState;
boolean playDown;

Button* allButtons[numberOfChannelButtons + 4];

boolean bankDownLastState = false;
boolean bankUpLastState = false;

byte bank = 0;

// Modes
const byte normalMode = 0;
const byte initMode = 1;
const byte modeCount = 2;
byte mode = normalMode;

const char* modeNames[] = {"Normal-Mode", "Init-Mode"};

boolean bothBanksDown = false;

const byte midiChannel = 1;

void OnNoteOn(byte channel, byte note, byte velocity) {
  digitalWrite(ledPin, HIGH);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(ledPin, LOW);
}

void SystemExclusiveMessage(const unsigned char *array, short unsigned int size, bool sysexbool) {
  unsigned char *dataArray = (unsigned char*)(array + 4);
  unsigned int realSize = size - 5;

  int decodedLen = base64_dec_len((char*)dataArray, realSize);
  Serial.println(decodedLen);
  char decoded[decodedLen];

  base64_decode(decoded, (char*)dataArray, realSize);

  Serial.println(decoded);

}

void setupButtons() {
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    channelButtons[i].init(channelButtonStart + i, ledPins[i]);
    allButtons[i] = &channelButtons[i];
  }
  bankDownButton.init(bankDownPin, bankDownLed);
  allButtons[numberOfChannelButtons + 0] = &bankDownButton;
  bankUpButton.init(bankUpPin, bankUpLed);
  allButtons[numberOfChannelButtons + 1] = &bankUpButton;
  stopButton.init(stopPin, stopLed);
  allButtons[numberOfChannelButtons + 2] = &stopButton;
  playButton.init(playPin, playLed);
  allButtons[numberOfChannelButtons + 3] = &playButton;
}

void setupLeds() {
  pinMode(ledPin, OUTPUT);
  pinMode(bpmLed, OUTPUT);
}


void setup()
{
  setupButtons();
  setupLeds();

  Serial.begin(9600);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);
  usbMIDI.setHandleSysEx(SystemExclusiveMessage);

  ledTest();

  Serial.println("MIDI Controller");
}

void ledTest() {
  ledTest(ledPin);
  ledTest(bpmLed);
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    ledTest(ledPins[i]);
    //    digitalWrite(channelButtonLedStart +i,HIGH);
  }
  ledTest(bankDownLed);
  ledTest(bankUpLed);
  ledTest(stopLed);
  ledTest(playLed);
}

void ledTest(byte pin) {
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
  delay(200);
}

void debugButton(Button *button) {
#ifdef DEBUG_BUTTONS
  Serial.print("Button: ");
  Serial.print((*button).getPin());
  Serial.print(" Just pressed: ");
  Serial.print((*button).isJustPressed());
  Serial.print(" Pressed: ");
  Serial.print((*button).isPressed());
  Serial.print(" Just released : ");
  Serial.println((*button).isJustReleased());
#endif
}

void updateButtons() {
  for (byte i = 0; i < numberOfAllButtons; i++) {
    if ((*allButtons[i]).update()) {
      debugButton(allButtons[i]);
    }
  }
}

void sendInitMidiNotes() {
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    if (channelButtons[i].isJustReleased()) {
      usbMIDI.sendNoteOff(i, 0, midiChannel);
    } else if (channelButtons[i].isJustPressed()) {
      usbMIDI.sendNoteOn(i, 99, midiChannel);
    }
  }
}

void loop()
{
  usbMIDI.read();

  updateButtons();

  boolean modeChange = changeMode();
  if (modeChange) {
    Serial.println(modeNames[mode]);
    resetLeds();
  }

  switch (mode) {
    case normalMode:
      handleNormalMode();
      break;
    case initMode:
      handleInitMode();
      break;
  }

  /*if (!modeChange) {
    changeBank();
    }

    updateLeds();

    sendMidiNotes();*/

}

void handleInitMode() {
  sendInitMidiNotes();
  updateInitLeds();
}

void handleNormalMode() {
  changeBank();
}

boolean changeMode() {
  if (bankDownButton.isPressed() && bankUpButton.isPressed()) {
    bothBanksDown = true;
  } else if (bothBanksDown && ((bankDownButton.isJustReleased() && !bankUpButton.isPressed()) || (bankUpButton.isJustReleased() && !bankDownButton.isPressed()))) {
    bothBanksDown = false;
    mode++;
    mode = mode % modeCount;
    return true;
  } else if (bothBanksDown) {
    return false;
  }
  return false;
}

void changeBank() {
  boolean update = false;
  if (bankDownButton.isJustReleased()) {
    bank--;
    update = true;
  } else if (bankUpButton.isJustReleased()) {
    bank++;
    update = true;
  }
  if (update) {
    Serial.print("Bank ");
    Serial.println(bank);
  }
}

void updateLeds() {
  /*if(initMode) {
    return;
    }
    for (byte i = 0; i < numberOfChannelButtons; i++) {
    if (channelButtons[i].isJustReleased()) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
    }*/
}

void updateInitLeds() {
  for (byte i = 0; i < numberOfAllButtons; i++) {
    if ((*allButtons[i]).isPressed()) {
      (*allButtons[i]).turnLedOn();
    } else {
      (*allButtons[i]).turnLedOff();
    }
  }
}

void resetLeds() {
  for (byte i = 0; i < numberOfLeds; i++) {
    digitalWrite(allLeds[i], LOW);
  }
}

void RealTimeSystem(byte realtimebyte) {
  if (realtimebyte == CLOCK) {
    counter++;
    if (counter == 24) {
      counter = 0;
      digitalWrite(bpmLed, HIGH);
    }
    if (counter == 22) {
      digitalWrite(bpmLed, HIGH);
    }

    if (counter == 6) {
      digitalWrite(bpmLed, LOW);
    }
  }

  if (realtimebyte == START || realtimebyte == CONTINUE) {
    counter = 0;
    digitalWrite(bpmLed, HIGH);
  }

  if (realtimebyte == STOP) {
    digitalWrite(bpmLed, LOW);
  }
}

