#include "zkeyeventmapperfactory.h"
#include "zkeyeventmapper.h"
#include "zstackoperator.h"

ZKeyEventMapperFactory::ZKeyEventMapperFactory()
{
}

ZKeyEventMapper* ZKeyEventMapperFactory::MakeSwcNodeMapper()
{

  ZKeyEventMapper *mapper = new ZKeyEventMapper;
  mapper->setOperation(Qt::Key_Backspace, ZStackOperator::OP_SWC_DELETE_NODE);
  mapper->setOperation(Qt::Key_Delete, ZStackOperator::OP_SWC_DELETE_NODE);
  mapper->setOperation(Qt::Key_X, ZStackOperator::OP_SWC_DELETE_NODE);
  mapper->setOperation(Qt::Key_W, ZStackOperator::OP_SWC_MOVE_NODE_UP);
  mapper->setOperation(Qt::Key_S, ZStackOperator::OP_SWC_MOVE_NODE_DOWN);
  mapper->setOperation(Qt::Key_A, ZStackOperator::OP_SWC_MOVE_NODE_LEFT);
  mapper->setOperation(Qt::Key_D, ZStackOperator::OP_SWC_MOVE_NODE_RIGHT);

  mapper->setOperation(Qt::Key_G, ZStackOperator::OP_SWC_ADD_NODE);
/*
  m_plainKeyMap[Qt::Key_Comma] = ZSwcTree::OPERATION_DECREASE_NODE_SIZE;
  m_plainKeyMap[Qt::Key_Q] = ZSwcTree::OPERATION_DECREASE_NODE_SIZE;
  m_plainKeyMap[Qt::Key_Period] = ZSwcTree::OPERATION_INCREASE_NODE_SIZE;
  m_plainKeyMap[Qt::Key_E] = ZSwcTree::OPERATION_INCREASE_NODE_SIZE;
  m_plainKeyMap[Qt::Key_C] = ZSwcTree::OPERATION_CONNECT_NODE;
  m_plainKeyMap[Qt::Key_B] = ZSwcTree::OPERATION_BREAK_NODE;
  m_plainKeyMap[Qt::Key_N] = ZSwcTree::OPERATION_CONNECT_ISOLATE;
  m_plainKeyMap[Qt::Key_Z] = ZSwcTree::OPERATION_ZOOM_TO_SELECTED_NODE;
  m_plainKeyMap[Qt::Key_I] = ZSwcTree::OPERATION_INSERT_NODE;
  m_plainKeyMap[Qt::Key_F] = ZSwcTree::OPERATION_CHANGE_NODE_FACUS;
  m_plainKeyMap[Qt::Key_V] = ZSwcTree::OPERATION_MOVE_NODE;
  m_plainKeyMap[Qt::Key_R] = ZSwcTree::OPERATION_RESET_BRANCH_POINT;
  m_plainKeyMap[Qt::Key_Space] = ZSwcTree::OPERATION_EXTEND_NODE;
  */

  return mapper;
}
