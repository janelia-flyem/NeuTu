#include "zwidgetmessage.h"

#include <QStringList>

ZWidgetMessage::ZWidgetMessage() : m_type(NeuTube::MSG_INFORMATION)
{
}

ZWidgetMessage::ZWidgetMessage(const QString &msg, NeuTube::EMessageType type) :
  m_message(msg), m_type(type)
{

}

QString ZWidgetMessage::toHtmlString(
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
  return toHtmlString(m_message, m_type);
}

QString ZWidgetMessage::toHtmlString(
    const QStringList &msgList, NeuTube::EMessageType type)
{
  QString output;

  foreach (const QString msg, msgList) {
    output += toHtmlString(msg, type);
  }

  return output;
}
