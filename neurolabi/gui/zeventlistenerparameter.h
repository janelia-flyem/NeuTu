#ifndef ZEVENTLISTENERPARAMETER_H
#define ZEVENTLISTENERPARAMETER_H

#include "widgets/zparameter.h"
#include <QList>
#include <QMouseEvent>
#include <QKeyEvent>

/*!
 * \brief The parameter class of listening events
 *
 * The class is designed to listen to mouse or keyboard events. It is set up by
 * the \a listenTo function and used by calling the \a sendEvent function. When
 * an event matches one in the listening list, a triggered signal will be emitted.
 *
 * Unlike other paramter classes, this class cannot be used to create widgets.
 */
class ZEventListenerParameter : public ZParameter
{
Q_OBJECT
public:
  explicit ZEventListenerParameter(const QString& name, bool sharing = false, QObject* parent = nullptr);

  inline void setSharing(bool s)
  { if (m_sharing != s) { m_sharing = s; emit valueChanged(); }}

  inline bool isSharing() const
  { return m_sharing; }

  // buttons and modifiers should be exact match with the input to trigger the event signal
  // An OR-combinations of buttons means all buttons should be pressed at the time
  // An OR-combinations of modifiers means all modifiers should be pressed at the time
  void
  listenTo(const QString& actionName, const Qt::MouseButtons& buttons, const Qt::KeyboardModifiers& modifiers,
           QEvent::Type type);

  //
  void listenTo(const QString& actionName, Qt::Key key, const Qt::KeyboardModifiers& modifiers,
                QEvent::Type type = QEvent::KeyPress);

  void clearAll();

  void sendEvent(QEvent* e, int w, int h);

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

  virtual void setValueSameAs(const ZParameter& /*rhs*/) override;

signals:

  void eventTriggered(QEvent* e, int w, int h);

  void mouseEventTriggered(QMouseEvent* e, int w, int h);

  void keyEventTriggered(QKeyEvent* e, int w, int h);

  void wheelEventTriggered(QWheelEvent* e, int w, int h);

protected:
  struct MouseEvent
  {
    MouseEvent(const QString& actionName_, Qt::MouseButtons buttons_,
               Qt::KeyboardModifiers modifiers_, QEvent::Type type_)
      : actionName(actionName_), buttons(buttons_), modifiers(modifiers_), type(type_)
    {}

    QString actionName;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QEvent::Type type;
  };

  struct KeyEvent
  {
    KeyEvent(const QString& actionName_, Qt::Key key_, Qt::KeyboardModifiers modifiers_,
             QEvent::Type type_ = QEvent::KeyPress)
      : actionName(actionName_), key(key_), modifiers(modifiers_), type(type_)
    {}

    QString actionName;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers;
    QEvent::Type type;
  };

  virtual QWidget* actualCreateWidget(QWidget* parent) override;

private:
  bool m_sharing;
  QList<MouseEvent> m_mouseEvents;
  QList<KeyEvent> m_keyEvents;
};

#endif // ZEVENTLISTENERPARAMETER_H
