#include "utilities.h"

#include "zlog.h"
#include "zqslog.h"
#include "common/neutudefs.h"
#include "zwidgetmessage.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  // FIXME
  KLOG("") << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString())
       << ZLog::Level(2);
}

void LogUrlIO(const QString &topic,
    const QString &action, const QString &url,  const QByteArray &payload)
{
  KLOG(topic.toStdString()) << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString())
       << ZLog::Level(2);
  if (!payload.isEmpty()) {
    KLOG(topic.toStdString()) << ZLog::Info()
         << ZLog::Description(
              QString("Payload length: %1").arg(payload.length()).toStdString())
         << ZLog::Level(2);
  }
}
//void LogBodyOperation(
//    const QString &action, uint64_t bodyId, EBodyLabelType labelType)
//{
//  KLOG(neutu::TOPIC_NULL) << ZLog::Info()
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

void LogKafkaMessage(const std::string &topic, const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_KAFKA)) {
    std::string plainStr = msg.toPlainString().toStdString();
    switch (msg.getType()) {
    case neutu::EMessageType::INFORMATION:
      KINFO(topic) << plainStr;
      break;
    case neutu::EMessageType::WARNING:
      KWARN(topic) << plainStr;
      break;
    case neutu::EMessageType::ERROR:
      KERROR(topic) << plainStr;
      break;
    case neutu::EMessageType::DEBUG:
      KDEBUG(topic) << ZLog::Debug() << ZLog::Description(plainStr);
      break;
    }
  }
}

}

void LogMessage(const std::string &topic, const ZWidgetMessage &msg)
{
  LogLocalMessage(msg);
  LogKafkaMessage(topic, msg);
}

void LogMessageF(const std::string &topic, const std::string &str, neutu::EMessageType type)
{
  ZWidgetMessage msg(str, type);
  msg.setTarget(ZWidgetMessage::TARGET_LOG_FILE | ZWidgetMessage::TARGET_KAFKA);
  LogMessage(topic, msg);
}

void LogProfileInfo(
    const std::string &topic,
    int64_t duration, const std::string &title, const std::string &info)
{
  KLOG(topic) << ZLog::Profile() << ZLog::Duration(duration)
       << ZLog::Title(title)
       << ZLog::Description(info);
}

void LogError(const std::string &topic, const std::string &msg)
{
  ZLOG(topic) << ZLog::Error() << ZLog::Description(msg);
}

}
