#include "ToggleFlipFlop.h"

ToggleFlipFlop::ToggleFlipFlop() {
    lastToggleState = false;
    lastDebounceTime = 0;
    debounceDelay = 50;
} 

void ToggleFlipFlop::update(bool &targetVar, bool toggleInput, bool togglePressedState) {

    bool isTogglePressed = (toggleInput == togglePressedState);

    if (isTogglePressed != lastToggleState) {
        if ((millis() - lastDebounceTime) > debounceDelay) {

            if (isTogglePressed) {
                targetVar = !targetVar; // Flip the state
                }
                lastDebounceTime = millis();
                lastToggleState = isTogglePressed;
        }
    }
}

void ToggleFlipFlop::reset(bool &targetVar){
    targetVar = false; 
    lastToggleState = false; 
}
