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
    TARGET_NULL = 0,
    TARGET_TEXT = BIT_FLAG(1),
    TARGET_TEXT_APPENDING = 0x3,
    TARGET_DIALOG = BIT_FLAG(3),
    TARGET_STATUS_BAR = BIT_FLAG(4),
    TARGET_CUSTOM_AREA = BIT_FLAG(5),
    TARGET_LOG_FILE = BIT_FLAG(6),
    TARGET_KAFKA = BIT_FLAG(7)
  };

  Q_DECLARE_FLAGS(FTargets, ETarget)

  ZWidgetMessage();

  explicit ZWidgetMessage(FTargets target);

  explicit ZWidgetMessage(const std::string &msg);
  explicit ZWidgetMessage(const char *msg);
  explicit ZWidgetMessage(const QString &msg);
  explicit ZWidgetMessage(const QString &title, const QString &msg);

  explicit ZWidgetMessage(
      const std::string &msg, neutu::EMessageType type);
  explicit ZWidgetMessage(const char *msg, neutu::EMessageType type);
  explicit ZWidgetMessage(const QString &msg, neutu::EMessageType type);
  explicit ZWidgetMessage(const QString &title, const QString &msg,
                          neutu::EMessageType type);

  explicit ZWidgetMessage(
      const std::string &msg, neutu::EMessageType type, FTargets target);
  explicit ZWidgetMessage(
      const char *msg, neutu::EMessageType type, FTargets target);
  explicit ZWidgetMessage(
      const QString &msg, neutu::EMessageType type, FTargets target);
  explicit ZWidgetMessage(const QString &title, const QString &msg,
                          neutu::EMessageType type, FTargets target);


  QString toHtmlString() const;
  struct ToHtmlStringOption {
    ToHtmlStringOption(bool c, bool m) {
      coloring = c;
      makingParagraph = m;
    }
    ToHtmlStringOption(neutu::EMessageType type) {
      if (type == neutu::EMessageType::INFORMATION) {
        this->coloring = false;
        this->makingParagraph = false;
      }
    }
    bool coloring = true;
    bool makingParagraph = true;
  };

  static QString ToHtmlString(
      const QString &msg, neutu::EMessageType type,
      const ToHtmlStringOption &option);
  static QString ToHtmlString(const QStringList &msgList,
                              neutu::EMessageType type);
  QString toPlainString() const;

  inline bool isAppending() const { return hasTarget(TARGET_TEXT_APPENDING); }

//  inline ETarget getTarget() const {
//    return m_target;
//  }

  bool hasTarget(ETarget target) const;
  bool hasTarget(FTargets targets) const;
  bool hasTargetOtherThan(FTargets targets) const;

  inline neutu::EMessageType getType() const {
    return m_type;
  }

  inline void setTarget(ETarget target) {
    m_targets = target;
  }

  inline void setTarget(FTargets target) {
    m_targets = target;
  }

  inline void setType(neutu::EMessageType type) {
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
  neutu::EMessageType m_type = neutu::EMessageType::INFORMATION;
//  bool m_appending;
//  ETarget m_target;
  FTargets m_targets = QFlags<ZWidgetMessage::ETarget>(TARGET_TEXT_APPENDING | TARGET_KAFKA);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ZWidgetMessage::FTargets)

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
  ZWidgetMessageFactory& as(neutu::EMessageType type);
  ZWidgetMessageFactory& title(const char *title);

private:
  ZWidgetMessage m_message;
};

#endif // ZWIDGETMESSAGE_H
