#ifndef TIMER_ON_H
#define TIMER_ON_H
#include<Arduino.h>

class TimerON {
  private:
    unsigned long startTime;
    bool isRunning;

  public:
    TimerON();
    void update(bool &timedVar, bool timedState, bool trigger, bool trigState, unsigned long duration);

    void reset();
};

#endif