#ifndef ZWIDGETMESSAGE_H
#define ZWIDGETMESSAGE_H

#include <QStringList>
#include <QObject>
#include <string>

#include "neutube.h"

class ZWidgetMessage
{
public:
  enum ETarget {
    TARGET_TEXT, TARGET_TEXT_APPENDING, TARGET_DIALOG, TARGET_STATUS_BAR,
    TARGET_CUSTOM_AREA, TARGET_LOG_FILE
  };

  Q_DECLARE_FLAGS(FTargets, ETarget)

  ZWidgetMessage(ETarget target = TARGET_TEXT_APPENDING);
  explicit ZWidgetMessage(const std::string &msg,
                 neutube::EMessageType type = neutube::EMessageType::INFORMATION,
                 ETarget target = TARGET_TEXT_APPENDING);
  explicit ZWidgetMessage(const char *msg,
                 neutube::EMessageType type = neutube::EMessageType::INFORMATION,
                 ETarget target = TARGET_TEXT_APPENDING);
  explicit ZWidgetMessage(const QString &msg,
                 neutube::EMessageType type = neutube::EMessageType::INFORMATION,
                 ETarget target = TARGET_TEXT_APPENDING);
  explicit ZWidgetMessage(const QString &title, const QString &msg,
                 neutube::EMessageType type = neutube::EMessageType::INFORMATION,
                 ETarget target = TARGET_TEXT_APPENDING);

  QString toHtmlString() const;
  static QString ToHtmlString(const QString &msg, neutube::EMessageType type);
  static QString ToHtmlString(const QStringList &msgList,
                              neutube::EMessageType type);
  QString toPlainString() const;

  inline bool isAppending() const { return m_target == TARGET_TEXT_APPENDING; }

  inline ETarget getTarget() const {
    return m_target;
  }

  inline neutube::EMessageType getType() const {
    return m_type;
  }

  inline void setTarget(ETarget target) {
    m_target = target;
  }

  inline void setType(neutube::EMessageType type) {
    m_type = type;
  }

  inline void setTitle(const QString &title) {
    m_title = title;
  }

  inline const QString &getTitle() const { return m_title; }

  template <typename T1, typename T2>
  static void ConnectMessagePipe(T1 *source, T2 *target);

  template <typename T1, typename T2>
  static void DisconnectMessagePipe(T1 *source, T2 *target);

  //Obsolete API
  template <typename T1, typename T2>
  static void ConnectMessagePipe(T1 *source, T2 *target, bool dumping);

  static QString appendTime(const QString &message);

  void appendMessage(const QString &message);
  void setMessage(const QString &msg);

  bool hasMessage() const;

private:
  QString m_title;
  QStringList m_message;
  neutube::EMessageType m_type;
//  bool m_appending;
  ETarget m_target;
};

template <typename T1, typename T2>
void ZWidgetMessage::ConnectMessagePipe(
    T1 *source, T2 *target, bool dumping)
{
  if (dumping) {
    QObject::connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                    target, SLOT(dump(ZWidgetMessage)));
  } else {
    QObject::connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                    target, SIGNAL(messageGenerated(ZWidgetMessage)));
  }
}

template <typename T1, typename T2>
void ZWidgetMessage::ConnectMessagePipe(T1 *source, T2 *target)
{
  QObject::connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                   target, SLOT(processMessage(ZWidgetMessage)));
}

template <typename T1, typename T2>
void ZWidgetMessage::DisconnectMessagePipe(T1 *source, T2 *target)
{
  QObject::disconnect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
                   target, SLOT(processMessage(ZWidgetMessage)));
}

struct ZWidgetMessageFactory
{
  ZWidgetMessageFactory(const char *msg);
  operator ZWidgetMessage() const;

  static ZWidgetMessageFactory Make(const char *msg);

  ZWidgetMessageFactory& to(ZWidgetMessage::ETarget target);
  ZWidgetMessageFactory& as(neutube::EMessageType type);
  ZWidgetMessageFactory& title(const char *title);

private:
  ZWidgetMessage m_message;
};

#endif // ZWIDGETMESSAGE_H
