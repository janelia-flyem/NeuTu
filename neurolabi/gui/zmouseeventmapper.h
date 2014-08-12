#ifndef ZMOUSEEVENTMAPPER_H
#define ZMOUSEEVENTMAPPER_H

#include <stdlib.h>
#include <map>
#include "zintpoint.h"

class ZInteractiveContext;
class QMouseEvent;

class ZMouseEventMapper
{
public:
  ZMouseEventMapper(ZInteractiveContext *context = NULL);

  enum EOperation {
    OP_NULL, OP_MOVE_IMAGE, OP_MOVE_OBJECT, OP_CAPTURE_MOUSE_POSITION,
    OP_PROCESS_OBJECT, OP_RESOTRE_EXPLORE_MODE, OP_CAPTURE_IMAGE_INFO,
    OP_PAINT_STROKE, OP_START_MOVE_IMAGE
  };

  enum EButton {
    LEFT_BUTTON, RIGHT_BUTTON
  };

  enum EAction {
    BUTTON_PRESS, BUTTON_RELEASE
  };

  virtual EOperation getOperation(QMouseEvent *event);

  inline void setContext(ZInteractiveContext *context) {
    m_context = context;
  }

  void setPosition(int x, int y, int z, EButton button, EAction action);
  ZIntPoint getPosition(EButton button, EAction action) const;

  typedef std::map<EButton, std::map<EAction, ZIntPoint> > TMousePosition;

protected:
  ZInteractiveContext *m_context;
  TMousePosition m_position;

};


class ZMouseEventLeftButtonReleaseMapper : public ZMouseEventMapper
{
public:
  EOperation getOperation(QMouseEvent *event);
};

class ZMouseEventMoveMapper : public ZMouseEventMapper
{
public:
  EOperation getOperation(QMouseEvent *event);
};

#endif // ZMOUSEEVENTMAPPER_H
