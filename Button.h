#ifndef Button_h
#define Button_h

#include <Bounce2.h>
#include "Arduino.h"

class Button {
  public:
    // Class Variable


    // Public Attributes


    // Constructors
    Button();

    // Destructors
    ~Button();

    // Public functions
    boolean update();
    void init(byte pin);
    void init(byte pin, byte led);
    boolean isPressed();
    boolean isJustPressed();
    boolean isJustReleased();
    byte getPin();
    void turnLedOn();
    void turnLedOff();

  private:
    // Private Attributes
    byte _pin;
    byte _led;
    Bounce _button;
    byte _previousState;
    boolean _justReleased;
    boolean _justPressed;
    boolean _pressed;
    // Private Functions


};



#endif


