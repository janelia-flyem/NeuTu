#ifndef ZFLYEMPROOFDOCCOMMAND_H
#define ZFLYEMPROOFDOCCOMMAND_H


#include "zstackdoccommand.h"

class ZFlyEmProofDoc;

namespace ZFlyEmProofDocCommand {
class MergeBody : public ZUndoCommand
{
public:
  MergeBody(ZStackDoc *doc, QUndoCommand *parent = NULL);
  void undo();
  void redo();

  ZFlyEmProofDoc* getCompleteDocument();

private:
  ZStackDoc *m_doc;
};

}

#endif // ZFLYEMPROOFDOCCOMMAND_H
