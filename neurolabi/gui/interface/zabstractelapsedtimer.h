#ifndef ZABSTRACTELAPSEDTIMER_H
#define ZABSTRACTELAPSEDTIMER_H

#include <cstdint>

class ZAbstractElapsedTimer
{
public:
  ZAbstractElapsedTimer();

  virtual void start() = 0;
  virtual void restart() = 0;
  virtual int64_t elapsed() = 0;
};

#endif // ZABSTRACTELAPSEDTIMER_H
