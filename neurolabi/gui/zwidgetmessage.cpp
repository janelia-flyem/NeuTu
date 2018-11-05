#include "zwidgetmessage.h"

#include <QStringList>
#include <QDateTime>

ZWidgetMessage::ZWidgetMessage(ETarget target) :
  m_type(neutube::EMessageType::INFORMATION), m_target(target)
{
}

ZWidgetMessage::ZWidgetMessage(const QString &msg, neutube::EMessageType type,
                               ETarget target) :
  m_type(type), m_target(target)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const char *msg, neutube::EMessageType type,
                               ETarget target) :
  m_type(type), m_target(target)
{
  m_message.append(msg);
}

ZWidgetMessage::ZWidgetMessage(const std::string &msg, neutube::EMessageType type,
                               ETarget target) :
  m_type(type), m_target(target)
{
  m_message.append(msg.c_str());
}

ZWidgetMessage::ZWidgetMessage(
    const QString &title, const QString &msg,
    neutube::EMessageType type, ETarget target) :
  m_title(title), m_type(type), m_target(target)
{
  m_message.append(msg);
}

QString ZWidgetMessage::ToHtmlString(
    const QString &msg, neutube::EMessageType type)
{
  QString output = msg;

  if (!output.startsWith("<p>")) {
    switch (type) {
    case neutube::EMessageType::INFORMATION:
//      output += "<font color = \"#007700\">test</font>";
//      output = "<p style=\" margin-top:0px;\">" + output + "</p>";
      break;
    case neutube::EMessageType::ERROR:
      output = "<p><font color=\"#FF0000\">" + output + "</font></p>";
      break;
    case neutube::EMessageType::WARNING:
      output = "<p><font color=\"#777700\">" + output + "</font></p>";
      break;
    default:
      break;
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
    const QStringList &msgList, neutube::EMessageType type)
{
  QString output;

  foreach (const QString msg, msgList) {
    output += ToHtmlString(msg, type);
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

ZWidgetMessageFactory& ZWidgetMessageFactory::as(neutube::EMessageType type)
{
  m_message.setType(type);

  return *this;
}

ZWidgetMessageFactory& ZWidgetMessageFactory::title(const char *title)
{
  m_message.setTitle(title);

  return *this;
}
