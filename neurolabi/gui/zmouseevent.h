#ifndef ZMOUSEEVENT_H
#define ZMOUSEEVENT_H

#include <qnamespace.h>
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"
#include "common/neutudefs.h"
#include "data3d/zsliceviewtransform.h"

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

  void set(QMouseEvent *event, const ZSliceViewTransform &t);
  void set(QMouseEvent *event, EAction action, const ZSliceViewTransform &t);
//  void setSliceAxis(neutu::EAxis axis);
  void setSliceViewTransform(const ZSliceViewTransform &t);
  neutu::EAxis getSliceAxis() const;

  void setViewId(int id);
  int getViewId() const;
  //void setStackPosition(const ZPoint &pt);
  //void setStackPosition(double x, double y, double z);

//  void setRawStackPosition(const ZPoint &pt);
//  void setRawStackPosition(double x, double y, double z);

  void setStackPosition(const ZPoint &pt);
  void setDataPosition(const ZPoint &pt);

  void setPressEvent(QMouseEvent *event, const ZSliceViewTransform &t);
  void setReleaseEvent(QMouseEvent *event, const ZSliceViewTransform &t);
  void setMoveEvent(QMouseEvent *event, const ZSliceViewTransform &t);

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

  ZPoint getPosition(neutu::ECoordinateSystem cs) const;
  ZPoint getPosition(neutu::data3d::ESpace space) const;

  inline const ZIntPoint& getWidgetPosition() const {
    return m_widgetPosition;
  }

  /*
  inline const ZPoint& getRawStackPosition() const {
    return m_rawStackPosition;
  }
  */

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
//  ZPoint m_rawStackPosition; //If z is negative, it means a projection
  ZPoint m_stackPosition;
  ZPoint m_dataPosition;
  bool m_isInStack;
  int m_viewId = 0;
//  neutu::EAxis m_sliceAxis;
  ZSliceViewTransform m_sliceViewTransform;
};

#endif // ZMOUSEEVENT_H
