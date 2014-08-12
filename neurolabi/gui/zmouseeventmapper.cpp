#include "zmouseeventmapper.h"
#include <QMouseEvent>
#include "zinteractivecontext.h"
#include "zintpoint.h"

ZMouseEventMapper::ZMouseEventMapper(ZInteractiveContext *context) :
  m_context(context)
{
}

ZMouseEventMapper::EOperation ZMouseEventMapper::getOperation(
    QMouseEvent */*event*/)
{
  return OP_NULL;
}

////////////////////////////////////
ZMouseEventMapper::EOperation ZMouseEventLeftButtonReleaseMapper::getOperation(
    QMouseEvent *event)
{
  EOperation op = OP_NULL;
  if (m_context != NULL) {
    switch (m_context->exploreMode()) {
    case ZInteractiveContext::EXPLORE_CAPTURE_MOUSE: //It triggers a processing step
      if (event->modifiers() == Qt::ShiftModifier) {
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
    int x, int y, int z, EButton button, EAction action)
{
  m_position[button][action] = ZIntPoint(x, y, z);
}

ZIntPoint
ZMouseEventMapper::getPosition(EButton button, EAction action) const
{
  if (m_position.count(button) > 0) {
    if (m_position.at(button).count(action) > 0) {
      return m_position.at(button).at(action);
    }
  }

  return ZIntPoint();
}

/////////////////////////////////
#define MOUSE_MOVE_IMAGE_THRESHOLD 25
ZMouseEventMapper::EOperation ZMouseEventMoveMapper::getOperation(
    QMouseEvent *event)
{
  EOperation op = OP_NULL;
  if (m_context != NULL) {
    bool canMoveImage = false;

    if (event->buttons() == Qt::LeftButton) {

      if (event->modifiers() == Qt::ShiftModifier) {
        if (m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_MOVE_NODE) {
          op = OP_MOVE_OBJECT;
        }
        canMoveImage = true;
      } else {
        if (m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_EXTEND ||
            m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
          ZIntPoint pressPos = getPosition(LEFT_BUTTON, BUTTON_PRESS);
          int dx = pressPos.getX() - event->x();
          int dy = pressPos.getY() - event->y();
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
