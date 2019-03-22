#include "zflyembody3ddoccommand.h"
#include "zflyembody3ddoc.h"

ZFlyEmBody3dDocCommand::Base::Base(ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{
}

ZFlyEmBody3dDoc* ZFlyEmBody3dDocCommand::Base::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmBody3dDoc*>(m_doc);
}

ZFlyEmBody3dDocCommand::AddTodo::AddTodo(ZStackDoc *doc, QUndoCommand *parent)
  : Base(doc, parent)
{
}


void ZFlyEmBody3dDocCommand::AddTodo::setTodoItem(
    int x, int y, int z, bool checked, neutu::EToDoAction action,
    uint64_t bodyId)
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    m_item = doc->makeTodoItem(x, y, z, checked, bodyId);
    m_item.setAction(action);
    m_bodyId = bodyId;
  }
}

bool ZFlyEmBody3dDocCommand::AddTodo::hasValidItem() const
{
  return m_item.isValid();
}

void ZFlyEmBody3dDocCommand::AddTodo::redo()
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL && m_item.isValid()) {
    doc->addTodo(m_item, m_bodyId);
  }
}

void ZFlyEmBody3dDocCommand::AddTodo::undo()
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL && m_item.isValid()) {
    doc->removeTodo(m_item, m_bodyId);
  }
}

/////////////////////////////////////////////////

ZFlyEmBody3dDocCommand::RemoveTodo::RemoveTodo(
    ZStackDoc *doc, QUndoCommand *parent) : Base(doc, parent)
{
}

void ZFlyEmBody3dDocCommand::RemoveTodo::addTodoItem(
    int x, int y, int z, uint64_t bodyId)
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    ZFlyEmToDoItem item = doc->readTodoItem(x, y, z);
    if (item.isValid()) {
      item.setBodyId(bodyId);
      m_itemList.append(item);
    }
  }
}

void ZFlyEmBody3dDocCommand::RemoveTodo::redo()
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL && !m_itemList.isEmpty()) {
    doc->removeTodo(m_itemList);
  }
}

bool ZFlyEmBody3dDocCommand::RemoveTodo::hasValidItem() const
{
  return !m_itemList.isEmpty();
}

void ZFlyEmBody3dDocCommand::RemoveTodo::undo()
{
  ZFlyEmBody3dDoc *doc = getCompleteDocument();
  if (doc != NULL && !m_itemList.isEmpty()) {
    doc->addTodo(m_itemList);
  }
}


