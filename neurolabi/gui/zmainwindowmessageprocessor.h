#ifndef ZMAINWINDOWMESSAGEPROCESSOR_H
#define ZMAINWINDOWMESSAGEPROCESSOR_H

#include "zmessageprocessor.h"

class ZMainWindowMessageProcessor : public ZMessageProcessor
{
public:
  ZMainWindowMessageProcessor();

  void processMessage(ZMessage *message, QWidget *host) const;

};

#endif // ZMAINWINDOWMESSAGEPROCESSOR_H
