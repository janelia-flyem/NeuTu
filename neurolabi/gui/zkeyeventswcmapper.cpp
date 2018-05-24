#include "zkeyeventswcmapper.h"
#include <QKeyEvent>

ZKeyEventSwcMapper::ZKeyEventSwcMapper(neutube::Document::ETag tag) :
  m_docTag(tag)
{
  initKeyMap();
}

void ZKeyEventSwcMapper::initKeyMap()
{
  m_plainKeyMap[Qt::Key_Backspace] = ZSwcTree::OPERATION_DELETE_NODE;
  m_plainKeyMap[Qt::Key_Delete] = ZSwcTree::OPERATION_DELETE_NODE;
  m_plainKeyMap[Qt::Key_X] = ZSwcTree::OPERATION_DELETE_NODE;

  m_plainKeyMap[Qt::Key_W] = ZSwcTree::OPERATION_MOVE_NODE_UP;
  m_plainKeyMap[Qt::Key_A] = ZSwcTree::OPERATION_MOVE_NODE_LEFT;
  m_plainKeyMap[Qt::Key_S] = ZSwcTree::OPERATION_MOVE_NODE_DOWN;
  m_plainKeyMap[Qt::Key_D] = ZSwcTree::OPERATION_MOVE_NODE_RIGHT;
  m_plainKeyMap[Qt::Key_G] = ZSwcTree::OPERATION_ADD_NODE;

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

  m_shiftKeyMap[Qt::Key_W] = ZSwcTree::OPERATION_MOVE_NODE_UP_FAST;
  m_shiftKeyMap[Qt::Key_A] = ZSwcTree::OPERATION_MOVE_NODE_LEFT_FAST;
  m_shiftKeyMap[Qt::Key_S] = ZSwcTree::OPERATION_MOVE_NODE_DOWN_FAST;
  m_shiftKeyMap[Qt::Key_D] = ZSwcTree::OPERATION_MOVE_NODE_RIGHT_FAST;
  m_shiftKeyMap[Qt::Key_C] = ZSwcTree::OPERATION_CONNECT_NODE_SMART;
  m_shiftKeyMap[Qt::Key_E] = ZSwcTree::OPERATION_INCREASE_NODE_SIZE_FAST;
  m_shiftKeyMap[Qt::Key_R] = ZSwcTree::OPERATION_DECREASE_NODE_SIZE_FAST;

  m_controlKeyMap[Qt::Key_A] = ZSwcTree::OPERATION_SELECT_ALL_NODE;
  m_controlKeyMap[Qt::Key_R] = ZSwcTree::OPERATION_SET_AS_ROOT;

  updateKeyMap();
}

ZSwcTree::EOperation ZKeyEventSwcMapper::getOperation(QKeyEvent *event)
{
  ZSwcTree::EOperation operation = ZSwcTree::OPERATION_NULL;

  if (event->modifiers() == Qt::NoModifier) {
    if (m_plainKeyMap.contains(event->key())) {
      operation = m_plainKeyMap[event->key()];
    }
  } else if (event->modifiers() == Qt::ControlModifier) {
    if (m_controlKeyMap.contains(event->key())) {
      operation = m_controlKeyMap[event->key()];
    }
  } else if (event->modifiers() == Qt::ShiftModifier) {
    if (m_shiftKeyMap.contains(event->key())) {
      operation = m_shiftKeyMap[event->key()];
    }
  } else if (event->modifiers() == Qt::AltModifier) {
    if (m_altKeyMap.contains(event->key())) {
      operation = m_altKeyMap[event->key()];
    }
  }

  return operation;
}

void ZKeyEventSwcMapper::setTag(neutube::Document::ETag tag)
{
  m_docTag = tag;
  updateKeyMap();
}

void ZKeyEventSwcMapper::updateKeyMap()
{
  if (m_docTag == neutube::Document::FLYEM_SPLIT ||
      m_docTag == neutube::Document::FLYEM_PROOFREAD) {
    m_plainKeyMap[Qt::Key_G] = ZSwcTree::OPERATION_NULL;
    m_plainKeyMap[Qt::Key_R] = ZSwcTree::OPERATION_NULL;
    m_controlKeyMap[Qt::Key_G] = ZSwcTree::OPERATION_ADD_NODE;
  }
}
