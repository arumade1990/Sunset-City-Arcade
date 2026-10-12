#ifndef ONE_SHOT_PULSE_H
#define ONE_SHOT_PULSE_H

#include <Arduino.h>

class OneShotPulse {
  private:
    unsigned long startTime;
    bool isRunning;
    bool lastTriggerState;

  public:
    OneShotPulse();
    void update(bool &pulsedVar, bool pulsedState, bool trigger, bool trigState, unsigned long duration);
    void reset();
};

#endif