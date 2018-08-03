#include "zinteractivecontext.h"
#include "zpoint.h"
#include "widgets/zimagewidget.h"

ZInteractiveContext::ZInteractiveContext()
{
  m_traceMode = TRACE_OFF;
  m_tubeEditMode = TUBE_EDIT_OFF;
  m_viewMode = VIEW_NORMAL;
  m_exploreMode = EXPLORE_OFF;
  m_oldExploreMode = EXPLORE_OFF;
  m_markPunctaMode = MARK_PUNCTA_OFF;
//  m_swcEditMode = SWC_EDIT_SELECT;
  m_swcEditMode = SWC_EDIT_OFF;
  m_strokeEditMode = STROKE_EDIT_OFF;
  m_rectEditMode = RECT_EDIT_OFF;
  m_bookmarkEditMode = BOOKMARK_EDIT_OFF;
  m_todoEditMode = TODO_EDIT_OFF;
  m_synapseEditMode = SYNAPSE_EDIT_OFF;
  m_exitingEdit = false;
  m_blockingContextMenu = false;
  m_sliceAxis = neutube::Z_AXIS;
  m_acceptingRect = false;
  m_rectSpan = false;
  m_keyIndex = 1;
  m_uniqueMode = INTERACT_FREE;
}


bool ZInteractiveContext::isTraceModeOff() const
{
  if (/*m_swcEditMode != SWC_EDIT_SELECT ||*/
      m_swcEditMode != SWC_EDIT_OFF) {
    return false;
  }

  return (m_traceMode == TRACE_OFF);
}

bool ZInteractiveContext::isContextMenuActivated() const
{
  return ((m_swcEditMode == SWC_EDIT_OFF /*|| m_swcEditMode == SWC_EDIT_SELECT*/) &&
          m_tubeEditMode == TUBE_EDIT_OFF &&
          m_strokeEditMode == STROKE_EDIT_OFF &&
          m_rectEditMode == RECT_EDIT_OFF &&
          m_bookmarkEditMode == BOOKMARK_EDIT_OFF &&
          m_synapseEditMode == SYNAPSE_EDIT_OFF &&
          m_todoEditMode == TODO_EDIT_OFF &&
          !m_exitingEdit &&
          !m_blockingContextMenu);
}

void ZInteractiveContext::blockContextMenu(bool blocking)
{
  m_blockingContextMenu = blocking;
}

void ZInteractiveContext::setUniqueMode(EUniqueMode mode)
{
  m_uniqueMode = mode;
}

ZInteractiveContext::EUniqueMode ZInteractiveContext::getUniqueMode() const
{
  EUniqueMode mode = m_uniqueMode;

  if (mode != INTERACT_FREE) {
    return mode;
  }

  if (exploreMode() == EXPLORE_MOVE_IMAGE) {
    return INTERACT_IMAGE_MOVE;
  }

//  if (isExploreModeOff()) {
    switch (swcEditMode()) {
    case SWC_EDIT_ADD_NODE:
      mode = INTERACT_SWC_ADD_NODE;
      break;
    case SWC_EDIT_CONNECT:
      mode = INTERACT_SWC_CONNECT;
      break;
    case SWC_EDIT_EXTEND:
    case SWC_EDIT_SMART_EXTEND:
      mode = INTERACT_SWC_EXTEND;
      break;
    case SWC_EDIT_MOVE_NODE:
      mode = INTERACT_SWC_MOVE_NODE;
      break;
    case SWC_EDIT_LOCK_FOCUS:
      mode = INTERACT_SWC_LOCK_FOCUS;
      break;
    default:
      break;
    }

    if (mode == INTERACT_FREE) {
      switch (strokeEditMode()) {
      case STROKE_DRAW:
        mode = INTERACT_STROKE_DRAW;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      switch (rectEditMode()) {
      case RECT_DRAW:
        mode = INTERACT_RECT_DRAW;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      switch (MarkPunctaMode()) {
      case MARK_PUNCTA:
        mode = INTERACT_PUNCTA_MARK;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      switch (bookmarkEditMode()) {
      case BOOKMARK_ADD:
        mode = INTERACT_ADD_BOOKMARK;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      switch (synapseEditMode()) {
      case SYNAPSE_ADD_PRE:
      case SYNAPSE_ADD_POST:
        mode = INTERACT_ADD_SYNAPSE;
        break;
      case SYNAPSE_MOVE:
        mode = INTERACT_MOVE_SYNAPSE;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      switch (todoEditMode()) {
      case TODO_ADD_ITEM:
        mode = INTERACT_ADD_TODO_ITEM;
        break;
      default:
        break;
      }
    }

    if (mode == INTERACT_FREE) {
      if (exploreMode() == EXPLORE_LOCAL) {
        mode = INTERACT_EXPLORE_LOCAL;
      } else if (exploreMode() == EXPLORE_EXTERNALLY) {
        mode = INTERACT_EXPLORE_EXTERNALLY;
      }
    }

//  } else {
//    if (mode == INTERACT_FREE) {
//    switch (exploreMode()) {
//    case EXPLORE_MOVE_IMAGE:
//      mode = INTERACT_IMAGE_MOVE;
//      break;
//    case EXPLORE_CAPTURE_MOUSE:
//      mode = INTERACT_IMAGE_CAPTURE;
//      break;
//    case EXPLORE_ZOOM_IN_IMAGE:
//      mode = INTERACT_IMAGE_ZOOM_IN;
//      break;
//    case EXPLORE_ZOOM_OUT_IMAGE:
//      mode = INTERACT_IMAGE_ZOOM_OUT;
//      break;
//    default:
//      break;
//    }
//  }

  return mode;
}
