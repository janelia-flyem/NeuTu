#include "zeventlistenerparameter.h"

#include <QLabel>
#include <QEvent>

ZEventListenerParameter::ZEventListenerParameter(const QString& name, bool sharing, QObject* parent)
  : ZParameter(name, parent)
  , m_sharing(sharing)
{
}

void
ZEventListenerParameter::listenTo(const QString& actionName, const Qt::MouseButtons& buttons,
                                  const Qt::KeyboardModifiers& modifiers,
                                  QEvent::Type type)
{
  m_mouseEvents.push_back(MouseEvent(actionName, buttons, modifiers, type));
  emit valueChanged();
}

void ZEventListenerParameter::listenTo(const QString& actionName, Qt::Key key, const Qt::KeyboardModifiers& modifiers,
                                       QEvent::Type type)
{
  m_keyEvents.push_back(KeyEvent(actionName, key, modifiers, type));
  emit valueChanged();
}

void ZEventListenerParameter::clearAll()
{
  m_mouseEvents.clear();
  m_keyEvents.clear();
  emit valueChanged();
}

void ZEventListenerParameter::sendEvent(QEvent* e, int w, int h)
{
  if (!m_isWidgetsEnabled)
    return;

  if (QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(e)) {
    bool accept = false;
    //LOG(INFO) << mouseEvent->modifiers() << " " << mouseEvent->button() << " " << mouseEvent->buttons();
    for (const auto& me : m_mouseEvents) {
      accept = true;
      accept &= (mouseEvent->modifiers() == me.modifiers);
      accept &= (mouseEvent->type() == me.type);
      if (mouseEvent->type() == QEvent::MouseMove)
        accept &= ((mouseEvent->buttons() == me.buttons));
      else
        accept &= ((mouseEvent->button() == me.buttons));
      if (accept)
        break;
    }
    if (accept) {
      emit eventTriggered(e, w, h);
      emit mouseEventTriggered(mouseEvent, w, h);

      if (m_sharing)
        e->ignore();
    }
  } else if (QWheelEvent* wheelEvent = dynamic_cast<QWheelEvent*>(e)) {
    bool accept = false;
    for (const auto& me : m_mouseEvents) {
      accept = true;
      accept &= (wheelEvent->modifiers() == me.modifiers);
      accept &= (wheelEvent->type() == me.type);
      accept &= ((wheelEvent->buttons() == me.buttons));
      if (accept)
        break;
    }
    if (accept) {
      emit eventTriggered(e, w, h);
      emit wheelEventTriggered(wheelEvent, w, h);

      if (m_sharing)
        e->ignore();
    }
  } else if (QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(e)) {
    bool accept = false;
    for (const auto& ke : m_keyEvents) {
      accept = true;
      accept &= (keyEvent->modifiers() == ke.modifiers);
      accept &= (keyEvent->type() == ke.type);
      accept &= (keyEvent->key() == ke.key);
      if (accept)
        break;
    }
    if (accept) {
      emit eventTriggered(e, w, h);
      emit keyEventTriggered(keyEvent, w, h);

      if (m_sharing)
        e->ignore();
    }
  }
}

QWidget* ZEventListenerParameter::actualCreateWidget(QWidget* parent)
{
  // todo: interface?
  return new QLabel("Place holder", parent);
}

void ZEventListenerParameter::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const ZEventListenerParameter* src = static_cast<const ZEventListenerParameter*>(&rhs);
  m_sharing = src->m_sharing;
  m_mouseEvents = src->m_mouseEvents;
  m_keyEvents = src->m_keyEvents;
  ZParameter::setSameAs(rhs);
}

void ZEventListenerParameter::setValueSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
}

void ZEventListenerParameter::interpolate(const ZParameter& prev, double progress, ZParameter& dest)
{
  Q_UNUSED(progress)
  CHECK(this->isSameType(prev) && this->isSameType(dest));
}
