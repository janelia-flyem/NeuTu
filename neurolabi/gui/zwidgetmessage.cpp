#include "zwidgetmessage.h"

#include <QStringList>
#include <QDateTime>

ZWidgetMessage::ZWidgetMessage()
{
}

ZWidgetMessage::ZWidgetMessage(FTargets target) : m_targets(target)
{
}

ZWidgetMessage::ZWidgetMessage(const QString &msg)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const char *msg)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const std::string &msg)
{
  m_message.append(msg.c_str());
}

ZWidgetMessage::ZWidgetMessage(const QString &title, const QString &msg) :
  m_title(title)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const QString &msg, neutu::EMessageType type) :
  m_type(type)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const char *msg, neutu::EMessageType type) :
  m_type(type)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(
    const std::string &msg, neutu::EMessageType type) :
  m_type(type)
{
  m_message.append(msg.c_str());
}

ZWidgetMessage::ZWidgetMessage(
    const QString &title, const QString &msg, neutu::EMessageType type) :
  m_title(title), m_type(type)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(
    const QString &msg, neutu::EMessageType type, FTargets target) :
  m_type(type), m_targets(target)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(
    const char *msg, neutu::EMessageType type, FTargets target) :
  m_type(type), m_targets(target)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(
    const std::string &msg, neutu::EMessageType type, FTargets target) :
  m_type(type), m_targets(target)
{
  m_message.append(msg.c_str());
}

ZWidgetMessage::ZWidgetMessage(
    const QString &title, const QString &msg, neutu::EMessageType type, FTargets target) :
  m_title(title), m_type(type), m_targets(target)
{
  m_message.append(msg);
}


bool ZWidgetMessage::hasTarget(ETarget target) const
{
  return (m_targets & target) == target;
}

bool ZWidgetMessage::hasTarget(FTargets target) const
{
  return (m_targets & target) == target;
}

bool ZWidgetMessage::hasTargetOtherThan(FTargets targets) const
{
  return (m_targets & ~targets);
}


QString ZWidgetMessage::ToHtmlString(
    const QString &msg, neutu::EMessageType type, const ToHtmlStringOption &option)
{
  QString output = msg;

  if (!output.startsWith("<p>")) {
    if (option.coloring) {
      switch (type) {
      case neutu::EMessageType::INFORMATION:
        output = "<font color = \"#007700\">" + output + "</font>";
        break;
      case neutu::EMessageType::ERROR:
        output = "<font color=\"#FF0000\">" + output + "</font>";
        break;
      case neutu::EMessageType::WARNING:
        output = "<font color=\"#777700\">" + output + "</font>";
        break;
      default:
        break;
      }
    }
    if (option.makingParagraph) {
      output = "<p>" + output + "</p>";
    }
  }

  return output;
}

QString ZWidgetMessage::toHtmlString() const
{
  return ToHtmlString(m_message, m_type);
}

QString ZWidgetMessage::toPlainString() const
{
  QString result;
  foreach (const QString &str, m_message) {
    result += str + " ";
  }

  return result;
}

QString ZWidgetMessage::ToHtmlString(
    const QStringList &msgList, neutu::EMessageType type)
{
  QString output;

  foreach (const QString msg, msgList) {
    output += ToHtmlString(msg, type, ToHtmlStringOption(type));
  }

  return output;
}

QString ZWidgetMessage::appendTime(const QString &message)
{
  return "[" + QDateTime::currentDateTime().toLocalTime().
      toString("yyyy-MM-dd hh:mm:ss") + "] " + message;
}

void ZWidgetMessage::appendMessage(const QString &message)
{
  m_message.append(message);
}

void ZWidgetMessage::setMessage(const QString &msg)
{
  m_message.clear();
  if (!msg.isEmpty()) {
    m_message.append(msg);
  }
}

bool ZWidgetMessage::hasMessage() const
{
  return !m_message.isEmpty();
}

ZWidgetMessageFactory::operator ZWidgetMessage() const
{
  return m_message;
}

ZWidgetMessageFactory::ZWidgetMessageFactory(const char *msg)
{
  m_message.setMessage(msg);
}

ZWidgetMessageFactory ZWidgetMessageFactory::Make(const char *msg)
{
  return ZWidgetMessageFactory(msg);
}

ZWidgetMessageFactory& ZWidgetMessageFactory::to(ZWidgetMessage::ETarget target)
{
  m_message.setTarget(target);

  return *this;
}

ZWidgetMessageFactory& ZWidgetMessageFactory::as(neutu::EMessageType type)
{
  m_message.setType(type);

  return *this;
}

ZWidgetMessageFactory& ZWidgetMessageFactory::title(const char *title)
{
  m_message.setTitle(title);

  return *this;
}
