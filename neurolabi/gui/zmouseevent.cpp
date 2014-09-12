#include "zmouseevent.h"
#include <QMouseEvent>

ZMouseEvent::ZMouseEvent() : m_buttons(Qt::NoButton),
  m_action(ZMouseEvent::ACTION_NONE), m_modifiers(Qt::NoModifier)
{
}

/*
void ZMouseEvent::setStackPosition(const ZPoint &pt)
{
  setStackPosition(pt.x(), pt.y(), pt.z());
}

void ZMouseEvent::setStackPosition(double x, double y, double z)
{
  m_stackPosition.set(x, y, z);
}
*/

void ZMouseEvent::setRawStackPosition(const ZPoint &pt)
{
  setRawStackPosition(pt.x(), pt.y(), pt.z());
}

void ZMouseEvent::setRawStackPosition(double x, double y, double z)
{
  m_rawStackPosition.set(x, y, z);
}


void ZMouseEvent::set(QMouseEvent *event, int z)
{
  //m_button = event->button();
  m_buttons = event->buttons();
  m_action = ACTION_NONE;
  m_modifiers = event->modifiers();
  m_position.set(event->x(), event->y(), z);
}

void ZMouseEvent::set(QMouseEvent *event, EAction action, int z)
{
  set(event, z);
  m_action = action;
  if (action == ACTION_RELEASE) {
    m_buttons = event->button();
  }
}

void ZMouseEvent::setPressEvent(QMouseEvent *event, int z)
{
  set(event, ACTION_PRESS, z);
}

void ZMouseEvent::setMoveEvent(QMouseEvent *event, int z)
{
  set(event, ACTION_MOVE, z);
}

void ZMouseEvent::setReleaseEvent(QMouseEvent *event, int z)
{
  set(event, ACTION_RELEASE, z);
}

bool ZMouseEvent::isNull() const
{
  return m_action == ACTION_NONE;
}
