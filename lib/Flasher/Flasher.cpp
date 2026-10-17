
#include"Flasher.h"

Flasher::Flasher(unsigned long currentInterval) {
    previousMillis = 0;
    interval = currentInterval;
    running = false;
    idleState = false;
    remainingToggles = 0;
    totalCount = -1;     // Initialize default
}

bool Flasher::start(bool startIdleState, int count) {
    previousMillis = millis();
    idleState = startIdleState;
    running = true;
    totalCount = count; // NEW: Save the count for future resets
    
    if (count > 0) {
        remainingToggles = count * 2; 
    } else {
        remainingToggles = -1; 
    }
    return true;
}

bool Flasher::resetCount(bool modifier) {
   // if (!running) {
   //     return false; // Don't reset if it isn't currently active
   // }
    
    previousMillis = millis(); // Reset the time window so the next flash isn't premature
    
    if (totalCount > 0 && modifier) {
        remainingToggles = totalCount * 2; // Restore original countdown
    }else if(!modifier){
        running = true;
        remainingToggles = totalCount * 2;
    }else if (totalCount < 0){
        remainingToggles = -1;             // Keep infinite flashing if it was infinite
    }
    return true;
}

bool Flasher::flash(bool &variableToFlash) {
    if (!running) {
        variableToFlash = idleState;
    return false;
    }
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        variableToFlash = !variableToFlash; 

    if (remainingToggles > 0) {
        remainingToggles--;
        if (remainingToggles == 0) {
            stop(variableToFlash); 
        }
    }

    return true; 
    }
    return false; 
}

void Flasher::setInterval(unsigned long newInterval) {
    interval = newInterval;
}

bool Flasher::stop(bool &variableToStop) {
    running = false;
    remainingToggles = 0;
    variableToStop = idleState;
    return true;
}

bool Flasher::isRunning() {
    return running;
}