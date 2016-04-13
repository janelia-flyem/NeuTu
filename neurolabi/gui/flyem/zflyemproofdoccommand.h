#ifndef ZFLYEMPROOFDOCCOMMAND_H
#define ZFLYEMPROOFDOCCOMMAND_H

#include "zstackdoccommand.h"
#include "zflyembodymerger.h"

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

class UnmergeBody : public ZUndoCommand
{
public:
  UnmergeBody(ZStackDoc *doc, QUndoCommand *parent = NULL);
  void undo();
  void redo();

  ZFlyEmProofDoc* getCompleteDocument();

private:
  ZStackDoc *m_doc;
  ZFlyEmBodyMerger::TLabelMapList m_oldMapList;
  std::set<uint64_t> m_bodySet;
};

}

#endif // ZFLYEMPROOFDOCCOMMAND_H
