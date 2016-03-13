#include "base64/Base64.h"

const int ledPin = 13;
byte counter;
const byte CLOCK = 248;
const byte START = 250;
const byte CONTINUE = 251;
const byte STOP = 252;
const int redLed = 1;

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

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  Serial.begin(9600);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);
  usbMIDI.setHandleSysEx(SystemExclusiveMessage);
  Serial.println("start");
}

void loop()
{
  usbMIDI.read();
}

void RealTimeSystem(byte realtimebyte) {
  if (realtimebyte == CLOCK) {
    counter++;
    if (counter == 24) {
      counter = 0;
      digitalWrite(redLed, HIGH);
    }
    if (counter == 23) {
      digitalWrite(redLed, HIGH);
    }

    if (counter == 6) {
      digitalWrite(redLed, LOW);
    }
  }

  if (realtimebyte == START || realtimebyte == CONTINUE) {
    counter = 0;
    digitalWrite(redLed, HIGH);
  }

  if (realtimebyte == STOP) {
    digitalWrite(redLed, LOW);
  }
}

