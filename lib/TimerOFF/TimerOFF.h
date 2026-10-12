#ifndef TIMER_OFF_H
#define TIMER_OFF_H

#include<Arduino.h>

class TimerOFF{
  private:
    unsigned long startTime;
    bool isRunning;
    bool lastTriggerState;
    bool trigWasPressed;

  public:
    TimerOFF();
    void update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration);
    void reset();
};

#endif