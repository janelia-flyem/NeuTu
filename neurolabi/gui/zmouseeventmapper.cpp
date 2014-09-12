#include "zmouseeventmapper.h"
#include <QMouseEvent>
#include "zinteractivecontext.h"
#include "zintpoint.h"
#include "zmouseevent.h"

ZMouseEventMapper::ZMouseEventMapper(ZInteractiveContext *context) :
  m_context(context)
{
}

#if 0
ZMouseEventMapper::EOperation ZMouseEventMapper::getOperation(
    QMouseEvent */*event*/)
{
  return OP_NULL;
}
#endif

ZMouseEventMapper::EOperation ZMouseEventMapper::getOperation(
    const ZMouseEvent &/*event*/) const
{
  return OP_NULL;
}
////////////////////////////////////
ZMouseEventMapper::EOperation ZMouseEventLeftButtonReleaseMapper::getOperation(
    const ZMouseEvent &event) const
{
  EOperation op = OP_NULL;
  if (m_context != NULL) {
    switch (m_context->exploreMode()) {
    case ZInteractiveContext::EXPLORE_CAPTURE_MOUSE: //It triggers a processing step
      if (event.getModifiers() == Qt::ShiftModifier) {
        op = OP_CAPTURE_MOUSE_POSITION;
      }
      break;
    case ZInteractiveContext::EXPLORE_OFF:
      op = OP_PROCESS_OBJECT;
      break;
    case ZInteractiveContext::EXPLORE_MOVE_IMAGE:
      op = OP_RESOTRE_EXPLORE_MODE;
      break;
    default:
      break;
    }
  }

  return op;
}

void ZMouseEventMapper::setPosition(
    int x, int y, int z, Qt::MouseButton button, ZMouseEvent::EAction action)
{
  m_position[button][action] = ZIntPoint(x, y, z);
}

ZIntPoint
ZMouseEventMapper::getPosition(Qt::MouseButton button,
                               ZMouseEvent::EAction action) const
{
  ZIntPoint pt;

  if (m_position.count(button) > 0) {
    if (m_position.at(button).count(action) > 0) {
      pt = m_position.at(button).at(action);
    }
  }

  return pt;
}

/////////////////////////////////
#define MOUSE_MOVE_IMAGE_THRESHOLD 25
ZMouseEventMapper::EOperation ZMouseEventMoveMapper::getOperation(
    const ZMouseEvent &event) const
{
  EOperation op = OP_NULL;
  if (m_context != NULL) {
    bool canMoveImage = false;

    if (event.getButtons() == Qt::LeftButton) {
      if (event.getModifiers() == Qt::ShiftModifier) {
        if (m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_MOVE_NODE) {
          op = OP_MOVE_OBJECT;
        }
        canMoveImage = true;
      } else {
        if (m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_EXTEND ||
            m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
          ZIntPoint pressPos =
              getPosition(Qt::LeftButton, ZMouseEvent::ACTION_PRESS);
          int dx = pressPos.getX() - event.getX();
          int dy = pressPos.getY() - event.getY();
          if (dx * dx + dy * dy > MOUSE_MOVE_IMAGE_THRESHOLD) {
            canMoveImage = true;
          }
        } else {
          canMoveImage = true;
        }
      }

      if (op == OP_NULL) {
        if (m_context->strokeEditMode() == ZInteractiveContext::STROKE_DRAW) {
          op = OP_PAINT_STROKE;
        }
      }

      if (op == OP_NULL) {
        if (canMoveImage) {
          if (m_context->exploreMode() == ZInteractiveContext::EXPLORE_MOVE_IMAGE) {
            op = OP_MOVE_IMAGE;
          } else {
            op = OP_START_MOVE_IMAGE;
          }
        }
      }
    }

    if (op == OP_NULL) {
      op = OP_CAPTURE_IMAGE_INFO;
    }
  }

  return op;
}
