#ifndef ZSTACKOPERATOR_H
#define ZSTACKOPERATOR_H

#include <Qt>
#include "swctreenode.h"
#include "zmouseevent.h"

class ZMouseEventRecorder;
class ZStroke2d;
class ZObject3d;
class ZStackObject;
class ZStackDoc;

class ZStackOperator
{
public:
  ZStackOperator();

  enum EOperation {
    OP_NULL,

    OP_MOVE_IMAGE, OP_MOVE_OBJECT, OP_CAPTURE_MOUSE_POSITION,
    OP_DESELECT_ALL,
    OP_PROCESS_OBJECT, OP_RESTORE_EXPLORE_MODE, OP_TRACK_MOUSE_MOVE,
    OP_TRACK_MOUSE_MOVE_WITH_STROKE_TOGGLE,
    OP_ZOOM_IN, OP_ZOOM_OUT, OP_ZOOM_IN_GRAB_POS, OP_ZOOM_OUT_GRAB_POS,
    OP_PAINT_STROKE, OP_START_MOVE_IMAGE, /*OP_SHOW_STACK_CONTEXT_MENU,*/
    OP_SHOW_SWC_CONTEXT_MENU, OP_SHOW_STROKE_CONTEXT_MENU,
    OP_SHOW_PUNCTA_CONTEXT_MENU,  OP_SHOW_TRACE_CONTEXT_MENU,
    OP_SHOW_PUNCTA_MENU, OP_SHOW_BODY_CONTEXT_MENU,
    OP_SHOW_CONTEXT_MENU,
    OP_EXIT_EDIT_MODE, OP_CUSTOM_MOUSE_RELEASE,

    OP_IMAGE_MOVE_UP, OP_IMAGE_MOVE_DOWN, OP_IMAGE_MOVE_LEFT,
    OP_IMAGE_MOVE_RIGHT, OP_IMAGE_MOVE_UP_FAST, OP_IMAGE_MOVE_DOWN_FAST,
    OP_IMAGE_MOVE_LEFT_FAST, OP_IMAGE_MOVE_RIGHT_FAST,
    OP_STACK_TOGGLE_PROJECTION, OP_STACK_INC_SLICE,
    OP_STACK_DEC_SLICE, OP_STACK_INC_SLICE_FAST,
    OP_STACK_DEC_SLICE_FAST, OP_EXIT_ZOOM_MODE,

    OP_SWC_SELECT, OP_SWC_SELECT_SINGLE_NODE, OP_SWC_SELECT_MULTIPLE_NODE,
    OP_SWC_DESELECT_SINGLE_NODE, OP_SWC_DESELECT_ALL_NODE,
    OP_SWC_EXTEND, OP_SWC_SMART_EXTEND, OP_SWC_CONNECT,
    OP_SWC_ADD_NODE, OP_SWC_DELETE_NODE, OP_SWC_SELECT_ALL_NODE,
    OP_SWC_MOVE_NODE_LEFT, OP_SWC_MOVE_NODE_LEFT_FAST,
    OP_SWC_MOVE_NODE_RIGHT, OP_SWC_MOVE_NODE_RIGHT_FAST,
    OP_SWC_MOVE_NODE_UP, OP_SWC_MOVE_NODE_UP_FAST,
    OP_SWC_MOVE_NODE_DOWN, OP_SWC_MOVE_NODE_DOWN_FAST,
    OP_SWC_INCREASE_NODE_SIZE, OP_SWC_DECREASE_NODE_SIZE,
    OP_SWC_CONNECT_NODE, OP_SWC_CONNECT_NODE_SMART,
    OP_SWC_BREAK_NODE, OP_SWC_CONNECT_ISOLATE,
    OP_SWC_ZOOM_TO_SELECTED_NODE, OP_SWC_INSERT_NODE, OP_SWC_MOVE_NODE,
    OP_SWC_RESET_BRANCH_POINT, OP_SWC_CHANGE_NODE_FOCUS,
    OP_SWC_SELECT_CONNECTION,
    OP_SWC_SELECT_FLOOD, OP_SWC_CONNECT_TO,
    OP_SWC_LOCATE_FOCUS, OP_SWC_ENTER_ADD_NODE, OP_SWC_ENTER_EXTEND_NODE,
    OP_SWC_SET_AS_ROOT,

    OP_PUNCTA_SELECT_SINGLE, OP_PUNCTA_SELECT_MULTIPLE,
    OP_STROKE_ADD_NEW, OP_STROKE_START_PAINT,
    OP_STROKE_SELECT_SINGLE, OP_STROKE_SELECT_MULTIPLE,
    OP_STROKE_LOCATE_FOCUS,
    OP_OBJECT3D_SELECT_SINGLE, OP_OBJECT3D_SELECT_MULTIPLE,
    OP_OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE,
    OP_OBJECT3D_SCAN_TOGGLE_SELECT,
    OP_OBJECT3D_LOCATE_FOCUS,
    OP_OBJECT3D_SCAN_LOCATE_FOCUS,
    OP_STACK_LOCATE_SLICE, OP_STACK_VIEW_PROJECTION,
    OP_STACK_VIEW_SLICE,

    OP_RECT_ROI_INIT,
    OP_RECT_ROI_UPDATE, OP_RECT_ROI_TO_CUBOID, OP_RECT_ROI_ACCEPT,
    OP_RECT_ROI_APPEND,

    OP_OBJECT3D_SCAN_SELECT_SINGLE, OP_OBJECT3D_SCAN_SELECT_MULTIPLE,
    OP_DVID_SPARSE_STACK_LOCATE_FOCUS,

    OP_OBJECT_SELECT_SINGLE, OP_OBJECT_SELECT_MULTIPLE,
    OP_OBJECT_TOGGLE_VISIBILITY,
    OP_OBJECT_TOGGLE_TMP_RESULT_VISIBILITY,
    OP_OBJECT_SELECT_IN_ROI, OP_OBJECT_DELETE_SELECTED,

    OP_ACTIVE_STROKE_INCREASE_SIZE, OP_ACTIVE_STROKE_DECREASE_SIZE,
    OP_ACTIVE_STROKE_ESTIMATE_SIZE,

    OP_BOOKMARK_ENTER_ADD_MODE, OP_BOOKMARK_DELETE, OP_BOOKMARK_ADD_NEW,
    OP_BOOKMARK_ANNOTATE, OP_BOOKMARK_SELECT_SIGNLE,

    OP_DVID_SYNAPSE_SELECT_SINGLE, OP_DVID_SYNAPSE_SELECT_MULTIPLE,
    OP_DVID_SYNAPSE_SELECT_TOGGLE,
    OP_DVID_SYNAPSE_ADD, OP_DVID_SYNAPSE_MOVE
  };

  inline EOperation getOperation() const {
    return m_op;
  }

  inline void setOperation(EOperation op) {
    m_op = op;
  }

  inline void setMouseEventRecorder(const ZMouseEventRecorder *recorder) {
    m_mouseEventRecorder = recorder;
  }

  /*
  inline Swc_Tree_Node* getHitSwcNode() const {
    return m_hitNode;
  }
*/
  inline ZStackObject *getHitObject() const
  {
    return m_hitObject;
  }

  template <typename T>
  T* getHitObject() const;

  inline void setHitObject(ZStackObject *obj) {
    m_hitObject = obj;
  }

  inline int getPunctaIndex() const {
    return m_punctaIndex;
  }

  bool isNull() const;

  ZPoint getMouseOffset(NeuTube::ECoordinateSystem cs) const;
  inline const ZMouseEventRecorder* getMouseEventRecorder() const {
    return m_mouseEventRecorder;
  }

  inline void setTogglingStrokeLabel(bool toggling) {
    m_togglingStrokeLabel = toggling;
  }

  inline bool togglingStrokeLabel() const {
    return m_togglingStrokeLabel;
  }

  inline Qt::MouseButtons getPressedButtons() const {
    return m_buttonPressed;
  }

  void setPressedButtons(const Qt::MouseButtons &buttons);

  static bool IsOperable(EOperation op, const ZStackDoc *doc);

private:
  EOperation m_op;
  //Swc_Tree_Node *m_hitNode;
  ZStackObject *m_hitObject;
//  ZStroke2d *m_hitStroke;
//  ZObject3d *m_hitObj3d;
  int m_punctaIndex;
  bool m_togglingStrokeLabel;
  Qt::MouseButtons m_buttonPressed;
  const ZMouseEventRecorder *m_mouseEventRecorder;
};

template<>
Swc_Tree_Node* ZStackOperator::getHitObject<Swc_Tree_Node>() const;

template <typename T>
T* ZStackOperator::getHitObject() const
{
  return dynamic_cast<T*>(m_hitObject);
}


#endif // ZSTACKOPERATOR_H
