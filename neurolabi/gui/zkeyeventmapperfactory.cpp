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
  mapper->setOperation(Qt::Key_L, ZStackOperator::OP_SWC_ADD_NODE);

  return mapper;
}
