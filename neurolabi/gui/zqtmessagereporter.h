#ifndef ZQTMESSAGEREPORTER_H
#define ZQTMESSAGEREPORTER_H

#include <QMessageBox>
#include <QWidget>
#include "zmessagereporter.h"

class ZQtMessageReporter : public ZMessageReporter
{
public:
  ZQtMessageReporter();
  ZQtMessageReporter(QWidget *parent);
  virtual ~ZQtMessageReporter();
  virtual void report(const std::string &title, const std::string &message,
                      NeuTube::EMessageType msgType);

private:
  QWidget *m_parent;
//  QMessageBox m_box;
};

#endif // ZQTMESSAGEREPORTER_H
