#ifndef ZWIDGETMESSAGE_H
#define ZWIDGETMESSAGE_H

#include <QString>
#include "neutube.h"

class ZWidgetMessage
{
public:
  ZWidgetMessage();
  ZWidgetMessage(const QString &msg, NeuTube::EMessageType type);

  QString toHtmlString() const;
  static QString toHtmlString(const QString &msg, NeuTube::EMessageType type);
  static QString toHtmlString(const QStringList &msgList,
                              NeuTube::EMessageType type);

private:
  QString m_message;
  NeuTube::EMessageType m_type;
};

#endif // ZWIDGETMESSAGE_H
