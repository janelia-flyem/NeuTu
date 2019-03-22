#ifndef ZFLYEMBODY3DDOCCOMMAND_H
#define ZFLYEMBODY3DDOCCOMMAND_H

#include "zstackdoccommand.h"
#include "zflyemtodoitem.h"

class ZFlyEmBody3dDoc;

namespace ZFlyEmBody3dDocCommand
{
class Base : public ZUndoCommand
{
public:
  Base(ZStackDoc *doc, QUndoCommand *parent = NULL);

  ZFlyEmBody3dDoc* getCompleteDocument() const;

protected:
  ZStackDoc *m_doc;
};

class AddTodo : public Base
{
public:
  AddTodo(ZStackDoc *doc, QUndoCommand *parent = NULL);

  void setTodoItem(int x, int y, int z, bool checked,
                   neutu::EToDoAction action, uint64_t bodyId);
  bool hasValidItem() const;

  void redo();
  void undo();

private:
  ZFlyEmToDoItem m_item;
  uint64_t m_bodyId;
};

class RemoveTodo : public Base
{
public:
  RemoveTodo(ZStackDoc *doc, QUndoCommand *parent = NULL);

  void addTodoItem(int x, int y, int z, uint64_t bodyId);
  bool hasValidItem() const;

  void redo();
  void undo();

private:
  QList<ZFlyEmToDoItem> m_itemList; //Items to remove. All elements are valid.
};

}

#endif // ZFLYEMBODY3DDOCCOMMAND_H
