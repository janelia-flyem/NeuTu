#ifndef ZFLYEMPROOFDOC_H
#define ZFLYEMPROOFDOC_H

#include "zstackdoc.h"
#include "zflyembodymerger.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoccommand.h"
#include "zflyembodysplitproject.h"

class ZDvidSparseStack;

class ZFlyEmProofDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDoc(ZStack *stack, QObject *parent = 0);

  static ZFlyEmProofDoc* Make();

  void mergeSelected();

  void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  void updateTileData();

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  ZDvidTileEnsemble* getDvidTileEnsemble() const;
  ZDvidLabelSlice* getDvidLabelSlice() const;
  ZDvidSparseStack* getDvidSparseStack() const;

  ZFlyEmBodyMerger* getBodyMerger() {
    return &m_bodyMerger;
  }

  void updateBodyObject();

  void clearData();

  bool isSplittable(uint64_t bodyId) const;

signals:

public slots:

private:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;
  ZFlyEmBodySplitProject m_splitProject;
};

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


#endif // ZFLYEMPROOFDOC_H
