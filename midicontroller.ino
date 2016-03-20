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
//const byte channelButtonLedStart = 14;
const byte numberOfChannelButtons = 2;
const byte ledPins[] = {/*10, 11, 12,*/ 14, 15, 16};
Bounce channelButtons[numberOfChannelButtons];
boolean channelButtonStates[numberOfChannelButtons];
boolean channelButtonDown[numberOfChannelButtons];

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
    pinMode(channelButtonStart + i, INPUT_PULLUP);
    channelButtons[i].attach(channelButtonStart + i);
    channelButtons[i].interval(10);
    channelButtonStates[i] = false;
    channelButtonDown[i] = false;
  }
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
    if (channelButtons[i].update()) {
      if (channelButtons[i].read() == LOW) {
        channelButtonDown[i] = true;
      } else if (channelButtonDown[i]) {
        channelButtonStates[i] = 1 - channelButtonStates[i];
        channelButtonDown[i] = false;
        if (channelButtonStates[i]) {
          digitalWrite(ledPins[i], HIGH);
        } else {
          digitalWrite(ledPins[i], LOW);
        }
      }
    }
  }
}

void loop()
{
  usbMIDI.read();
  updateButtons();
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

