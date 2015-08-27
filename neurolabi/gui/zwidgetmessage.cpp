#include "zwidgetmessage.h"

#include <QStringList>
#include <QDateTime>

ZWidgetMessage::ZWidgetMessage() :
  m_type(NeuTube::MSG_INFORMATION), m_target(TARGET_TEXT)
{
}

ZWidgetMessage::ZWidgetMessage(const QString &msg, NeuTube::EMessageType type,
                               ETarget target) :
  m_type(type), m_target(target)
{
  m_message.append(msg);
}

QString ZWidgetMessage::ToHtmlString(
    const QString &msg, NeuTube::EMessageType type)
{
  QString output = msg;

  if (!output.startsWith("<p>")) {
    switch (type) {
    case NeuTube::MSG_INFORMATION:
//      output += "<font color = \"#007700\">test</font>";
//      output = "<p style=\" margin-top:0px;\">" + output + "</p>";
      break;
    case NeuTube::MSG_ERROR:
      output = "<p><font color=\"#FF0000\">" + output + "</font></p>";
      break;
    case NeuTube::MSG_WARNING:
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
    const QStringList &msgList, NeuTube::EMessageType type)
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
