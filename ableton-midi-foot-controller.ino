#include "Button.h"
#include "base64/Base64.h"
#include <Wire.h>
#include "NewLiquidCrystal/LiquidCrystal_I2C.h"

//#define DEBUG_BUTTONS true
//#define SINGLE_LED_TEST true
//#define LCD_TEST true
//#define SONG_CONFIGURATION_DEBUG true
//#define DEBUG_BANK true

byte counter;
byte tempCounter;
const byte CLOCK = 248;
const byte START = 250;
const byte CONTINUE = 251;
const byte STOP = 252;

#define SYSEX_BUFFER_SIZE 100
char sysexBuffer[SYSEX_BUFFER_SIZE];

#define MAX_ROW_LENGTH 16

char partBuffer[MAX_ROW_LENGTH + 1];

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

// LCD
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

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

const byte numberOfBanks = 5;

// Modes
const byte UNCONFIGURED = 0;
const byte NORMAL_MODE = 1;
const byte INIT_MODE = 2;
const byte SONG_MODE = 3;
const byte modeCount = 4;
byte mode = UNCONFIGURED;

byte initBank = 0;

const char mode_0[] PROGMEM = "Normal-Mode";
const char mode_1[] PROGMEM = "Init-Mode";
const char* const modeNames[] PROGMEM = {mode_0, mode_1};

const char displayName[] PROGMEM = "MIDI Controller";

boolean bothBanksDown = false;

const byte midiChannel = 1;

const byte SONG_CONFIGURATION = 0;
const byte CONFIGURATION_START = 1;
const byte CONFIGURATION_FINISHED = 2;
const byte CURRENT_PART = 3;
const byte NEXT_PART = 4;
const byte SONG_SIGNATURE = 5;

const byte SONG_OFFSET = 10;

const byte numberOfSongs = 30;
char songs[numberOfSongs][MAX_ROW_LENGTH + 1];

const byte CONTROL_OFFSET = SONG_OFFSET + numberOfSongs + 1;

byte selectedSong = -1;

const byte STOP_MIDI_NOTE = 0;
const byte REPEAT_MIDI_NOTE = CONTROL_OFFSET;
const byte SEND_CONFIGURATION = CONTROL_OFFSET + 1;
const byte STOP_CLIPS = CONTROL_OFFSET + 2;

const byte INVALID_MIDI_NOTE = 254;
byte midiNoteToSend = INVALID_MIDI_NOTE;
elapsedMillis midiNoteSendStart = 0;
boolean startedMidiNoteSending = false;

boolean playing = false;
byte numerator;
byte denominator;
byte bars;
double fullDenominator;

byte currentNumerator = 0;

boolean nextPartScheduled = false;

boolean repeat = false;
elapsedMillis repeatElapsed = 0;

elapsedMillis triggeredElapsed = 0;

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
  char decoded[decodedLen];

  base64_decode(decoded, (char*)dataArray, realSize);

  boolean isConfigurationMessage = strchr(decoded, '{') != NULL;

  if (isConfigurationMessage) {
    byte configurationType = getConfigurationType(decoded + 1);
    switch (configurationType) {
      case SONG_CONFIGURATION:
        handleSongConfiguration(decoded + 4);
        break;
      case CONFIGURATION_START:
        handleConfigurationStart();
        break;
      case CONFIGURATION_FINISHED:
        handleConfigurationFinished();
        break;
      case CURRENT_PART:
        handleCurrentPart(decoded + 4);
        break;
      case NEXT_PART:
        handleNextPart(decoded + 4);
        break;
      case SONG_SIGNATURE:
        handleSongSignature(decoded + 4);
        break;
    }
  }
}

void handleCurrentPart(char* message) {
  if (mode != SONG_MODE) {
    return;
  }

  copyToDisplayBuffer(partBuffer, message);

  nextPartScheduled = false;
  bars = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(partBuffer);
}

void handleSongSignature(char* messageOriginal) {
  if (mode != SONG_MODE) {
    return;
  }
  sysexBuffer[0] = (char)0;

  strcpy(sysexBuffer, messageOriginal);
  char* strings;
  strings = strtok(sysexBuffer, "|");
  if (strings == NULL) {
    return;
  }

  byte numeratorTemp = atoi(strings);

  strings = strtok(NULL, "|");
  if (strings == NULL) {
    return;
  }

  byte denominatorTemp = atoi(strings);
  numerator = numeratorTemp;
  denominator = denominatorTemp;

}

void clearLine(byte line) {
  lcd.setCursor(0, line);
  lcd.print(F("                "));
}

void handleNextPart(char* message) {
  if (mode != SONG_MODE) {
    return;
  }

  copyToDisplayBuffer(partBuffer, message);

  nextPartScheduled = true;
  triggeredElapsed = 0;

  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print(partBuffer);
}

void copyToDisplayBuffer(char* displayBuffer, char* message) {
  displayBuffer[0] = (char)0;

  strncpy(displayBuffer, message, MAX_ROW_LENGTH);
  byte messageLength = strlen(message);
  if (messageLength < MAX_ROW_LENGTH) {
    displayBuffer[messageLength] = (char)0;
  } else {
    displayBuffer[MAX_ROW_LENGTH] = (char)0;
  }
}

void handleConfigurationStart() {
  lcd.clear();
  lcd.print(F("Configuration"));
  lcd.setCursor(0, 1);
  lcd.print(F("start"));
}

void handleConfigurationFinished() {
#ifdef SONG_CONFIGURATION_DEBUG
  for (byte i = 0; i < numberOfSongs; i++) {
    if (songs[i]) {
      Serial.print(i);
      Serial.print(F(" "));
      Serial.println(songs[i]);
    }
  }
#endif
  lcd.clear();
  lcd.print(F("Configuration"));
  lcd.setCursor(0, 1);
  lcd.print(F("finished"));

  allLedsTest();
  lcd.clear();

  selectedSong = 0;
  changeMode(NORMAL_MODE);
}

void handleSongConfiguration(char* messageOriginal) {
  sysexBuffer[0] = (char)0;
  strcpy(sysexBuffer, messageOriginal);
  char* strings;
  strings = strtok(sysexBuffer, "|");
  if (strings == NULL) {
    return;
  }

  byte index = atoi(strings);

  strings = strtok(NULL, "|");
  if (strings == NULL) {
    return;
  }

#ifdef SONG_CONFIGURATION_DEBUG
  Serial.print(F("Received song configuration "));
  Serial.print(index);
  Serial.print(F(" "));
  Serial.println(strings);
#endif

  copyToDisplayBuffer(songs[index], strings);
}

byte getConfigurationType(char* messageOriginal) {
  strcpy(sysexBuffer, messageOriginal);
  char* strings;
  strings = strtok(sysexBuffer, "|");
  if (strings == NULL) {
    return -1;
  }
  if (strcmp(strings , "SC") == 0) {
    return SONG_CONFIGURATION;
  } else if (strcmp(strings , "CS") == 0) {
    return CONFIGURATION_START;
  } else if (strcmp(strings , "CF") == 0) {
    return CONFIGURATION_FINISHED;
  } else if (strcmp(strings, "CP") == 0) {
    return CURRENT_PART;
  } else if (strcmp(strings, "NP") == 0) {
    return NEXT_PART;
  } else if (strcmp(strings, "SS") == 0) {
    return SONG_SIGNATURE;
  }
  return -1;
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

  setupLcd();

  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);
  usbMIDI.setHandleSysEx(SystemExclusiveMessage);

#ifdef LCD_TEST
  lcdTest();
#endif
#ifdef SINGLE_LED_TEST
  ledTest();
#endif

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(displayName);
  allLedsTest();

  delay(3000);

  midiNoteSendStart = 0;
  midiNoteToSend = SEND_CONFIGURATION;
}

#ifdef LCD_TEST
void lcdTest() {
  for (int i = 0; i < 3; i++) {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
}
#endif

void setupLcd() {
  lcd.begin(16, 2);
  lcd.backlight();
}

#ifdef SINGLE_LED_TEST
void ledTest() {
  ledTest(ledPin);
  ledTest(bpmLed);
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    ledTest(ledPins[i]);
  }
  ledTest(bankDownLed);
  ledTest(bankUpLed);
  ledTest(stopLed);
  ledTest(playLed);
}
#endif

void allLedsTest() {
  for (int i = 0; i < numberOfLeds; i++) {
    digitalWrite(allLeds[i], HIGH);
  }
  delay(1000);
  for (int i = 0; i < numberOfLeds; i++) {
    digitalWrite(allLeds[i], LOW);
  }
}

void ledTest(byte pin) {
  digitalWrite(pin, HIGH);
  delay(150);
  digitalWrite(pin, LOW);
  delay(150);
}

void debugButton(Button *button) {
#ifdef DEBUG_BUTTONS
  Serial.print(F("Button: "));
  Serial.print((*button).getPin());
  Serial.print(F(" Just pressed: "));
  Serial.print((*button).isJustPressed());
  Serial.print(F(" Pressed: "));
  Serial.print((*button).isPressed());
  Serial.print(F(" Just released : "));
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
      usbMIDI.sendNoteOff(i + (initBank * numberOfChannelButtons), 0, midiChannel);
    } else if (channelButtons[i].isJustPressed()) {
      usbMIDI.sendNoteOn(i + (initBank * numberOfChannelButtons), 99, midiChannel);
      Serial.print(F("Midi-Note "));
      Serial.println(i + (initBank * numberOfChannelButtons));
    }
  }
}


void sendMidiNote() {
  if (midiNoteToSend != INVALID_MIDI_NOTE) {
    if (midiNoteSendStart > 100) {
      usbMIDI.sendNoteOff(midiNoteToSend, 0, midiChannel);
      midiNoteToSend = INVALID_MIDI_NOTE;
      startedMidiNoteSending = false;
    } else if (!startedMidiNoteSending) {
      usbMIDI.sendNoteOn(midiNoteToSend, 99, midiChannel);
      startedMidiNoteSending = true;
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
    return;
  }

  if (bothBanksDown) {
    return;
  }

  switch (mode) {
    case NORMAL_MODE:
      handleNormalMode();
      break;
    case INIT_MODE:
      handleInitMode();
      break;
    case SONG_MODE:
      handleSongMode();
      break;
    case UNCONFIGURED:
      handleUnconfigured();
      break;
  }
}

void handleUnconfigured() {
  sendMidiNote();
}

void handleSongMode() {
  if (!repeat) {
    if (playing) {
      playButton.turnLedOn();
    } else {
      playButton.turnLedOff();
    }
  }

  if (stopButton.isJustPressed()) {
    stopButton.turnLedOn();
  }

  if (stopButton.isJustReleased()) {
    midiNoteSendStart = 0;
    midiNoteToSend = STOP_MIDI_NOTE;
    bars = 0;
    changeMode(NORMAL_MODE);
    return;
  }

  if (playButton.isJustReleased()) {
    repeat = !repeat;
    repeatElapsed = 0;
    midiNoteSendStart = 0;
    midiNoteToSend = REPEAT_MIDI_NOTE;
  }

  if (repeat && repeatElapsed > 300) {
    if (playButton.isLedTurnedOn()) {
      playButton.turnLedOff();
    } else {
      playButton.turnLedOn();
    }
    repeatElapsed = 0;
  }

  if (!nextPartScheduled) {
    bankUpButton.turnLedOff();
  }

  if (triggeredElapsed > 200 && nextPartScheduled) {
    if (bankUpButton.isLedTurnedOn()) {
      bankUpButton.turnLedOff();
    } else {
      bankUpButton.turnLedOn();
    }
    triggeredElapsed = 0;
  }

  sendMidiNote();
}

void changeMode(byte newMode) {
  mode = newMode;
  resetLeds();
  if (mode == NORMAL_MODE) {
    handleNormalMode(true);
  }
}

void handleInitMode() {
  changeInitMidiBank();
  sendInitMidiNotes();
  updateInitLeds();
}

void changeInitMidiBank() {
  boolean update = false;
  if (bankDownButton.isJustReleased()) {
    initBank--;
    if (initBank == 255) {
      initBank = 126;
    }
    update = true;
  } else if (bankUpButton.isJustReleased()) {
    initBank++;
    initBank = initBank % 127;
    update = true;
  }
  if (update) {
    Serial.print(F("MIDI offset "));
    Serial.println(initBank);
  }
}

void handleNormalMode(boolean forceUpdate) {
  boolean bankChange = changeBank();
  if (bankChange) {
    selectedSong = 0;
  }
  boolean update = false;
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    if (channelButtons[i].isJustReleased()) {
      update = selectedSong != i;
      selectedSong = i;
    }
  }

  if (update || forceUpdate || bankChange) {
    for (byte i = 0; i < numberOfChannelButtons; i++) {
      channelButtons[i].turnLedOff();
    }
    channelButtons[selectedSong].turnLedOn();
    char* song = songs[selectedSong + (bank * numberOfChannelButtons)];
    lcd.clear();
    if (song) {
      lcd.print(song);
      lcd.setCursor(0, 1);
      lcd.print(F("Bank: "));
      lcd.print(bank + 1);
      lcd.print(F(" Song: "));
      lcd.print(selectedSong + 1);
    }
  }

  if (playButton.isJustPressed()) {
    playButton.turnLedOn();
  }

  if (playButton.isJustReleased()) {
    midiNoteSendStart = 0;
    midiNoteToSend = selectedSong + (bank * numberOfChannelButtons) + SONG_OFFSET;
    playButton.turnLedOff();
  }

  if (stopButton.isJustPressed()) {
    stopButton.turnLedOn();
  }

  if (stopButton.isJustReleased()) {
    if (midiNoteToSend == INVALID_MIDI_NOTE) {
      midiNoteSendStart = 0;
      midiNoteToSend = STOP_CLIPS;
    }
    stopButton.turnLedOff();
  }

  sendMidiNote();
}

void handleNormalMode() {
  handleNormalMode(false);
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

boolean changeBank() {
  boolean update = false;
  if (bankDownButton.isJustReleased()) {
    bank--;
    update = true;
    if (bank == 255) {
      bank = numberOfBanks - 1 ;
    }
  } else if (bankUpButton.isJustReleased()) {
    bank++;
    bank = bank % numberOfBanks;
    update = true;
  }
#ifdef DEBUG_BANK
  if (update) {
    Serial.print(F("Bank "));
    Serial.println(bank);
  }
#endif
  return update;
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

void handleBarChange() {
  if (mode != SONG_MODE || nextPartScheduled) {
    return;
  }
  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print(bars);
  lcd.print(F(" bars"));
}

void turnAllChannelButtonsOff() {
  for (byte i = 0; i < numberOfChannelButtons; i++) {
    channelButtons[i].turnLedOff();
  }
}

void RealTimeSystem(byte realtimebyte) {
  if (realtimebyte == CLOCK) {
    counter++;

    if (denominator > 0) {
      double tempIncrease = (tempCounter / 24.0) * (denominator / 4.0);
      double increase = (1 / 24.0) * (denominator / 4.0) + tempIncrease;
      fullDenominator = fullDenominator + increase;
      tempCounter = 0;

      currentNumerator = ((byte) fullDenominator) % numberOfChannelButtons;

      if (fullDenominator >= numerator && numerator > 0) {
        fullDenominator = 0;
        currentNumerator = 0;
        bars++;
        handleBarChange();
      }
    } else {
      tempCounter++;
    }

    if (counter == 24) {
      counter = 0;
      digitalWrite(bpmLed, HIGH);

      channelButtons[currentNumerator].turnLedOn();
    }
    if (counter == 5) {
      turnAllChannelButtonsOff();
      digitalWrite(bpmLed, LOW);
    }
  }

  if (realtimebyte == START || realtimebyte == CONTINUE) {
    counter = 0;
    digitalWrite(bpmLed, HIGH);
  }

  if (realtimebyte == START) {
    playing = true;
    repeat = false;
    fullDenominator = 0;
    currentNumerator = 0;
    changeMode(SONG_MODE);
  }

  if (realtimebyte == STOP) {
    digitalWrite(bpmLed, LOW);
    playing = false;
    bars = 0;
    currentNumerator = 0;
    denominator = 0;
    numerator = 0;
    changeMode(NORMAL_MODE);
  }
}

