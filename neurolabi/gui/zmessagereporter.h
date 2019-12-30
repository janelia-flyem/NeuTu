#ifndef ZMESSAGEPORTER_H
#define ZMESSAGEPORTER_H

#include <string>
#include <ostream>

#include "common/neutudefs.h"

class ZMessageReporter
{
public:
  ZMessageReporter();
  virtual ~ZMessageReporter() {}

  /*
  enum EMessageType {
    Information, Warning, Error, Debug
  };
  */

  virtual void report(const std::string &title, const std::string &message,
                      neutu::EMessageType msgType);

  static void report(std::ostream &stream,
                     const std::string &title, const std::string &message,
                     neutu::EMessageType msgType);

private:
  static int m_count;
};

#endif // ZERRORREPORTER_H
