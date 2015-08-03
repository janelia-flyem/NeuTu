#ifndef ZFLYEMPROOFDOC_H
#define ZFLYEMPROOFDOC_H

#include "zstackdoc.h"
#include "zflyembodymerger.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoccommand.h"
//#include "zflyembodysplitproject.h"

class ZDvidSparseStack;
class ZFlyEmSupervisor;
class ZFlyEmBookmark;
class ZPuncta;

class ZFlyEmProofDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDoc(QObject *parent = 0);

  static ZFlyEmProofDoc* Make();

  void mergeSelected(ZFlyEmSupervisor *supervisor);

  void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  void updateTileData();

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  ZDvidTileEnsemble* getDvidTileEnsemble() const;
  ZDvidLabelSlice* getDvidLabelSlice() const;
  const ZDvidSparseStack* getBodyForSplit() const;
  ZDvidSparseStack* getBodyForSplit();

  const ZSparseStack* getSparseStack() const;
  ZSparseStack* getSparseStack();

  //bool hasSparseStack() const;
  bool hasVisibleSparseStack() const;

  ZFlyEmBodyMerger* getBodyMerger() {
    return &m_bodyMerger;
  }

  void updateBodyObject();

  void clearData();

  bool isSplittable(uint64_t bodyId) const;

  void saveMergeOperation();
  void downloadBodyMask();
  void clearBodyMerger();

  QList<uint64_t> getMergedSource(uint64_t bodyId) const;
  QSet<uint64_t> getMergedSource(const QSet<uint64_t> &bodySet) const;

  void importFlyEmBookmark(const std::string &filePath);
  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

  /*!
   * \brief Get body ID at a certain location
   *
   * \return The body ID mapped by merge operations.
   */
  uint64_t getBodyId(int x, int y, int z);
  uint64_t getBodyId(const ZIntPoint &pt);

  void saveCustomBookmark();
  void downloadBookmark();

  void enhanceTileContrast(bool highContrast);

  inline void setCustomBookmarkSaveState(bool state) {
    m_isCustomBookmarkSaved = state;
  }

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();

signals:
  void bodyMerged();
  void bodyUnmerged();
  void userBookmarkModified();

public slots:
  void updateDvidLabelObject();
  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
  void saveCustomBookmarkSlot();

protected:
  void autoSave();
  void customNotifyObjectModified(ZStackObject::EType type);

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

private:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;

  bool m_isCustomBookmarkSaved;
  QTimer *m_bookmarkTimer;
  //ZFlyEmBodySplitProject m_splitProject;
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
