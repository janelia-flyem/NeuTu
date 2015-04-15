#ifndef ZMESSAGEPROCESSOR_H
#define ZMESSAGEPROCESSOR_H

#include <string>

class ZMessage;
class QWidget;

class ZMessageProcessor
{
public:
  ZMessageProcessor();

  virtual void processMessage(ZMessage *message, QWidget *host) const;

  virtual std::string toString() const;
  virtual void print() const;
};

#endif // ZMESSAGEPROCESSOR_H
