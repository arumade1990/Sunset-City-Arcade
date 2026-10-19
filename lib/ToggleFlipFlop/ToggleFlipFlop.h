#ifndef TOGGLE_FLIP_FLOP_H
#define TOGGLE_FLIP_FLOP_H

#include<Arduino.h>

class ToggleFlipFlop {
  private:
    bool lastToggleState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay; 

  public:
  ToggleFlipFlop();
    void update(bool &targetVar, bool toggleInput, bool togglePressedState);
    void reset(bool &targetVar);
};

#endif