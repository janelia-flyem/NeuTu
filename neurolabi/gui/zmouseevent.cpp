#include "zmouseevent.h"
#include <QMouseEvent>

ZMouseEvent::ZMouseEvent() : m_buttons(Qt::NoButton),
  m_action(ZMouseEvent::EAction::NONE), m_modifiers(Qt::NoModifier),
  m_isInStack(false)
{
//  m_sliceAxis = neutu::EAxis::Z;
}

ZPoint ZMouseEvent::getPosition(neutu::ECoordinateSystem cs) const
{
  switch (cs) {
  case neutu::ECoordinateSystem::WIDGET:
    return m_widgetPosition.toPoint();
  case neutu::ECoordinateSystem::STACK:
    return m_stackPosition;
//  case neutu::ECoordinateSystem::RAW_STACK:
//    return m_rawStackPosition;
  case neutu::ECoordinateSystem::SCREEN:
    return m_globalPosition.toPoint();
  case neutu::ECoordinateSystem::ORGDATA:
    return m_dataPosition;
  default:
    break;
  }

  return ZPoint(0, 0, 0);
}

ZPoint ZMouseEvent::getPosition(neutu::data3d::ESpace space) const
{
  switch (space) {
  case neutu::data3d::ESpace::MODEL:
    return m_dataPosition;
  case neutu::data3d::ESpace::VIEW:
    return m_stackPosition;
  case neutu::data3d::ESpace::CANVAS:
    return m_widgetPosition.toPoint();
  }

  return ZPoint(0, 0, 0);
}

/*
void ZMouseEvent::setRawStackPosition(const ZPoint &pt)
{
  setRawStackPosition(pt.x(), pt.y(), pt.z());
}

void ZMouseEvent::setRawStackPosition(double x, double y, double z)
{
  m_rawStackPosition.set(x, y, z);
}
*/

void ZMouseEvent::setStackPosition(const ZPoint &pt)
{
  m_stackPosition = pt;
}

void ZMouseEvent::setDataPosition(const ZPoint &pt)
{
  m_dataPosition = pt;
#ifdef _DEBUG_
  if (getAction() == ZMouseEvent::EAction::RELEASE) {
    std::cout << "setDataPosition: " << m_dataPosition.toString() << std::endl;
  }
#endif
}

void ZMouseEvent::set(QMouseEvent *event, const ZSliceViewTransform &t)
{
  //m_button = event->button();
  m_buttons = event->buttons();
  m_action = EAction::NONE;
  m_modifiers = event->modifiers();
  m_widgetPosition.set(event->x(), event->y(), 0);
  m_globalPosition.set(event->globalX(), event->globalY(), 0);
  m_sliceViewTransform = t;
}

void ZMouseEvent::set(QMouseEvent *event, EAction action, const ZSliceViewTransform &t)
{
  set(event, t);
  m_action = action;
  if (action == EAction::RELEASE) {
    m_buttons = event->button();
  }
}

neutu::EAxis ZMouseEvent::getSliceAxis() const
{
  return m_sliceViewTransform.getSliceAxis();
}

void ZMouseEvent::setSliceViewTransform(const ZSliceViewTransform &t)
{
  m_sliceViewTransform = t;
}

/*
void ZMouseEvent::setSliceAxis(neutu::EAxis axis)
{
  m_sliceAxis = axis;
}
*/

void ZMouseEvent::setPressEvent(QMouseEvent *event, const ZSliceViewTransform &t)
{
  set(event, EAction::PRESS, t);
}

void ZMouseEvent::setMoveEvent(QMouseEvent *event, const ZSliceViewTransform &t)
{
  set(event, EAction::MOVE, t);
}

void ZMouseEvent::setReleaseEvent(QMouseEvent *event, const ZSliceViewTransform &t)
{
  set(event, EAction::RELEASE, t);
}

bool ZMouseEvent::isNull() const
{
  return m_action == EAction::NONE;
}

void ZMouseEvent::print() const
{
  std::cout << "Mouse event: ";
  if (getButtons() & Qt::LeftButton) {
    std::cout << "Left button; ";
  }
  if (getButtons() & Qt::RightButton) {
    std::cout << "Right button; ";
  }
  if (getButtons() & Qt::MidButton) {
    std::cout << "Middle button; ";
  }

  switch (getAction()) {
  case EAction::DOUBLE_CLICK:
    std::cout << "Double click; ";
    break;
  case EAction::MOVE:
    std::cout << "Move; ";
    break;
  case EAction::PRESS:
    std::cout << "Press; ";
    break;
  case EAction::RELEASE:
    std::cout << "Release; ";
    break;
  default:
    break;
  }

  if (getModifiers() & Qt::ShiftModifier) {
    std::cout << "Shift; ";
  }
  if (getModifiers() & Qt::ControlModifier) {
    std::cout << "Control; ";
  }
  if (getModifiers() & Qt::AltModifier) {
    std::cout << "Alt; ";
  }

  std::cout << std::endl;
}
