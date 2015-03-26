#ifndef ZBODYSPLITBUTTON_H
#define ZBODYSPLITBUTTON_H

#include <QPushButton>

#include "zmessageprocessor.h"

class ZMessageManager;

class ZBodySplitButton : public QPushButton
{
  Q_OBJECT
public:
  explicit ZBodySplitButton(QWidget *parent = 0);

  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager(ZMessageManager *parent = NULL);

signals:

public slots:
  void requestSplit();

private:
  ZMessageManager *m_messageManager;
};

#endif // ZBODYSPLITBUTTON_H
