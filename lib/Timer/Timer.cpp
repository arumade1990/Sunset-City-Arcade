#include "Timer.h" // Links this file back to its blueprint folder

Timer::Timer(unsigned long currentInterval) {
  interval = currentInterval;
  previousMillis = 0;
  running = false;
}

void Timer::start() {
  previousMillis = millis();
  running = true;
}

bool Timer::isRunning() {
  return running;
}

void Timer::stop() {
  running = false;
}

void Timer::setInterval(unsigned long newInterval) {
  interval = newInterval;
}

bool Timer::isReady() {
  if (!running) return false;
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 
    return true;
  }
  return false;
}
