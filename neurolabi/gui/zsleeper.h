#ifndef ZSLEEPER_H
#define ZSLEEPER_H

#include <QThread>

class ZSleeper : public QThread
{
public:
  ZSleeper();

public:
  static void usleep(unsigned long usecs){QThread::usleep(usecs);}
  static void msleep(unsigned long msecs){QThread::msleep(msecs);}
  static void sleep(unsigned long secs){QThread::sleep(secs);}
};

#endif // ZSLEEPER_H
