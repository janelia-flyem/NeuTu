#ifndef ZLOGGABLE_H
#define ZLOGGABLE_H

#include "common/neutudefs.h"

/*!
 * \brief The interace class for logging
 */
class ZLoggable
{
public:
  ZLoggable();

  void addLog(const std::string &msg,
           neutu::EMessageType type = neutu::EMessageType::INFORMATION,
           int level = 1);
  void setVerboseLevel(int level) {
    m_verboseLevel = level;
  }

  using LogFunc =
    std::function<void(const std::string&, neutu::EMessageType)>;

  void setLogger(LogFunc logger);

private:
  int m_verboseLevel = 1; //the larger the value, the more verbose.
  LogFunc m_logger;
};

#endif // ZLOGGABLE_H
