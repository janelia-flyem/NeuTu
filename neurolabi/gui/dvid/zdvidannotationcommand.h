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
}

#endif // ZDVIDANNOTATIONCOMMAND_H
