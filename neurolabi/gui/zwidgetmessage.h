#ifndef ZWIDGETMESSAGE_H
#define ZWIDGETMESSAGE_H

#include <QStringList>
#include <QObject>
#include "neutube.h"

class ZWidgetMessage
{
public:
  ZWidgetMessage();
  ZWidgetMessage(const QString &msg,
                 NeuTube::EMessageType type = NeuTube::MSG_INFORMATION,
                 bool appending = true);

  QString toHtmlString() const;
  static QString ToHtmlString(const QString &msg, NeuTube::EMessageType type);
  static QString ToHtmlString(const QStringList &msgList,
                              NeuTube::EMessageType type);

  inline bool isAppending() const {
    return m_appending;
  }

  template <typename T1, typename T2>
  static void ConnectMessagePipe(T1 *source, T2 *target, bool dumping);

private:
  QStringList m_message;
  NeuTube::EMessageType m_type;
  bool m_appending;
};

template <typename T1, typename T2>
void ZWidgetMessage::ConnectMessagePipe(T1 *source, T2 *target, bool dumping)
{
  if (dumping) {
    QObject::connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                    target, SLOT(dump(ZWidgetMessage)));
  } else {
    QObject::connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                    target, SIGNAL(messageGenerated(ZWidgetMessage)));
  }
}


#endif // ZWIDGETMESSAGE_H
