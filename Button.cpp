#include "Button.h"

/**
 * Standard-Constructor
 *
 */
Button::Button() {
}

/**
 * Standard-Destructor
 *
 */
Button::~Button() {
}



void Button::init(byte pin, byte led) {
    init(pin);
    pinMode(led, OUTPUT);
    _led = led;
}

void Button::init(byte pin) {
    pinMode(pin, INPUT_PULLUP);
    _button = Bounce();
    _button.attach(pin);
    _button.interval(10);
    _previousState = HIGH;
    _pin = pin;
}

boolean Button::update() {
    boolean change = false;
    if(_justPressed || _justReleased) {
        change = true;
    }
    _justReleased = false;
    _justPressed = false;
    if (_button.update()) {
        byte currentState = _button.read();

        if(currentState != _previousState) {
            if (_pressed && currentState == HIGH) {
                _justReleased = true;
            }
            else if (!_pressed && currentState == LOW) {
                _justPressed = true;
            }
            _pressed = !currentState;
            change = true;
        }

        _previousState = currentState;
    }
    return change;
}

boolean Button::isPressed() {
    return _pressed;
}

boolean Button::isJustPressed() {
    return _justPressed;
}

boolean Button::isJustReleased() {
    return _justReleased;
}

byte Button::getPin() {
    return _pin;
}

void Button::turnLedOn() {
    if(_led) {
        digitalWrite(_led, HIGH);
        _ledOn = true;
    }
}

void Button::turnLedOff() {
    if(_led) {
        digitalWrite(_led, LOW);
        _ledOn = false;
    }
}

boolean Button::isLedTurnedOn() {
  return _ledOn;
}

