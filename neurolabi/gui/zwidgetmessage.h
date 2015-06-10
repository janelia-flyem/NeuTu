#ifndef ZWIDGETMESSAGE_H
#define ZWIDGETMESSAGE_H

#include <QStringList>
#include <QObject>
#include "neutube.h"

class ZWidgetMessage
{
public:
  enum ETarget {
    TARGET_TEXT, TARGET_TEXT_APPENDING, TARGET_DIALOG
  };


  ZWidgetMessage();
  ZWidgetMessage(const QString &msg,
                 NeuTube::EMessageType type = NeuTube::MSG_INFORMATION,
                 ETarget target = TARGET_TEXT_APPENDING);

  QString toHtmlString() const;
  static QString ToHtmlString(const QString &msg, NeuTube::EMessageType type);
  static QString ToHtmlString(const QStringList &msgList,
                              NeuTube::EMessageType type);

  inline bool isAppending() const { return m_target == TARGET_TEXT_APPENDING; }

  inline ETarget getTarget() const {
    return m_target;
  }

  inline void setTarget(ETarget target) {
    m_target = target;
  }

  template <typename T1, typename T2>
  static void ConnectMessagePipe(T1 *source, T2 *target, bool dumping);

  static QString appendTime(const QString &message);

private:
  QStringList m_message;
  NeuTube::EMessageType m_type;
//  bool m_appending;
  ETarget m_target;
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
