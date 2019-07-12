#include "zloggable.h"

#include <iostream>

ZLoggable::ZLoggable()
{
  m_logger = [](const std::string &str, neutu::EMessageType type) {
    switch (type) {
    case neutu::EMessageType::ERROR:
      std::cout << "ERROR: ";
      break;
    case neutu::EMessageType::WARNING:
      std::cout << "WARNING: ";
      break;
    case neutu::EMessageType::DEBUG:
      std::cout << "DEBUG: ";
      break;
    default:
      break;
    }

    std::cout << str << std::endl;
  };
}

void ZLoggable::addLog(const std::string &msg, neutu::EMessageType type, int level)
{
  if (m_verboseLevel >= level) {
    m_logger(msg, type);
  }
}

void ZLoggable::setLogger(LogFunc logger)
{
  m_logger = logger;
}
