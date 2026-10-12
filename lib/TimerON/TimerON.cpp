
#include "TimerON.h"

TimerON::TimerON() {
    startTime = 0;
    isRunning = false;
}

void TimerON::update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration) {

    bool triggerActivated = (trigger == trigState);

    if (triggerActivated) {
        if (!isRunning) {
            startTime = millis();
            isRunning = true;
        }

         if (millis() - startTime >= duration) {
            timedVar = timedState; 
        }else {
            timedVar = !timedState; 
    }
    }else {
        isRunning = false;
        timedVar = !timedState; 
    }
}

void TimerON::reset(){
    isRunning = false;
    startTime = 0;
}
