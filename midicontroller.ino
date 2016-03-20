#include <Bounce2.h>

#include <Adafruit_GFX.h>

#include <SPI.h>

#include <Wire.h>

#include <Adafruit_SSD1306.h>

#include "base64/Base64.h"

byte counter;
const byte CLOCK = 248;
const byte START = 250;
const byte CONTINUE = 251;
const byte STOP = 252;
const int ledPin = 13;

const byte channelButtonStart = 0;
const byte numberOfChannelButtons = 2;
const byte ledPins[] = {/*10, 11, 12,*/ 14, 15, 16};
Bounce channelButtons[numberOfChannelButtons];
boolean channelButtonStates[numberOfChannelButtons];
boolean channelButtonDown[numberOfChannelButtons];

const byte bankDownPin = 6;
const byte bankUpPin = 7;
Bounce bankDownButton = Bounce();
Bounce bankUpButton = Bounce();
boolean bankDownState;
boolean bankDownDown;
boolean bankUpState;
boolean bankUpDown;

const byte stopPin = 8;
const byte playPin = 9;
Bounce stopButton = Bounce();
Bounce playButton = Bounce();
boolean stopState;
boolean stopDown;
boolean playState;
boolean playDown;

boolean bankDownLastState = false;
boolean bankUpLastState = false;

byte bank = 0;

const byte bpmLed = 22;


void OnNoteOn(byte channel, byte note, byte velocity)
{
  digitalWrite(ledPin, HIGH);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
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
    initButton(channelButtonStart + i, &channelButtons[i], &channelButtonStates[i], &channelButtonDown[i]);
  }
  initButton(bankDownPin, &bankDownButton, &bankDownState, &bankDownDown);
  initButton(bankUpPin, &bankUpButton, &bankUpState, &bankUpDown);
  initButton(stopPin, &stopButton, &stopState, &stopDown);
  initButton(playPin, &playButton, &playState, &playDown);
}

void initButton(byte pin, Bounce *button, boolean *state, boolean *down) {
  pinMode(pin, INPUT_PULLUP);
  (*button).attach(pin);
  (*button).interval(10);
  *state = false;
  *down = false;
}

void setupLeds() {
  pinMode(ledPin, OUTPUT);
  pinMode(bpmLed, OUTPUT);
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
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

  Serial.println("start");
}

void ledTest() {
  ledTest(ledPin);
  ledTest(bpmLed);
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    ledTest(ledPins[i]);
    //    digitalWrite(channelButtonLedStart +i,HIGH);
  }
}

void ledTest(byte pin) {
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
  delay(200);
}

void updateButtons() {
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    updateButtonState(channelButtonStart + i, &channelButtons[i], &channelButtonStates[i], &channelButtonDown[i]);
  }
  updateButtonState(bankDownPin, &bankDownButton, &bankDownState, &bankDownDown);
  updateButtonState(bankUpPin, &bankUpButton, &bankUpState, &bankUpDown);
  updateButtonState(stopPin, &stopButton, &stopState, &stopDown);
  updateButtonState(playPin, &playButton, &playState, &playDown);
}

void updateButtonState(byte pin, Bounce *button, boolean *state, boolean *down) {
  if ((*button).update()) {
    if ((*button).read() == LOW) {
      *down = true;
      Serial.print("Button ");
      Serial.print(pin);
      Serial.println(" down");
    } else if (*down) {
      Serial.print("Button ");
      Serial.print(pin);
      Serial.println(" up");
      *state = 1 - *state;
      *down = false;
    }
  }
}

void loop()
{
  usbMIDI.read();

  updateButtons();

  updateState();

  updateLeds();


}

void updateState() {
  boolean update = false;
  if (bankDownState != bankDownLastState) {
    bankDownLastState = bankDownState;
    bank--;
    update = true;
  } else if (bankUpState != bankUpLastState) {
    bankUpLastState = bankUpState;
    bank++;
    update = true;
  }
  if (update) {
    Serial.print("Bank ");
    Serial.println(bank);
  }
}

void updateLeds() {
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    if (channelButtonStates[i]) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

void RealTimeSystem(byte realtimebyte) {
  if (realtimebyte == CLOCK) {
    counter++;
    if (counter == 24) {
      counter = 0;
      digitalWrite(bpmLed, HIGH);
    }
    if (counter == 23) {
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

