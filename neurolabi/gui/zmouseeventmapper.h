#ifndef ZMOUSEEVENTMAPPER_H
#define ZMOUSEEVENTMAPPER_H

#include <stdlib.h>
#include <map>
#include "zintpoint.h"
#include "zmouseevent.h"
#include "zmouseeventrecorder.h"

class ZInteractiveContext;
class QMouseEvent;
class ZMouseEvent;

class ZMouseEventMapper
{
public:
  ZMouseEventMapper(ZInteractiveContext *context = NULL);

  enum EOperation {
    OP_NULL, OP_MOVE_IMAGE, OP_MOVE_OBJECT, OP_CAPTURE_MOUSE_POSITION,
    OP_PROCESS_OBJECT, OP_RESOTRE_EXPLORE_MODE, OP_CAPTURE_IMAGE_INFO,
    OP_PAINT_STROKE, OP_START_MOVE_IMAGE,
    OP_SWC_SELECT, OP_SWC_EXTEND
  };

  /*
  enum EButton {
    LEFT_BUTTON, RIGHT_BUTTON
  };

  enum EAction {
    BUTTON_PRESS, BUTTON_RELEASE
  };
*/
  //virtual EOperation getOperation(QMouseEvent *event);
  virtual EOperation getOperation(const ZMouseEvent &event) const;
  //virtual EOperation getOperation()

  inline void setContext(ZInteractiveContext *context) {
    m_context = context;
  }

  void setPosition(int x, int y, int z, Qt::MouseButton button,
                   ZMouseEvent::EAction action);
  ZIntPoint getPosition(Qt::MouseButton button,
                        ZMouseEvent::EAction action) const;

  typedef std::map<Qt::MouseButton, std::map<ZMouseEvent::EAction, ZIntPoint> >
  TMousePosition;

  void process(QMouseEvent *event, int z);

protected:
  ZInteractiveContext *m_context;
  TMousePosition m_position;
};


class ZMouseEventLeftButtonReleaseMapper : public ZMouseEventMapper
{
public:
  EOperation getOperation(const ZMouseEvent &event) const;
};

class ZMouseEventMoveMapper : public ZMouseEventMapper
{
public:
  EOperation getOperation(const ZMouseEvent &event) const;
};

#endif // ZMOUSEEVENTMAPPER_H
