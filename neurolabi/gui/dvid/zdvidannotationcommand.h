#ifndef ZDVIDANNOTATIONCOMMAND_H
#define ZDVIDANNOTATIONCOMMAND_H

#include "zstackdoccommand.h"

class ZFlyEmProofDoc;

namespace ZStackDocCommand
{
namespace DvidAnnotationEdit {
class CompositeCommand : public ZUndoCommand
{
public:
  CompositeCommand(ZFlyEmProofDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CompositeCommand();

  void redo();
  void undo();
protected:
  ZFlyEmProofDoc *m_doc;
  bool m_isExecuted;
};
}

namespace FlyEmToDoItemEdit {
class RemoveItem : public ZUndoCommand
{
public:
  RemoveItem(ZFlyEmProofDoc *doc, int x, int y, int z,
             QUndoCommand *parent = NULL);
  virtual ~RemoveItem();
  void undo();
  void redo();

private:
  ZFlyEmProofDoc *m_doc;
  ZIntPoint m_item;
  ZJsonObject m_bakcup;
};
}
}

#endif // ZDVIDANNOTATIONCOMMAND_H
