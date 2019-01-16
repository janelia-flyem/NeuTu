#include "utilities.h"

#include "zlog.h"
#include "neutube_def.h"
#include "zwidgetmessage.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString());
}

void LogMessage(const ZWidgetMessage &msg)
{
  std::string plainStr = msg.toPlainString().toStdString();
  switch (msg.getType()) {
  case neutube::EMessageType::INFORMATION:
    KLog() << ZLog::Info() << ZLog::Description(plainStr);
    break;
  case neutube::EMessageType::WARNING:
    KLog() << ZLog::Warn() << ZLog::Description(plainStr);
//    LWARN() << msg.toPlainString();
    break;
  case neutube::EMessageType::ERROR:
    KLog() << ZLog::Error() << ZLog::Description(plainStr);
//    LERROR() << msg.toPlainString();
    break;
  case neutube::EMessageType::DEBUG:
    KDEBUG << ZLog::Debug() << ZLog::Description(plainStr);
//    LDEBUG() << msg.toPlainString();
    break;
  }
}

}
