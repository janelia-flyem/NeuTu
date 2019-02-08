#include "utilities.h"

#include "zlog.h"
#include "zqslog.h"
#include "common/neutube_def.h"
#include "zwidgetmessage.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString());
}

void LogBodyOperation(const QString &action, uint64_t bodyId)
{
  KLOG << ZLog::Info()
       << ZLog::Action(action.toStdString())
       << ZLog::Object("body", "", std::to_string(bodyId));
}

namespace {

void LogLocalMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_LOG_FILE)) {
    QString plainStr = msg.toPlainString();
    switch (msg.getType()) {
    case neutu::EMessageType::INFORMATION:
      LINFO_NLN() << plainStr;
      break;
    case neutu::EMessageType::WARNING:
      LWARN_NLN() << plainStr;
      break;
    case neutu::EMessageType::ERROR:
      LERROR_NLN() << plainStr;
      break;
    case neutu::EMessageType::DEBUG:
      LDEBUG_NLN() << plainStr;
      break;
    }
  }
}

void LogKafkaMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_KAFKA)) {
    std::string plainStr = msg.toPlainString().toStdString();
    switch (msg.getType()) {
    case neutu::EMessageType::INFORMATION:
      KINFO << plainStr;
      break;
    case neutu::EMessageType::WARNING:
      KWARN << plainStr;
      break;
    case neutu::EMessageType::ERROR:
      KERROR << plainStr;
      break;
    case neutu::EMessageType::DEBUG:
      KDEBUG << ZLog::Debug() << ZLog::Description(plainStr);
      break;
    }
  }
}

}

void LogMessage(const ZWidgetMessage &msg)
{
  LogLocalMessage(msg);
  LogKafkaMessage(msg);
}

}
