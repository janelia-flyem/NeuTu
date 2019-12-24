#include "zinteractivecontext.h"
#include "geometry/zpoint.h"
#include "widgets/zimagewidget.h"

/* Implementation details
 *
 * ZInteractiveContext is a class of managing the interactiion context of a MVC
 * environment. The context affects how the MVC responds to key and mouse events.
 * The context is categorized by modes, and a free mode means that no specific
 * mode is on. The editing modes are grouped by different enum types,
 * each corresponding to a certain type of object. Those modes include:
 *
 * TubeEditMode m_tubeEditMode: tube (obsolete)
 * SwcEditMode m_swcEditMode: SWC skeletons
 * StrokeEditMode m_strokeEditMode: strokes
 * RectEditMode m_rectEditMode: rectangles
 * BookmarkEditMode m_bookmarkEditMode: bookmarks
 * SynapseEditMode m_synapseEditMode: synapses
 * TodoEditMode m_todoEditMode: todos
 *
 * Those modes are exclusive. So switching from one mode to another needs to turn
 * off the old mode.
 */

ZInteractiveContext::ZInteractiveContext()
{
  m_viewMode = VIEW_NORMAL;
  m_exploreMode = EXPLORE_OFF;
  m_oldExploreMode = EXPLORE_OFF;

  m_traceMode = TRACE_OFF;

  m_tubeEditMode = TUBE_EDIT_OFF;
  m_markPunctaMode = MARK_PUNCTA_OFF;
  m_swcEditMode = SWC_EDIT_OFF;
  m_strokeEditMode = STROKE_EDIT_OFF;
  m_rectEditMode = RECT_EDIT_OFF;
  m_bookmarkEditMode = BOOKMARK_EDIT_OFF;
  m_todoEditMode = TODO_EDIT_OFF;
  m_synapseEditMode = SYNAPSE_EDIT_OFF;

  m_exitingEdit = false;
  m_blockingContextMenu = false;
  m_sliceAxis = neutu::EAxis::Z;
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

bool ZInteractiveContext::isFreeMode() const
{
  return getUniqueMode() == EUniqueMode::INTERACT_FREE;
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

  switch (exploreMode()) {
  case EXPLORE_MOVE_IMAGE:
    mode = INTERACT_IMAGE_MOVE;
    break;
  case  EXPLORE_LOCAL:
    mode = INTERACT_EXPLORE_LOCAL;
    break;
  case EXPLORE_EXTERNALLY:
    mode = INTERACT_EXPLORE_EXTERNALLY;
    break;
  case EXPLORE_DETAIL:
    mode = INTERACT_EXPLORE_DETAIL;
    break;
  default:
    break;
  }

  if (mode == INTERACT_FREE) {
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
    switch (editPunctaMode()) {
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

  return mode;
}

bool ZInteractiveContext::isEditOff() const
{
  return (m_tubeEditMode == TUBE_EDIT_OFF) &&
      (m_markPunctaMode == MARK_PUNCTA_OFF) &&
      (m_strokeEditMode == STROKE_EDIT_OFF) &&
      (m_rectEditMode == RECT_EDIT_OFF) &&
      (m_bookmarkEditMode == BOOKMARK_EDIT_OFF) &&
      (m_todoEditMode == TODO_EDIT_OFF) &&
      (m_synapseEditMode == SYNAPSE_EDIT_OFF) &&
      (m_swcEditMode == SWC_EDIT_OFF);
}

namespace {
template <typename T>
void change_edit_mode(T& mode, const T& value, bool &toggled)
{
  if (mode != value) {
    mode = value;
    toggled = true;
  }
}
}

bool ZInteractiveContext::turnOffEditMode()
{
  bool toggled = false;

  change_edit_mode(m_tubeEditMode, TUBE_EDIT_OFF, toggled);
  change_edit_mode(m_markPunctaMode, MARK_PUNCTA_OFF, toggled);
  change_edit_mode(m_strokeEditMode, STROKE_EDIT_OFF, toggled);
  change_edit_mode(m_rectEditMode, RECT_EDIT_OFF, toggled);
  change_edit_mode(m_bookmarkEditMode, BOOKMARK_EDIT_OFF, toggled);
  change_edit_mode(m_todoEditMode, TODO_EDIT_OFF, toggled);
  change_edit_mode(m_synapseEditMode, SYNAPSE_EDIT_OFF, toggled);
  change_edit_mode(m_swcEditMode, SWC_EDIT_OFF, toggled);

  return toggled;
}

