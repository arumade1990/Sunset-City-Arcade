#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h> // Gives the file access to millis() and types like unsigned long

class Timer {
  private:
    unsigned long previousMillis;
    unsigned long interval;
    bool running;

  public:
    Timer(unsigned long currentInterval);
    void start();
    bool isRunning();
    void stop();
    void setInterval(unsigned long newInterval);
    bool isReady();
};

#endif
