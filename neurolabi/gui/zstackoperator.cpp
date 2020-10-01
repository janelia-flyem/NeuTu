#include "zstackoperator.h"
#include "zmouseeventrecorder.h"
#include "zstroke2d.h"
#include "zswctree.h"
#include "mvc/zstackdoc.h"

ZStackOperator::ZStackOperator()
{
}

ZStackOperator::ZStackOperator(EOperation op) : m_op(op)
{

}

void ZStackOperator::clear()
{
  m_op = OP_NULL;
  m_hitObject = NULL;
//  m_punctaIndex = -1;
  m_viewId = -1;
  m_togglingStrokeLabel = false;
  m_buttonPressed = Qt::NoButton;
  m_label = 0;
  m_shift = false;
}

bool ZStackOperator::isNull() const
{
  return getOperation() == OP_NULL;
}

void ZStackOperator::setViewId(int viewId)
{
  m_viewId = viewId;
}

int ZStackOperator::getViewId() const
{
  return m_viewId;
}

ZPoint ZStackOperator::getMouseOffset(neutu::ECoordinateSystem cs) const
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
  case ZStackOperator::OP_OBJECT_DELETE_SELECTED:
    if (!doc->hasSelectedObject()) {
      opable = false;
    }
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
    if (doc->getTag() != neutu::Document::ETag::NORMAL &&
        doc->getTag() != neutu::Document::ETag::BIOCYTIN_STACK &&
        doc->getTag() != neutu::Document::ETag::FLYEM_ROI &&
        doc->getTag() != neutu::Document::ETag::FLYEM_PROOFREAD) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE_FAST:
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE_FAST:
    if (!doc->hasSelectedSwcNode()) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_OBJECT_SELECT_IN_ROI:
    if (doc->getRectRoi().isEmpty()) {
      opable = false;
    }
    break;
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

namespace {

template<typename T>
void append(std::ostream &stream, const std::string &name, const T &value)
{
  stream << name << ": " << value << "; ";
}

}

std::ostream& operator<< (std::ostream &stream, const ZStackOperator &op)
{
  stream << "{ type: ";
  switch (op.m_op) {
  case ZStackOperator::OP_NULL:
    stream << "NULL";
    break;
  case ZStackOperator::OP_MOVE_IMAGE:
    stream << "MOVE_IMAGE";
    break;
  case ZStackOperator::OP_MOVE_OBJECT:
    stream << "MOVE_OBJECT";
    break;
  case ZStackOperator::OP_CAPTURE_MOUSE_POSITION:
    stream << "CAPTURE_MOUSE_POSITION";
    break;
  case ZStackOperator::OP_DESELECT_ALL:
    stream << "DESELECT_ALL";
    break;
  case ZStackOperator::OP_PROCESS_OBJECT:
    stream << "PROCESS_OBJECT";
    break;
  case ZStackOperator::OP_RESTORE_EXPLORE_MODE:
    stream << "RESTORE_EXPLORE_MODE";
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE:
    stream << "TRACK_MOUSE_MOVE";
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE_WITH_STROKE_TOGGLE:
    stream << "TRACK_MOUSE_MOVE_WITH_STROKE_TOGGLE";
    break;
  case ZStackOperator::OP_ZOOM_IN:
    stream << "ZOOM_IN";
    break;
  case ZStackOperator::OP_ZOOM_OUT:
    stream << "ZOOM_OUT";
    break;
  case ZStackOperator::OP_ZOOM_IN_GRAB_POS:
    stream << "ZOOM_IN_GRAB_POS";
    break;
  case ZStackOperator::OP_ZOOM_OUT_GRAB_POS:
    stream << "ZOOM_OUT_GRAB_POS";
    break;
  case ZStackOperator::OP_ZOOM_TO:
    stream << "ZOOM_TO";
    break;
  case ZStackOperator::OP_PAINT_STROKE:
    stream << "PAINT_STROKE";
    break;
  case ZStackOperator::OP_START_PAINT_STROKE:
    stream << "START_PAINT_STROKE";
    break;
  case ZStackOperator::OP_START_MOVE_IMAGE:
    stream << "START_MOVE_IMAGE";
    break;
  case ZStackOperator::OP_SHOW_SWC_CONTEXT_MENU:
    stream << "SHOW_SWC_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_SHOW_STROKE_CONTEXT_MENU:
    stream << "SHOW_STROKE_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_SHOW_PUNCTA_CONTEXT_MENU:
    stream << "SHOW_PUNCTA_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_SHOW_TRACE_CONTEXT_MENU:
    stream << "SHOW_TRACE_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_SHOW_PUNCTA_MENU:
    stream << "SHOW_PUNCTA_MENU";
    break;
  case ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU:
    stream << "SHOW_BODY_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_SHOW_CONTEXT_MENU:
    stream << "SHOW_CONTEXT_MENU";
    break;
  case ZStackOperator::OP_EXIT_EDIT_MODE:
    stream << "EXIT_EDIT_MODE";
    break;
  case ZStackOperator::OP_CUSTOM_MOUSE_RELEASE:
    stream << "CUSTOM_MOUSE_RELEASE";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP:
    stream << "IMAGE_MOVE_UP";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_DOWN:
    stream << "IMAGE_MOVE_DOWN";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT:
    stream << "IMAGE_MOVE_LEFT";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT:
    stream << "IMAGE_MOVE_RIGHT";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP_FAST:
    stream << "IMAGE_MOVE_UP_FAST";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_DOWN_FAST:
    stream << "IMAGE_MOVE_DOWN_FAST";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT_FAST:
    stream << "IMAGE_MOVE_LEFT_FAST";
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT_FAST:
    stream << "IMAGE_MOVE_RIGHT_FAST";
    break;
  case ZStackOperator::OP_STACK_TOGGLE_PROJECTION:
    stream << "STACK_TOGGLE_PROJECTION";
    break;
  case ZStackOperator::OP_STACK_INC_SLICE:
    stream << "STACK_INC_SLICE";
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE:
    stream << "STACK_DEC_SLICE";
    break;
  case ZStackOperator::OP_STACK_INC_SLICE_FAST:
    stream << "STACK_INC_SLICE_FAST";
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE_FAST:
    stream << "STACK_DEC_SLICE_FAST";
    break;
  case ZStackOperator::OP_EXIT_ZOOM_MODE:
    stream << "EXIT_ZOOM_MODE";
    break;
  case ZStackOperator::OP_START_ROTATE_VIEW:
    stream << "START_ROTATE_VIEW";
    break;
  case ZStackOperator::OP_ROTATE_VIEW:
    stream << "ROTATE_VIEW";
    break;
  case ZStackOperator::OP_SWC_SELECT:
    stream << "SWC_SELECT";
    break;
  case ZStackOperator::OP_SWC_SELECT_SINGLE_NODE:
    stream << "SWC_SELECT_SINGLE_NODE";
    break;
  case ZStackOperator::OP_SWC_SELECT_MULTIPLE_NODE:
    stream << "SWC_SELECT_MULTIPLE_NODE";
    break;
  case ZStackOperator::OP_SWC_DESELECT_SINGLE_NODE:
    stream << "SWC_DESELECT_SINGLE_NODE";
    break;
  case ZStackOperator::OP_SWC_DESELECT_ALL_NODE:
    stream << "SWC_DESELECT_ALL_NODE";
    break;
  case ZStackOperator::OP_SWC_EXTEND:
    stream << "SWC_EXTEND";
    break;
  case ZStackOperator::OP_SWC_SMART_EXTEND:
    stream << "SWC_SMART_EXTEND";
    break;
  case ZStackOperator::OP_SWC_CONNECT:
    stream << "SWC_CONNECT";
    break;
  case ZStackOperator::OP_SWC_ADD_NODE:
    stream << "SWC_ADD_NODE";
    break;
  case ZStackOperator::OP_SWC_DELETE_NODE:
    stream << "SWC_DELETE_NODE";
    break;
  case ZStackOperator::OP_SWC_SELECT_ALL_NODE:
    stream << "SWC_SELECT_ALL_NODE";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT:
    stream << "SWC_MOVE_NODE_LEFT";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST:
    stream << "SWC_MOVE_NODE_LEFT_FAST";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT:
    stream << "SWC_MOVE_NODE_RIGHT";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST:
    stream << "SWC_MOVE_NODE_RIGHT_FAST";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_UP:
    stream << "SWC_MOVE_NODE_UP";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST:
    stream << "SWC_MOVE_NODE_UP_FAST";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN:
    stream << "SWC_MOVE_NODE_DOWN";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST:
    stream << "SWC_MOVE_NODE_DOWN_FAST";
    break;
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
    stream << "SWC_INCREASE_NODE_SIZE";
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
    stream << "SWC_DECREASE_NODE_SIZE";
    break;
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE_FAST:
    stream << "SWC_INCREASE_NODE_SIZE_FAST";
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE_FAST:
    stream << "SWC_DECREASE_NODE_SIZE_FAST";
    break;
  case ZStackOperator::OP_SWC_CONNECT_NODE:
    stream << "SWC_CONNECT_NODE";
    break;
  case ZStackOperator::OP_SWC_CONNECT_NODE_SMART:
    stream << "SWC_CONNECT_NODE_SMART";
    break;
  case ZStackOperator::OP_SWC_BREAK_NODE:
    stream << "SWC_BREAK_NODE";
    break;
  case ZStackOperator::OP_SWC_CONNECT_ISOLATE:
    stream << "SWC_CONNECT_ISOLATE";
    break;
  case ZStackOperator::OP_SWC_ZOOM_TO_SELECTED_NODE:
    stream << "SWC_ZOOM_TO_SELECTED_NODE";
    break;
  case ZStackOperator::OP_SWC_INSERT_NODE:
    stream << "SWC_INSERT_NODE";
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE:
    stream << "SWC_MOVE_NODE";
    break;
  case ZStackOperator::OP_SWC_RESET_BRANCH_POINT:
    stream << "SWC_RESET_BRANCH_POINT";
    break;
  case ZStackOperator::OP_SWC_CHANGE_NODE_FOCUS:
    stream << "SWC_CHANGE_NODE_FOCUS";
    break;
  case ZStackOperator::OP_SWC_SELECT_CONNECTION:
    stream << "SWC_SELECT_CONNECTION";
    break;
  case ZStackOperator::OP_SWC_SELECT_NODE_IN_ROI:
    stream << "SWC_SELECT_NODE_IN_ROI";
    break;
  case ZStackOperator::OP_SWC_SELECT_FLOOD:
    stream << "SWC_SELECT_FLOOD";
    break;
  case ZStackOperator::OP_SWC_SELECT_TREE_IN_ROI:
    stream << "SWC_SELECT_TREE_IN_ROI";
    break;
  case ZStackOperator::OP_SWC_SELECT_TERMINAL_BRANCH_IN_ROI:
    stream << "SWC_SELECT_TERMINAL_BRANCH_IN_ROI";
    break;
  case ZStackOperator::OP_SWC_CONNECT_TO:
    stream << "SWC_CONNECT_TO";
    break;
  case ZStackOperator::OP_SWC_LOCATE_FOCUS:
    stream << "SWC_LOCATE_FOCUS";
    break;
  case ZStackOperator::OP_SWC_ENTER_ADD_NODE:
    stream << "SWC_ENTER_ADD_NODE";
    break;
  case ZStackOperator::OP_SWC_ENTER_EXTEND_NODE:
    stream << "SWC_ENTER_EXTEND_NODE";
    break;
  case ZStackOperator::OP_SWC_SET_AS_ROOT:
    stream << "SWC_SET_AS_ROOT";
    break;
  case ZStackOperator::OP_PUNCTA_SELECT_SINGLE:
    stream << "PUNCTA_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_PUNCTA_SELECT_MULTIPLE:
    stream << "PUNCTA_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_STROKE_ADD_NEW:
    stream << "STROKE_ADD_NEW";
    break;
  case ZStackOperator::OP_STROKE_START_PAINT:
    stream << "STROKE_START_PAINT";
    break;
  case ZStackOperator::OP_STROKE_SELECT_SINGLE:
    stream << "STROKE_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_STROKE_SELECT_MULTIPLE:
    stream << "STROKE_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_STROKE_LOCATE_FOCUS:
    stream << "STROKE_LOCATE_FOCUS";
    break;
  case ZStackOperator::OP_OBJECT3D_SELECT_SINGLE:
    stream << "OBJECT3D_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_OBJECT3D_SELECT_MULTIPLE:
    stream << "OBJECT3D_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE:
    stream << "OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT:
    stream << "OBJECT3D_SCAN_TOGGLE_SELECT";
    break;
  case ZStackOperator::OP_OBJECT3D_LOCATE_FOCUS:
    stream << "OBJECT3D_LOCATE_FOCUS";
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_LOCATE_FOCUS:
    stream << "OBJECT3D_SCAN_LOCATE_FOCUS";
    break;
  case ZStackOperator::OP_STACK_LOCATE_SLICE:
    stream << "STACK_LOCATE_SLICE";
    break;
  case ZStackOperator::OP_STACK_VIEW_PROJECTION:
    stream << "STACK_VIEW_PROJECTION";
    break;
  case ZStackOperator::OP_STACK_VIEW_SLICE:
    stream << "STACK_VIEW_SLICE";
    break;
  case ZStackOperator::OP_RECT_ROI_INIT:
    stream << "RECT_ROI_INIT";
    break;
  case ZStackOperator::OP_RECT_ROI_UPDATE:
    stream << "RECT_ROI_UPDATE";
    break;
  case ZStackOperator::OP_RECT_ROI_TO_CUBOID:
    stream << "RECT_ROI_TO_CUBOID";
    break;
  case ZStackOperator::OP_RECT_ROI_ACCEPT:
    stream << "RECT_ROI_ACCEPT";
    break;
  case ZStackOperator::OP_RECT_ROI_APPEND:
    stream << "RECT_ROI_APPEND";
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_SELECT_SINGLE:
    stream << "OBJECT3D_SCAN_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_SELECT_MULTIPLE:
    stream << "OBJECT3D_SCAN_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_DVID_SPARSE_STACK_LOCATE_FOCUS:
    stream << "DVID_SPARSE_STACK_LOCATE_FOCUS";
    break;
  case ZStackOperator::OP_OBJECT_SELECT_SINGLE:
    stream << "OBJECT_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_OBJECT_SELECT_MULTIPLE:
    stream << "OBJECT_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_OBJECT_SELECT_TOGGLE:
    stream << "OBJECT_SELECT_TOGGLE";
    break;
  case ZStackOperator::OP_OBJECT_TOGGLE_VISIBILITY:
    stream << "OBJECT_TOGGLE_VISIBILITY";
    break;
  case ZStackOperator::OP_OBJECT_TOGGLE_TMP_RESULT_VISIBILITY:
    stream << "OBJECT_TOGGLE_TMP_RESULT_VISIBILITY";
    break;
  case ZStackOperator::OP_OBJECT_SELECT_IN_ROI:
    stream << "OBJECT_SELECT_IN_ROI";
    break;
  case ZStackOperator::OP_OBJECT_DELETE_SELECTED:
    stream << "OBJECT_DELETE_SELECTED";
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_INCREASE_SIZE:
    stream << "ACTIVE_STROKE_INCREASE_SIZE";
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_DECREASE_SIZE:
    stream << "ACTIVE_STROKE_DECREASE_SIZE";
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_ESTIMATE_SIZE:
    stream << "ACTIVE_STROKE_ESTIMATE_SIZE";
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_CHANGE_LABEL:
    stream << "ACTIVE_STROKE_CHANGE_LABEL";
    break;
  case ZStackOperator::OP_BOOKMARK_ENTER_ADD_MODE:
    stream << "BOOKMARK_ENTER_ADD_MODE";
    break;
  case ZStackOperator::OP_BOOKMARK_DELETE:
    stream << "BOOKMARK_DELETE";
    break;
  case ZStackOperator::OP_BOOKMARK_ADD_NEW:
    stream << "BOOKMARK_ADD_NEW";
    break;
  case ZStackOperator::OP_BOOKMARK_ANNOTATE:
    stream << "BOOKMARK_ANNOTATE";
    break;
  case ZStackOperator::OP_BOOKMARK_SELECT_SIGNLE:
    stream << "BOOKMARK_SELECT_SIGNLE";
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_SINGLE:
    stream << "DVID_LABEL_SLICE_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_MULTIPLE:
    stream << "DVID_LABEL_SLICE_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT:
    stream << "DVID_LABEL_SLICE_TOGGLE_SELECT";
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT_SINGLE:
    stream << "DVID_LABEL_SLICE_TOGGLE_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_SINGLE:
    stream << "DVID_SYNAPSE_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_MULTIPLE:
    stream << "DVID_SYNAPSE_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_TOGGLE:
    stream << "DVID_SYNAPSE_SELECT_TOGGLE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD:
    stream << "DVID_SYNAPSE_ADD";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD_ORPHAN:
    stream << "DVID_SYNAPSE_ADD_ORPHAN";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_MOVE:
    stream << "DVID_SYNAPSE_MOVE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ANNOTATE:
    stream << "DVID_SYNAPSE_ANNOTATE";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_START_TBAR:
    stream << "DVID_SYNAPSE_START_TBAR";
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_START_PSD:
    stream << "DVID_SYNAPSE_START_PSD";
    break;
  case ZStackOperator::OP_FLYEM_TOD_ENTER_ADD_MODE:
    stream << "FLYEM_TOD_ENTER_ADD_MODE";
    break;
  case ZStackOperator::OP_FLYEM_TODO_ADD:
    stream << "FLYEM_TODO_ADD";
    break;
  case ZStackOperator::OP_FLYEM_TODO_DELETE:
    stream << "FLYEM_TODO_DELETE";
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_SINGLE:
    stream << "FLYEM_TODO_SELECT_SINGLE";
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_MULTIPLE:
    stream << "FLYEM_TODO_SELECT_MULTIPLE";
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_TOGGLE:
    stream << "FLYEM_TODO_SELECT_TOGGLE";
    break;
  case ZStackOperator::OP_FLYEM_TODO_ANNOTATE:
    stream << "FLYEM_TODO_ANNOTATE";
    break;
  case ZStackOperator::OP_TOGGLE_SEGMENTATION:
    stream << "TOGGLE_SEGMENTATION";
    break;
  case ZStackOperator::OP_REFRESH_SEGMENTATION:
    stream << "REFRESH_SEGMENTATION";
    break;
  case ZStackOperator::OP_FLYEM_CROP_BODY:
    stream << "FLYEM_CROP_BODY";
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY_LOCAL:
    stream << "FLYEM_SPLIT_BODY_LOCAL";
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY:
    stream << "FLYEM_SPLIT_BODY";
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY_FULL:
    stream << "FLYEM_SPLIT_BODY_FULL";
    break;
  case ZStackOperator::OP_GRAYSCALE_TOGGLE:
    stream << "GRAYSCALE_TOGGLE";
    break;
  case ZStackOperator::OP_CROSSHAIR_GRAB:
    stream << "CROSSHAIR_GRAB";
    break;
  case ZStackOperator::OP_CROSSHAIR_MOVE:
    stream << "CROSSHAIR_MOVE";
    break;
  case ZStackOperator::OP_CROSSHAIR_RELEASE:
    stream << "CROSSHAIR_RELEASE";
    break;
  case ZStackOperator::OP_EXPLORE_LOCAL:
    stream << "EXPLORE_LOCAL";
    break;
  }

  stream << "; ";
  append(stream, "view id", op.m_viewId);
  append(stream, "pressed", op.m_buttonPressed);
  append(stream, "label", op.m_label);
  append(stream, "shift", op.m_shift);
  if (op.m_hitObject) {
    append(stream, "hit", op.m_hitObject->getTypeName());
  }
  stream << "}";

  return stream;
}

