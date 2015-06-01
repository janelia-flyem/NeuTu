#include "zwidgetmessage.h"

#include <QStringList>

ZWidgetMessage::ZWidgetMessage() :
  m_type(NeuTube::MSG_INFORMATION), m_appending(false)
{
}

ZWidgetMessage::ZWidgetMessage(const QString &msg, NeuTube::EMessageType type,
                               bool appending) :
  m_type(type), m_appending(appending)
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
      output = "<p>" + output + "</p>";
      break;
    case NeuTube::MSG_ERROR:
      output = "<p><font color=\"#FF0000\">" + output + "</font></p>";
      break;
    case NeuTube::MSG_WARING:
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

QString ZWidgetMessage::ToHtmlString(
    const QStringList &msgList, NeuTube::EMessageType type)
{
  QString output;

  foreach (const QString msg, msgList) {
    output += ToHtmlString(msg, type);
  }

  return output;
}
