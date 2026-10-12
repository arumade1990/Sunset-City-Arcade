
#include "OneShotPulse.h"

OneShotPulse::OneShotPulse() {
    startTime;
    isRunning = false;
    lastTriggerState = false;
}

void OneShotPulse::update(bool &pulsedVar, bool pulsedState, bool trigger, bool trigState, unsigned long duration) {
    bool triggerActivated = (trigger == trigState && lastTriggerState != trigState);
    lastTriggerState = trigger;

    if (triggerActivated && !isRunning) {
        pulsedVar = pulsedState;
        startTime = millis();
        isRunning = true;
    }

    if (isRunning) {
        if (millis() - startTime >= duration) {
            pulsedVar = !pulsedState;
            isRunning = false;
        }
    }
}

void OneShotPulse::reset(){
    isRunning = false;
    lastTriggerState = false;
}
