#ifndef ZMOUSEEVENT_H
#define ZMOUSEEVENT_H

#include <qnamespace.h>
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"
#include "common/neutube_def.h"

class QMouseEvent;

class ZMouseEvent
{
public:
  ZMouseEvent();

  enum class EAction {
    NONE, PRESS, RELEASE, MOVE, DOUBLE_CLICK
  };

//  enum ECoordinateSystem {
//    COORD_WIDGET, COORD_GLOBAL, COORD_RAW_STACK, COORD_STACK
//  };

  bool isNull() const;

  void set(QMouseEvent *event, int z);
  void set(QMouseEvent *event, EAction action, int z);
  void setSliceAxis(neutube::EAxis axis);
  neutube::EAxis getSliceAxis() const;

  //void setStackPosition(const ZPoint &pt);
  //void setStackPosition(double x, double y, double z);

  void setRawStackPosition(const ZPoint &pt);
  void setRawStackPosition(double x, double y, double z);

  void setStackPosition(const ZPoint &pt);
  void setDataPositoin(const ZPoint &pt);

  void setPressEvent(QMouseEvent *event, int z);
  void setReleaseEvent(QMouseEvent *event, int z);
  void setMoveEvent(QMouseEvent *event, int z);

  inline EAction getAction() const {
    return m_action;
  }

  /*
  inline Qt::MouseButton getButton() const {
    return m_button;
  }
  */

  /*!
   * \brief Get the buttons causing the event
   */
  inline Qt::MouseButtons getButtons() const {
    return m_buttons;
  }

  inline Qt::KeyboardModifiers getModifiers() const {
    return m_modifiers;
  }

  void addModifier(Qt::KeyboardModifier modifier) {
    m_modifiers |= modifier;
  }

  void removeModifier(Qt::KeyboardModifier modifier) {
    m_modifiers &= ~modifier;
  }

  ZPoint getPosition(neutube::ECoordinateSystem cs) const;

  inline const ZIntPoint& getWidgetPosition() const {
    return m_widgetPosition;
  }

  inline const ZPoint& getRawStackPosition() const {
    return m_rawStackPosition;
  }

  inline const ZPoint& getStackPosition() const {
    return m_stackPosition;
  }

  inline const ZPoint& getDataPosition() const {
    return m_dataPosition;
  }

  inline int getX() const {
    return getWidgetPosition().getX();
  }

  inline int getY() const {
    return getWidgetPosition().getY();
  }

  inline int getZ() const {
    return getWidgetPosition().getZ();
  }

  inline bool isInStack() const {
    return m_isInStack;
  }

  void setInStack(bool isInStack) {
    m_isInStack = isInStack;
  }

  void print() const;

private:
  //Qt::MouseButton m_button;
  Qt::MouseButtons m_buttons;
  EAction m_action;
  Qt::KeyboardModifiers m_modifiers;
  ZIntPoint m_widgetPosition;
  ZIntPoint m_globalPosition;
  ZPoint m_rawStackPosition; //If z is negative, it means a projection
  ZPoint m_stackPosition;
  ZPoint m_dataPosition;
  bool m_isInStack;
  neutube::EAxis m_sliceAxis;
};

#endif // ZMOUSEEVENT_H
