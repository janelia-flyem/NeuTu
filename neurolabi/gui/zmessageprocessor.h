#ifndef ZMESSAGEPROCESSOR_H
#define ZMESSAGEPROCESSOR_H

class ZMessage;
class QWidget;

class ZMessageProcessor
{
public:
  ZMessageProcessor();

  virtual void processMessage(ZMessage *message, QWidget *host) const;
};

#endif // ZMESSAGEPROCESSOR_H
