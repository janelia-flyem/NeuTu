#ifndef ZBENCHTIMER_H
#define ZBENCHTIMER_H

#include <chrono>
#include <iostream>
#include <QDebug>

/* usage:
  bench with repeats:
    ZBenchTimer bt;
    BENCH_AND_LOG(bt,10,5,testFun(),"fun1");
  bench without repeats:
    ZBenchTimer bt;
    bt.start();
    testFun();
    STOP_AND_LOG(bt);
  */

#define BENCH_AND_LOG(TIMER, TRIES, REP, CODE, FUNCNAME) { \
  TIMER.reset(); \
  TIMER.setName(FUNCNAME); \
  for(int i=0; i<TRIES; ++i){ \
    TIMER.start(); \
    for(int j=0; j<REP; ++j){ \
      CODE; \
    } \
    TIMER.stop(); \
  } \
  LOG(INFO) << TIMER; \
}

#define STOP_AND_LOG(TIMER) { \
  TIMER.stop(); \
  LOG(INFO) << TIMER; \
}

class ZBenchTimer
{
public:
  explicit ZBenchTimer(const std::string& funName = "");

  inline void reset()
  {
    m_best = std::numeric_limits<double>::max();
    m_worst = -1;
    m_total = 0;
    m_rep = 0;
    m_time = 0;
    m_pauseTime = 0;
    m_totalPauseTime = 0;
    m_paused = false;
    m_average = 0;
    m_averagePauseTime = 0;
  }

  inline void resetAndStart(const std::string& newName)
  {
    setName(newName);
    reset();
    start();
  }

  void start();

  void stop();

  void pause();

  void resume();

  // elapsed time in seconds
  inline double time()
  { return m_time; }

  // average elapsed time in seconds.
  inline double average()
  { return m_average; }

  // best elapsed time in seconds
  inline double best()
  { return m_best; }

  // total elapsed time in seconds.
  inline double total()
  { return m_total; }

  // elapsed pause time in seconds
  inline double pauseTime()
  { return m_pauseTime; }

  // total elapsed pause time in seconds.
  inline double totalPauseTime()
  { return m_totalPauseTime; }

  inline void setName(const std::string& str)
  { m_name = str; }


protected:
  friend std::ostream& operator<<(std::ostream& s, const ZBenchTimer& m);

  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
  double m_time;
  double m_best;
  double m_worst;
  double m_average;
  int m_rep;
  double m_total;
  std::string m_name;

  double m_pauseTime;
  double m_totalPauseTime;
  double m_averagePauseTime;
  bool m_paused;
};

std::ostream& operator<<(std::ostream& s, const ZBenchTimer& m);

// qDebug output
QDebug& operator << (QDebug s, const ZBenchTimer& m);


#endif // ZBENCHTIMER_H
