#ifndef FLASHER_H
#define FLASHER_H

#include<Arduino.h>

class Flasher {
  private:
    unsigned long previousMillis;
    unsigned long interval;
    bool running;
    bool idleState;
    int remainingToggles; 
    int totalCount;       // NEW: Remembers the original starting count

  public:
    Flasher(unsigned long currentInterval);

    bool start(bool startIdleState, int count = -1);

    bool resetCount();

    bool flash(bool &variableToFlash);

    void setInterval(unsigned long newInterval);

    bool stop();

    bool isRunning();
};

#endif