#include "zbenchtimer.h"

#include "QsLog.h"
#include <sstream>

ZBenchTimer::ZBenchTimer(const std::string& funName)
  : m_name(funName)
{
  reset();
}

void ZBenchTimer::start()
{
  m_time = 0.0;
  m_pauseTime = 0.0;
  m_paused = false;
  m_start = std::chrono::high_resolution_clock::now();
}

void ZBenchTimer::stop()
{
  double elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_start).count();
  if (m_paused) {
    m_pauseTime += elapsed;
  } else {
    m_time += elapsed;
  }

  m_paused = false;

  m_best = std::min(m_best, m_time);
  m_worst = std::max(m_worst, m_time);
  m_rep++;
  m_total += m_time;
  m_average = m_total / m_rep;
  m_totalPauseTime += m_pauseTime;
  m_averagePauseTime = m_totalPauseTime / m_rep;
}

void ZBenchTimer::pause()
{
  if (m_paused)
    return;

  m_time += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_start).count();
  m_paused = true;
  m_start = std::chrono::high_resolution_clock::now();
}

void ZBenchTimer::resume()
{
  if (!m_paused)
    return;

  m_pauseTime += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_start).count();
  m_paused = false;
  m_start = std::chrono::high_resolution_clock::now();
}

std::ostream& operator<<(std::ostream& s, const ZBenchTimer& m)
{
  if (m.m_rep == 1) {
    s << "Function " << m.m_name << " took " << m.m_time << " seconds.\n";
    if (m.m_pauseTime > 0)
      s << "Function " << m.m_name << " pause " << m.m_pauseTime << " seconds.\n";
  } else if (m.m_rep > 1) {
    s << "Function " << m.m_name << " took on average " << m.m_average << " seconds.";
    s << " (out of " << m.m_rep << " repeats. best: " << m.m_best << "  worst: ";
    s << m.m_worst << ")\n";
    if (m.m_averagePauseTime > 0)
      s << "Function " << m.m_name << " pause on average " << m.m_averagePauseTime << " seconds.\n";
  }
  return s;
}

// qDebug output
QDebug& operator << (QDebug s, const ZBenchTimer& m)
{
  std::ostringstream oss;
  oss << m;
  std::string outstr = oss.str();
  // remove newline at last
  outstr.erase(outstr.end()-1);
  s.nospace() << outstr.c_str();
  return s.space();
}
