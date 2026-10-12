
#include "TimerOFF.h"

TimerOFF::TimerOFF(){
    startTime;
    isRunning = false;
    lastTriggerState = false;
    trigWasPressed = false;
}

void TimerOFF::update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration) {

    bool triggerPressed;
    
    if (trigger == trigState){
        triggerPressed = true;
    }else{
        triggerPressed = false;
    }

    if (triggerPressed == true) {
        timedVar = timedState;
        trigWasPressed = true;
        isRunning = false;
    }
    if(triggerPressed == false && trigWasPressed == true && !isRunning){
        startTime = millis();
        isRunning = true;
    }

    if (isRunning) {
        if (millis() - startTime >= duration) {
            timedVar = !timedState;
            trigWasPressed = false;
            isRunning = false;
    }
    }
}

void TimerOFF::reset(){
    isRunning = false;
    startTime = 0;
    trigWasPressed = false;
}
