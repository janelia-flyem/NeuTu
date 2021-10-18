#include "utilities.h"

#include "zlog.h"
#include "zqslog.h"
#include "common/neutudefs.h"
#include "zwidgetmessage.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString())
       << ZLog::Level(2);
}

void LogUrlIO(
    const QString &action, const QString &url,  const QByteArray &payload)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString())
       << ZLog::Level(2);
  if (!payload.isEmpty()) {
    KLOG << ZLog::Info()
         << ZLog::Description(
              QString("Payload length: %1").arg(payload.length()).toStdString())
         << ZLog::Level(2);
  }
}
//void LogBodyOperation(
//    const QString &action, uint64_t bodyId, EBodyLabelType labelType)
//{
//  KLOG << ZLog::Info()
//       << ZLog::Action(action.toStdString())
//       << ZLog::Object(neutu::ToString(labelType), "", std::to_string(bodyId));
//}

namespace {

void LogLocalMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_LOG_FILE)) {
    QString plainStr = msg.toPlainString();
    if (!plainStr.isEmpty()) {
      switch (msg.getType()) {
      case neutu::EMessageType::INFORMATION:
        LINFO_NLN() << plainStr.toStdString().c_str();
        break;
      case neutu::EMessageType::WARNING:
        LWARN_NLN() << plainStr.toStdString().c_str();
        break;
      case neutu::EMessageType::ERROR:
        LERROR_NLN() << plainStr.toStdString().c_str();
        break;
      case neutu::EMessageType::DEBUG:
        LDEBUG_NLN() << plainStr.toStdString().c_str();
        break;
      }
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

void LogMessageF(const std::string &str, neutu::EMessageType type)
{
  ZWidgetMessage msg(str, type);
  msg.setTarget(ZWidgetMessage::TARGET_LOG_FILE | ZWidgetMessage::TARGET_KAFKA);
  LogMessage(msg);
}

void LogProfileInfo(int64_t duration, const std::string &info)
{
  KLOG << ZLog::Profile() << ZLog::Duration(duration)
       << ZLog::Description(info);
}

void LogError(const std::string &msg)
{
  ZLOG << ZLog::Error() << ZLog::Description(msg);
}

}
