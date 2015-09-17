#include "zmainwindowmessageprocessor.h"
#include "zmessage.h"
#include "mainwindow.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

ZMainWindowMessageProcessor::ZMainWindowMessageProcessor()
{
}

void ZMainWindowMessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
  if (message == NULL) {
    return;
  }

  MainWindow *realHost = qobject_cast<MainWindow*>(host);
  if (realHost != NULL) {
    switch (message->getType()) {
    case ZMessage::TYPE_INFORMATION:
    {
      ZJsonObject messageBody = message->getMessageBody();
      std::string title = ZJsonParser::stringValue(messageBody["title"]);
      std::string msg = ZJsonParser::stringValue(messageBody["body"]);
      realHost->report(title, msg, NeuTube::MSG_INFORMATION);

      message->deactivate();
    }
      break;
    default:
      break;
    }
  }
}
