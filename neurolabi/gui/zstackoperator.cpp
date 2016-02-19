#include "zstackoperator.h"
#include "zmouseeventrecorder.h"
#include "zstroke2d.h"
#include "zswctree.h"
#include "zstackdoc.h"

ZStackOperator::ZStackOperator() :
  m_op(OP_NULL),
  m_hitObject(NULL),
  m_punctaIndex(-1), m_togglingStrokeLabel(false),
  m_buttonPressed(Qt::NoButton),
  m_mouseEventRecorder(NULL)
{
}

bool ZStackOperator::isNull() const
{
  return getOperation() == OP_NULL;
}

ZPoint ZStackOperator::getMouseOffset(NeuTube::ECoordinateSystem cs) const
{
  ZPoint offset(0, 0, 0);

  if (m_mouseEventRecorder != NULL) {
    offset = m_mouseEventRecorder->getPositionOffset(cs);
  }

  return offset;
}

void ZStackOperator::setPressedButtons(const Qt::MouseButtons &buttons)
{
  m_buttonPressed = buttons;
}

bool ZStackOperator::IsOperable(EOperation op, const ZStackDoc *doc)
{
  if (doc == NULL) {
    return false;
  }

  bool opable = true;
  switch (op) {
  case ZStackOperator::OP_NULL:
    opable = false;
    break;
  case ZStackOperator::OP_SWC_DELETE_NODE:
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT:
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT:
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_UP:
  case ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN:
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST:
  case ZStackOperator::OP_SWC_CONNECT_NODE:
  case ZStackOperator::OP_SWC_CONNECT_NODE_SMART:
  case ZStackOperator::OP_SWC_CONNECT_ISOLATE:
  case ZStackOperator::OP_SWC_ZOOM_TO_SELECTED_NODE:
  case ZStackOperator::OP_SWC_MOVE_NODE:
  case ZStackOperator::OP_SWC_CHANGE_NODE_FOCUS:
  case ZStackOperator::OP_SWC_SELECT_CONNECTION:
  case ZStackOperator::OP_SWC_SELECT_FLOOD:
    if (!doc->hasSelectedSwcNode()) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_EXTEND:
  case ZStackOperator::OP_SWC_SMART_EXTEND:
  case ZStackOperator::OP_SWC_RESET_BRANCH_POINT:
  case ZStackOperator::OP_SWC_CONNECT_TO:
  case ZStackOperator::OP_SWC_LOCATE_FOCUS:
  case ZStackOperator::OP_SWC_ENTER_EXTEND_NODE:
    if (doc->getSelectedSwcNodeList().size() != 1) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_BREAK_NODE:
  case ZStackOperator::OP_SWC_INSERT_NODE:
    if (doc->getSelectedSwcNodeList().size() <= 1) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_ENTER_ADD_NODE:
    if (doc->getTag() != NeuTube::Document::NORMAL &&
        doc->getTag() != NeuTube::Document::BIOCYTIN_STACK &&
        doc->getTag() != NeuTube::Document::FLYEM_ROI) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
    if (!doc->hasSelectedSwcNode()) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_OBJECT_SELECT_IN_ROI:
    if (!doc->getRect2dRoi().isValid()) {
      opable = false;
    }
  default:
    break;
  }

  return opable;
}

template<>
Swc_Tree_Node* ZStackOperator::getHitObject<Swc_Tree_Node>() const
{
  Swc_Tree_Node *tn = NULL;
  ZSwcTree *tree = getHitObject<ZSwcTree>();
  if (tree != NULL) {
    tn = tree->getHitNode();
  }

  return tn;
}
