#include "zflyemkeyoperationconfig.h"

#include <QMap>
#include "zstackoperator.h"
#include "zkeyoperationmap.h"

ZFlyEmKeyOperationConfig::ZFlyEmKeyOperationConfig()
{
}

void ZFlyEmKeyOperationConfig::configure(
    ZKeyOperationMap &map, ZKeyOperation::EGroup group)
{
  ZKeyOperationConfig::configure(map, group);

  switch (group) {
  case ZKeyOperation::OG_STACK:
    ConfigureFlyEmStackMap(map);
    break;
  case ZKeyOperation::OG_FLYEM_BOOKMARK:
    ConfigureFlyEmBookmarkMap(map);
    break;
  case ZKeyOperation::OG_SWC_TREE_NODE:
  {
    QMap<int, ZStackOperator::EOperation> &plainKeyMap = *(map.getPlainMap());
    plainKeyMap.remove(Qt::Key_F);
  }
    break;
  default:
    break;
  }
}

void ZFlyEmKeyOperationConfig::ConfigureFlyEmStackMap(ZKeyOperationMap &map)
{
  QMap<int, ZStackOperator::EOperation> &plainKeyMap = *(map.getPlainMap());

  plainKeyMap[Qt::Key_F] = ZStackOperator::OP_OBJECT_TOGGLE_VISIBILITY;
}

void ZFlyEmKeyOperationConfig::ConfigureFlyEmBookmarkMap(ZKeyOperationMap &map)
{
  QMap<int, ZStackOperator::EOperation> &plainKeyMap = *(map.getPlainMap());

  plainKeyMap[Qt::Key_Backspace] = ZStackOperator::OP_BOOKMARK_DELETE;
  plainKeyMap[Qt::Key_Delete] = ZStackOperator::OP_BOOKMARK_DELETE;
//  plainKeyMap[Qt::Key_Backspace] = ZStackOperator::OP_SWC_DELETE_NODE;

//  plainKeyMap[Qt::Key_W] = ZStackOperator::OP_SWC_MOVE_NODE_UP;
//  plainKeyMap[Qt::Key_A] = ZStackOperator::OP_SWC_MOVE_NODE_LEFT;
//  plainKeyMap[Qt::Key_S] = ZStackOperator::OP_SWC_MOVE_NODE_DOWN;
//  plainKeyMap[Qt::Key_D] = ZStackOperator::OP_SWC_MOVE_NODE_RIGHT;
  plainKeyMap[Qt::Key_G] = ZStackOperator::OP_BOOKMARK_ENTER_ADD_MODE;

//  plainKeyMap[Qt::Key_Q] = ZStackOperator::OP_SWC_DECREASE_NODE_SIZE;
//  plainKeyMap[Qt::Key_E] = ZStackOperator::OP_SWC_INCREASE_NODE_SIZE;
//  plainKeyMap[Qt::Key_C] = ZStackOperator::OP_SWC_CONNECT_NODE;
//  plainKeyMap[Qt::Key_B] = ZStackOperator::OP_SWC_BREAK_NODE;
//  plainKeyMap[Qt::Key_N] = ZStackOperator::OP_SWC_CONNECT_ISOLATE;
//  plainKeyMap[Qt::Key_Z] = ZStackOperator::OP_SWC_ZOOM_TO_SELECTED_NODE;
//  plainKeyMap[Qt::Key_I] = ZStackOperator::OP_SWC_INSERT_NODE;
//  plainKeyMap[Qt::Key_F] = ZStackOperator::OP_SWC_LOCATE_FOCUS;
//  plainKeyMap[Qt::Key_V] = ZStackOperator::OP_SWC_MOVE_NODE;
//  plainKeyMap[Qt::Key_R] = ZStackOperator::OP_SWC_RESET_BRANCH_POINT;
//  plainKeyMap[Qt::Key_Space] = ZStackOperator::OP_SWC_ENTER_EXTEND_NODE;

//  QMap<int, ZStackOperator::EOperation> &shiftKeyMap = *(map.getShiftMap());

//  shiftKeyMap[Qt::Key_W] = ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST;
//  shiftKeyMap[Qt::Key_A] = ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST;
//  shiftKeyMap[Qt::Key_S] = ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST;
//  shiftKeyMap[Qt::Key_D] = ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST;
//  shiftKeyMap[Qt::Key_C] = ZStackOperator::OP_SWC_CONNECT_NODE_SMART;

  QMap<int, ZStackOperator::EOperation> &controlKeyMap = *(map.getControlMap());
  controlKeyMap[Qt::Key_G] = ZStackOperator::OP_SWC_ENTER_ADD_NODE;
}
