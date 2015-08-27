#ifndef ZREPORTABLE_H
#define ZREPORTABLE_H

#include <string>
#include "zmessagereporter.h"

class ZMessageReporter;

class ZReportable
{
public:
  ZReportable();
  virtual ~ZReportable();

  inline void setReporter(ZMessageReporter *reporter) {
    m_reporter = reporter;
  }

  void destroyReporter();

  void report(const std::string &title, const std::string &msg,
              NeuTube::EMessageType msgType = NeuTube::MSG_INFORMATION);

private:
  ZMessageReporter m_defaultReporter;
  ZMessageReporter *m_reporter;
};

#endif // ZREPORTABLE_H
