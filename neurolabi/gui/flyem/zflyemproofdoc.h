#ifndef ZFLYEMPROOFDOC_H
#define ZFLYEMPROOFDOC_H

#include <QString>

#include "zstackdoc.h"
#include "zflyembodymerger.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoccommand.h"
#include "zsharedpointer.h"
//#include "zflyembodysplitproject.h"
#include "flyem/zflyembodycolorscheme.h"

class ZDvidSparseStack;
class ZFlyEmSupervisor;
class ZFlyEmBookmark;
class ZPuncta;
class ZDvidSparseStack;
class ZIntCuboidObj;

class ZFlyEmProofDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDoc(QObject *parent = 0);

  static ZFlyEmProofDoc* Make();

  void mergeSelected(ZFlyEmSupervisor *supervisor);

  void setDvidTarget(const ZDvidTarget &target);

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

  const ZFlyEmBodyMerger* getBodyMerger() const {
    return &m_bodyMerger;
  }

  void updateBodyObject();

  void clearData();

  /*!
   * \brief Get body ID at a certain location
   *
   * \return The body ID mapped by merge operations.
   */
  uint64_t getBodyId(int x, int y, int z);
  uint64_t getBodyId(const ZIntPoint &pt);

  std::set<uint64_t> getSelectedBodySet(NeuTube::EBodyLabelType labelType) const;
  void setSelectedBody(
      std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);
  void setSelectedBody(uint64_t bodyId, NeuTube::EBodyLabelType labelType);

  bool isSplittable(uint64_t bodyId) const;

  void backupMergeOperation();
  void saveMergeOperation();
  void downloadBodyMask();
  void clearBodyMerger();

  QList<uint64_t> getMergedSource(uint64_t bodyId) const;
  QSet<uint64_t> getMergedSource(const QSet<uint64_t> &bodySet) const;

  void importFlyEmBookmark(const std::string &filePath);
  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

  void saveCustomBookmark();
  void downloadBookmark();
  inline void setCustomBookmarkSaveState(bool state) {
    m_isCustomBookmarkSaved = state;
  }

  ZDvidSparseStack* getDvidSparseStack() const;

  void enhanceTileContrast(bool highContrast);

  void annotateBody(uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation);
  void useBodyNameMap(bool on);

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();
  void notifyBodyIsolated(uint64_t bodyId);

public: //ROI functions
  ZIntCuboidObj* getSplitRoi() const;
  void updateSplitRoi();

signals:
  void bodyMerged();
  void bodyUnmerged();
  void userBookmarkModified();
  void bodyIsolated(uint64_t bodyId);
  void bodySelectionChanged();

public slots:
  void updateDvidLabelObject();
  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
  void saveCustomBookmarkSlot();
  void deprecateSplitSource();

protected:
  void autoSave();
  void customNotifyObjectModified(ZStackObject::EType type);

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

  void init();
  void initTimer();
  void initAutoSave();

private:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;

  bool m_isCustomBookmarkSaved;
  QTimer *m_bookmarkTimer;

  QString m_mergeAutoSavePath;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_bodyColorMap;

  mutable ZSharedPointer<ZDvidSparseStack> m_splitSource;
//  mutable ZIntCuboid m_splitRoi;


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
