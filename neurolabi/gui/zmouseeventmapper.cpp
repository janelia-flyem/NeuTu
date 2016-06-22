#include "zmouseeventmapper.h"
#include <QMouseEvent>
#include "zinteractivecontext.h"
#include "zintpoint.h"
#include "zmouseevent.h"
#include "zstackdoc.h"
#include "neutube.h"
#include "zstackoperator.h"
#include "zstack.hxx"
#include "zstackdochittest.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyemproofdoc.h"

ZMouseEventMapper::ZMouseEventMapper(
    ZInteractiveContext *context, ZStackDoc *doc) :
  m_context(context), m_doc(doc), m_eventRecorder(NULL)
{
}

#if 0
ZMouseEventMapper::EOperation ZMouseEventMapper::getOperation(
    QMouseEvent */*event*/)
{
  return OP_NULL;
}
#endif

ZStackOperator ZMouseEventMapper::getOperation(
    const ZMouseEvent &/*event*/) const
{
  return ZStackOperator();
}

ZStackOperator ZMouseEventMapper::initOperation() const
{
  ZStackOperator op;
  op.setMouseEventRecorder(m_eventRecorder);

  return op;
}

/*
void ZMouseEventMapper::setPosition(
    int x, int y, int z, Qt::MouseButton button, ZMouseEvent::EAction action)
{
  m_position[button][action] = ZIntPoint(x, y, z);
}
*/

ZIntPoint
ZMouseEventMapper::getPosition(Qt::MouseButton button,
                               ZMouseEvent::EAction action) const
{
  ZIntPoint pt;

  if (m_eventRecorder != NULL) {
    ZMouseEvent event = m_eventRecorder->getMouseEvent(button, action);
    pt = event.getPosition();
  }
  /*
  else {
    if (m_position.count(button) > 0) {
      if (m_position.at(button).count(action) > 0) {
        pt = m_position.at(button).at(action);
      }
    }
  }
  */

  return pt;
}
///////////////ZMouseEventLeftButtonReleaseMapper/////////////////////
////////             UO             ////////////////////////////
void ZMouseEventLeftButtonReleaseMapper::processSelectionOperation(
    ZStackOperator &op, const ZMouseEvent &event) const
{
  if (op.getHitObject<Swc_Tree_Node>() != NULL) { //SWC select operation
    if (event.getModifiers() == Qt::NoModifier) {
      op.setOperation(ZStackOperator::OP_SWC_SELECT_SINGLE_NODE);
    } else if (event.getModifiers() == Qt::ShiftModifier) {
      op.setOperation(ZStackOperator::OP_SWC_SELECT_CONNECTION);
    } else if (event.getModifiers() == Qt::ControlModifier) {
      if (getDocument()->isSwcNodeSelected(
            op.getHitObject<Swc_Tree_Node>())) {
        op.setOperation(ZStackOperator::OP_SWC_DESELECT_SINGLE_NODE);
      } else {
        op.setOperation(ZStackOperator::OP_SWC_SELECT_MULTIPLE_NODE);
      }
    } else if (event.getModifiers() & Qt::AltModifier) {
      op.setOperation(ZStackOperator::OP_SWC_SELECT_FLOOD);
    }
  } else if (op.getHitObject<ZStackObject>() != NULL) {
    switch (op.getHitObject()->getType()) {
    case ZStackObject::TYPE_STROKE:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_STROKE_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier ||
                 event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_STROKE_SELECT_MULTIPLE);
      }
      break;
    case ZStackObject::TYPE_OBJ3D:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier ||
                 event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SELECT_MULTIPLE);
      }
      break;
    case ZStackObject::TYPE_PUNCTUM:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_PUNCTA_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier ||
                 event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_PUNCTA_SELECT_MULTIPLE);
      }
      break;
    case ZStackObject::TYPE_OBJECT3D_SCAN:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_SELECT_MULTIPLE);
      } else if (event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT);
      }
      break;
    case ZStackObject::TYPE_DVID_LABEL_SLICE:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_SELECT_MULTIPLE);
      } else if (event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT);
      }
      break;
    case ZStackObject::TYPE_FLYEM_BOOKMARK:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_BOOKMARK_SELECT_SIGNLE);
      } else if (event.getModifiers() == Qt::ShiftModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT_SELECT_MULTIPLE);
      } else if (event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT_SELECT_MULTIPLE);
      }
      break;
    case ZStackObject::TYPE_DVID_SYNAPE_ENSEMBLE:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier) {
        op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_SELECT_MULTIPLE);
      } else if (event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_SELECT_TOGGLE);
      }
      break;
    default:
      if (event.getModifiers() == Qt::NoModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT_SELECT_SINGLE);
      } else if (event.getModifiers() == Qt::ShiftModifier ||
                 event.getModifiers() == Qt::ControlModifier) {
        op.setOperation(ZStackOperator::OP_OBJECT_SELECT_MULTIPLE);
      }
      break;
    }
  }
}

ZStackOperator ZMouseEventLeftButtonReleaseMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  if (m_context != NULL) {
    switch (m_context->exploreMode()) {
    case ZInteractiveContext::EXPLORE_CAPTURE_MOUSE: //It triggers a processing step
      if (event.getModifiers() == Qt::ShiftModifier) {
        op.setOperation(ZStackOperator::OP_CAPTURE_MOUSE_POSITION);
      }
      break;
    case ZInteractiveContext::EXPLORE_MOVE_IMAGE:
      op.setOperation(ZStackOperator::OP_RESTORE_EXPLORE_MODE);
      break;
    default:
      break;
    }

    if (op.isNull()) {
      switch (m_context->getUniqueMode()) {
      case ZInteractiveContext::INTERACT_STROKE_DRAW:
        op.setOperation(ZStackOperator::OP_STROKE_ADD_NEW);
        break;
      case ZInteractiveContext::INTERACT_RECT_DRAW:
        if (event.getModifiers() == Qt::ShiftModifier) {
          op.setOperation(ZStackOperator::OP_RECT_ROI_APPEND);
        } else {
          op.setOperation(ZStackOperator::OP_RECT_ROI_ACCEPT);
        }
//        op.setOperation(ZStackOperator::OP_EXIT_EDIT_MODE);
        break;
      case ZInteractiveContext::INTERACT_ADD_BOOKMARK:
        op.setOperation(ZStackOperator::OP_BOOKMARK_ADD_NEW);
        break;
      case ZInteractiveContext::INTERACT_ADD_SYNAPSE:
        op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_ADD);
        break;
      case ZInteractiveContext::INTERACT_MOVE_SYNAPSE:
        op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_MOVE);
        break;
      default:
        break;
      }
    }

    if (op.isNull()) {
      ZPoint rawStackPosition = event.getRawStackPosition();
      ZPoint stackPosition = rawStackPosition + getDocument()->getStackOffset();
      if (m_doc->getStack() != NULL) {
        if (m_doc->getStack()->containsRaw(rawStackPosition)) {
          bool hitTestOn =
              (m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT ||
               m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_CONNECT ||
               m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_EXTEND ||
               m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF) &&
              m_context->strokeEditMode() == ZInteractiveContext::STROKE_EDIT_OFF;
          if (hitTestOn) {
            ZStackDocHitTest hitManager;

            if (m_context->isObjectProjectView()) {
              hitManager.hitTest(const_cast<ZStackDoc*>(getDocument()),
                                 stackPosition.x(), stackPosition.y(),
                                 m_context->getSliceAxis());
            } else {
              hitManager.hitTest(const_cast<ZStackDoc*>(getDocument()),
                                 stackPosition);
            }
            op.setHitObject(hitManager.getHitObject<ZStackObject>());

            bool selectionOn =
                ((m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT ||
                 m_context->swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF) &&
                 m_context->strokeEditMode() == ZInteractiveContext::STROKE_EDIT_OFF);

            if (selectionOn) {
              if (op.getHitObject() != NULL) {
                if (op.getHitObject()->isSelectable()) {
                  processSelectionOperation(op, event);
                }
              }
            }
          }
        }
      }
    }

    if (op.isNull()) {
      switch (m_context->swcEditMode()) {
      case ZInteractiveContext::SWC_EDIT_SELECT:
        if (event.getModifiers() == Qt::NoModifier) {
          if (!getDocument()->hasObjectSelected()) {
            if (getDocument()->getTag() == NeuTube::Document::NORMAL) {
              op.setOperation(ZStackOperator::OP_SHOW_TRACE_CONTEXT_MENU);
            }
          } else {
            op.setOperation(ZStackOperator::OP_DESELECT_ALL);
          }
        }
        break;
      case ZInteractiveContext::SWC_EDIT_CONNECT:
        op.setOperation(ZStackOperator::OP_SWC_CONNECT_TO);
        break;
      case ZInteractiveContext::SWC_EDIT_EXTEND:
        if (event.getModifiers() == Qt::ControlModifier) {
          op.setOperation(ZStackOperator::OP_SWC_EXTEND);
        } else {
          if (op.getHitObject<Swc_Tree_Node>() != NULL) {
//            if (op.getHitObject()->isSelectable()) {
              processSelectionOperation(op, event);
//            }
          }

          if (op.isNull()) {
            op.setOperation(ZStackOperator::OP_SWC_SMART_EXTEND);
          }
        }
        break;
      case ZInteractiveContext::SWC_EDIT_ADD_NODE:
        op.setOperation(ZStackOperator::OP_SWC_ADD_NODE);
        break;
      default:
        break;
      }
    }

    if (op.isNull()) {
      if (m_context->isStackSliceView()) {
        if (m_context->isContextMenuActivated() &&
            m_context->markPuncta() &&
            getDocument()->hasStackData()) {
          op.setOperation(ZStackOperator::OP_SHOW_PUNCTA_MENU);
        }
      }
    }
  }

  if (op.isNull()) {
    op.setOperation(ZStackOperator::OP_CUSTOM_MOUSE_RELEASE);
  }

  return op;
}

///////////////ZMouseEventLeftButtonDoubleClickMapper/////////////////
////////             :O             ////////////////////////////
ZStackOperator ZMouseEventLeftButtonDoubleClickMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();

  ZPoint stackPosition = event.getStackPosition();

//  if (event.getRawStackPosition().z() >= 0) {
//    op.setHitSwcNode(m_doc->swcHitTest(stackPosition));
//  } else {
//    op.setHitSwcNode(m_doc->swcHitTest(stackPosition.x(), stackPosition.y()));
//  }
  ZStackDocHitTest hitManager;
  if (event.getRawStackPosition().z() < 0) {
    hitManager.hitTest(const_cast<ZStackDoc*>(
                         getDocument()), stackPosition.x(), stackPosition.y(),
                       m_context->getSliceAxis());
  } else {
    hitManager.hitTest(const_cast<ZStackDoc*>(getDocument()), stackPosition);
  }
  //op.setHitSwcNode(hitManager.getHitObject<Swc_Tree_Node>());
  op.setHitObject(hitManager.getHitObject<ZStackObject>());


  if (op.getHitObject<Swc_Tree_Node>() != NULL) {
    op.setOperation(ZStackOperator::OP_SWC_LOCATE_FOCUS);
  } else if (op.getHitObject() != NULL) {
    if (op.getHitObject()->getType() == ZStackObject::TYPE_STROKE) {
      if (m_context->isObjectProjectView()) {
        op.setOperation(ZStackOperator::OP_STROKE_LOCATE_FOCUS);
      }
    } else if (op.getHitObject()->getType() == ZStackObject::TYPE_OBJ3D) {
      if (m_context->isObjectProjectView()) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_LOCATE_FOCUS);
      }
    } else if (op.getHitObject()->getType() == ZStackObject::TYPE_OBJECT3D_SCAN) {
      if (m_context->isObjectProjectView()) {
        op.setOperation(ZStackOperator::OP_OBJECT3D_SCAN_LOCATE_FOCUS);
      }
    } else if (op.getHitObject()->getType() == ZStackObject::TYPE_DVID_SPARSE_STACK) {
      if (m_context->isObjectProjectView()) {
        op.setOperation(ZStackOperator::OP_DVID_SPARSE_STACK_LOCATE_FOCUS);
      }
    } else if (op.getHitObject()->getType() == ZStackObject::TYPE_FLYEM_BOOKMARK) {
      op.setOperation(ZStackOperator::OP_BOOKMARK_ANNOTATE);
    }
  }

  if (op.isNull()) {
    if (event.isInStack()) {
      if (m_context->isProjectView()) {
        if (m_doc->hasStackData()) {
          op.setOperation(ZStackOperator::OP_STACK_LOCATE_SLICE);
        } else {
          op.setOperation(ZStackOperator::OP_STACK_VIEW_SLICE);
        }
      } else {
        if (getDocument()->getTag() != NeuTube::Document::FLYEM_PROOFREAD &&
            getDocument()->getStack()->depth() > 1) {
          if (event.getModifiers() == Qt::ShiftModifier) {
            op.setOperation(ZStackOperator::OP_STACK_VIEW_PROJECTION);
          }
        }
      }
    }
  }

  return op;
}

///////////////ZMouseEventLeftButtonPressMapper/////////////////
////////             -O             ////////////////////////////
ZStackOperator ZMouseEventLeftButtonPressMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  op.setPressedButtons(event.getButtons());

  if (event.isInStack()) {
    switch (m_context->getUniqueMode()) {
    case ZInteractiveContext::INTERACT_STROKE_DRAW:
      op.setOperation(ZStackOperator::OP_STROKE_START_PAINT);
      break;
    case ZInteractiveContext::INTERACT_RECT_DRAW:
      op.setOperation(ZStackOperator::OP_RECT_ROI_INIT);
      break;
    default:
      break;
    }
  }

  return op;
}

///////////////ZMouseEventLeftRightButtonPressMapper/////////////////
////////             --             ////////////////////////////
ZStackOperator ZMouseEventLeftRightButtonPressMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  op.setPressedButtons(event.getButtons());

  if (event.isInStack()) {
    switch (m_context->getUniqueMode()) {
    /*
    case ZInteractiveContext::INTERACT_STROKE_DRAW:
      op.setOperation(ZStackOperator::OP_STROKE_START_PAINT);
      break;
    case ZInteractiveContext::INTERACT_RECT_DRAW:
      op.setOperation(ZStackOperator::OP_RECT_ROI_INIT);
      break;
      */
    default:
      break;
    }
  }

  return op;
}

///////////////ZMouseEventMiddleButtonPressMapper/////////////////
////////             O-O             ////////////////////////////
ZStackOperator ZMouseEventMiddleButtonPressMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  op.setPressedButtons(event.getButtons());

  if (event.isInStack()) {
    switch (m_context->getUniqueMode()) {
    /*
    case ZInteractiveContext::INTERACT_STROKE_DRAW:
      op.setOperation(ZStackOperator::OP_STROKE_START_PAINT);
      break;
    case ZInteractiveContext::INTERACT_RECT_DRAW:
      op.setOperation(ZStackOperator::OP_RECT_ROI_INIT);
      break;
      */
    default:
      break;
    }
  }

  return op;
}


///////////////ZMouseEventRightButtonReleaseMapper/////////////////////
////////             OU             ////////////////////////////
ZStackOperator
ZMouseEventRightButtonReleaseMapper::getOperation(const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  if (m_context != NULL && m_doc.get() != NULL) {
    if (event.getButtons() == Qt::RightButton) {
      if (m_context->isContextMenuActivated()) {
        if (m_doc->hasSelectedSwcNode()) {
          op.setOperation(ZStackOperator::OP_SHOW_SWC_CONTEXT_MENU);
        } else if (m_doc->getTag() == NeuTube::Document::FLYEM_MERGE) {
          if (m_doc->getSelected(ZStackObject::TYPE_OBJECT3D_SCAN).size() == 1) {
            op.setOperation(ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU);
          }
        } else if (m_doc->getTag() == NeuTube::Document::BIOCYTIN_PROJECTION) {
            op.setOperation(ZStackOperator::OP_SHOW_STROKE_CONTEXT_MENU);
        }

        if (m_doc->getTag() == NeuTube::Document::FLYEM_PROOFREAD ||
            m_doc->getTag() == NeuTube::Document::FLYEM_ORTHO) {
          op.setOperation(ZStackOperator::OP_SHOW_CONTEXT_MENU);
#if 0
          ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(m_doc.get());
          if (doc != NULL) {
            ZDvidLabelSlice *slice = doc->getDvidLabelSlice(NeuTube::Z_AXIS);
            if (slice != NULL) {
              if (slice->getSelected(NeuTube::BODY_LABEL_MAPPED).size() == 1) {
                op.setOperation(ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU);
              }
            }
          }
#endif
        }

        if (op.isNull()) {
          op.setOperation(ZStackOperator::OP_SHOW_CONTEXT_MENU);
        }
      } else {
//        if (m_context->exploreMode())
        if (m_context->exploreMode() == ZInteractiveContext::EXPLORE_ZOOM_IN_IMAGE ||
            m_context->exploreMode() == ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE) {
          op.setOperation(ZStackOperator::OP_EXIT_ZOOM_MODE);
        } else {
          op.setOperation(ZStackOperator::OP_EXIT_EDIT_MODE);
        }
      }
    }
  }

  return op;
}

///////////////ZMouseEventRightButtonPressMapper/////////////////////
////////             O-             ////////////////////////////
ZStackOperator
ZMouseEventRightButtonPressMapper::getOperation(const ZMouseEvent &/*event*/) const
{
  ZStackOperator op = initOperation();

  return op;
}

//////////////ZMouseEventMapper///////////////////
////////             =OO             ////////////
#define MOUSE_MOVE_IMAGE_THRESHOLD 25
ZStackOperator ZMouseEventMoveMapper::getOperation(
    const ZMouseEvent &event) const
{
  ZStackOperator op = initOperation();
  op.setPressedButtons(event.getButtons());

  if (m_context != NULL) {
    bool canMoveImage = false;

    if (event.getButtons() == Qt::LeftButton) {
      if (event.getModifiers() == Qt::ShiftModifier) {
        if (m_context->getUniqueMode() ==
            ZInteractiveContext::INTERACT_OBJECT_MOVE ||
            m_context->getUniqueMode() ==
            ZInteractiveContext::INTERACT_SWC_MOVE_NODE) {
          op.setOperation(ZStackOperator::OP_MOVE_OBJECT);
        }
        canMoveImage = true;
      } else {
//        if (m_context->getUniqueMode() ==
//            ZInteractiveContext::INTERACT_SWC_EXTEND) {
        if (1) {
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
    } else if (event.getButtons() == (Qt::LeftButton | Qt::RightButton) ||
               event.getButtons() == Qt::MidButton) {
      canMoveImage = true;
    }

    if (event.getButtons() == Qt::LeftButton) {
      if (op.isNull()) {
        if (m_context->getUniqueMode() ==
            ZInteractiveContext::INTERACT_STROKE_DRAW) {
          op.setOperation(ZStackOperator::OP_PAINT_STROKE);
        } else if (m_context->getUniqueMode() ==
                   ZInteractiveContext::INTERACT_RECT_DRAW) {
          op.setOperation(ZStackOperator::OP_RECT_ROI_UPDATE);
        }
      }
    } else if (event.getButtons() == Qt::RightButton) {
      if (m_context->getUniqueMode() != ZInteractiveContext::INTERACT_IMAGE_MOVE) {
        ZIntPoint pressPos =
            getPosition(Qt::RightButton, ZMouseEvent::ACTION_PRESS);
        int dx = event.getX() - pressPos.getX();
        int dy = event.getY() - pressPos.getY();
        if (dx * dx + dy * dy > MOUSE_MOVE_IMAGE_THRESHOLD) {
          if (dy < -5) {
            op.setOperation(ZStackOperator::OP_ZOOM_IN_GRAB_POS);
          } else if (dy > 5) {
            op.setOperation(ZStackOperator::OP_ZOOM_OUT_GRAB_POS);
          }
        }
      }
    }

    if (op.isNull()) {
      if (canMoveImage) {
        if (m_context->exploreMode() ==
            ZInteractiveContext::EXPLORE_MOVE_IMAGE) {
          op.setOperation(ZStackOperator::OP_MOVE_IMAGE);
        } else {
          op.setOperation(ZStackOperator::OP_START_MOVE_IMAGE);
        }
      }
    }

    if (op.isNull()) {
      op.setOperation(ZStackOperator::OP_TRACK_MOUSE_MOVE);

#ifdef _FLYEM_
      if (event.getModifiers() == Qt::ShiftModifier &&
          m_context->getUniqueMode() ==
          ZInteractiveContext::INTERACT_STROKE_DRAW) {
        op.setTogglingStrokeLabel(true);
      }
#endif
    }
  }

  return op;
}
