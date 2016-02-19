#ifndef ZMOUSEEVENT_H
#define ZMOUSEEVENT_H

#include <qnamespace.h>
#include "zintpoint.h"
#include "zpoint.h"
#include "neutube.h"

class QMouseEvent;

class ZMouseEvent
{
public:
  ZMouseEvent();

  enum EAction {
    ACTION_NONE, ACTION_PRESS, ACTION_RELEASE, ACTION_MOVE, ACTION_DOUBLE_CLICK
  };

//  enum ECoordinateSystem {
//    COORD_WIDGET, COORD_GLOBAL, COORD_RAW_STACK, COORD_STACK
//  };

  bool isNull() const;

  void set(QMouseEvent *event, int z);
  void set(QMouseEvent *event, EAction action, int z);

  //void setStackPosition(const ZPoint &pt);
  //void setStackPosition(double x, double y, double z);

  void setRawStackPosition(const ZPoint &pt);
  void setRawStackPosition(double x, double y, double z);

  void setStackPosition(const ZPoint &pt);

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

  inline const ZIntPoint& getPosition() const {
    return m_position;
  }

  ZPoint getPosition(NeuTube::ECoordinateSystem cs) const;

  inline int getX() const {
    return getPosition().getX();
  }

  inline int getY() const {
    return getPosition().getY();
  }

  inline int getZ() const {
    return getPosition().getZ();
  }

  inline const ZPoint& getRawStackPosition() const {
    return m_rawStackPosition;
  }

  inline const ZPoint& getStackPosition() const {
    return m_stackPosition;
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
  ZIntPoint m_position;
  ZIntPoint m_globalPosition;
  ZPoint m_rawStackPosition; //If z is negative, it means a projection
  ZPoint m_stackPosition;
  bool m_isInStack;
};

#endif // ZMOUSEEVENT_H
