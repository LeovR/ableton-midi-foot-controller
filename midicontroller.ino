#include "Button.h"
#include "base64/Base64.h"

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>



byte counter;
const byte CLOCK = 248;
const byte START = 250;
const byte CONTINUE = 251;
const byte STOP = 252;
const int ledPin = 13;

const byte bpmLed = 22;

const byte channelButtonStart = 0;
const byte numberOfChannelButtons = 2;
const byte ledPins[] = {/*10, 11, 12,*/ 14, 15, 16};
Button channelButtons[numberOfChannelButtons];
boolean channelButtonStates[numberOfChannelButtons];
boolean channelButtonDown[numberOfChannelButtons];

const byte bankDownPin = 6;
const byte bankUpPin = 7;
Button bankDownButton = Button();
Button bankUpButton = Button();
boolean bankDownState;
boolean bankDownDown;
boolean bankUpState;
boolean bankUpDown;

const byte stopPin = 8;
const byte playPin = 9;
Button stopButton = Button();
Button playButton = Button();
boolean stopState;
boolean stopDown;
boolean playState;
boolean playDown;

boolean bankDownLastState = false;
boolean bankUpLastState = false;

byte bank = 0;

boolean initMode = false;

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
    channelButtons[i].init(channelButtonStart + i);
  }
  bankDownButton.init(bankDownPin);
  bankUpButton.init(bankUpPin);
  stopButton.init(stopPin);
  playButton.init(playPin);
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
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    if (channelButtons[i].update()) {
      debugButton(&channelButtons[i]);
    }
  }
  if (bankDownButton.update()) {
    debugButton(&bankDownButton);
  }
  if (bankUpButton.update()) {
    debugButton(&bankUpButton);
  }
  if (stopButton.update()) {
    debugButton(&stopButton);
  }
  if (playButton.update()) {
    debugButton(&playButton);
  }
}

void loop()
{
  usbMIDI.read();

  updateButtons();

  changeBank();

  changeMode();

  updateLeds();

}

boolean bothBanksDownLastState = false;

void changeMode() {
  /*if (bankDownDown && bankUpDown) {
    if (!previousBothBanksDown) {
      initMode = 1 - initMode;
      previousBothBanksDown = true;
      if (initMode) {
        Serial.println("Init mode");
      } else {
        Serial.println("Normal mode");
      }
    } else {
      previousBothBanksDown = false;
    }
    }*/

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

