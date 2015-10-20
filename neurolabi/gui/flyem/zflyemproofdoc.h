#ifndef ZFLYEMPROOFDOC_H
#define ZFLYEMPROOFDOC_H

#include <QString>
#include <QMap>

#include "zstackdoc.h"
#include "zflyembodymerger.h"
#include "dvid/zdvidtarget.h"
#include "zstackdoccommand.h"
#include "zsharedpointer.h"
//#include "zflyembodysplitproject.h"
#include "flyem/zflyembodycolorscheme.h"
#include "zflyembodyannotation.h"

class ZDvidSparseStack;
class ZFlyEmSupervisor;
class ZFlyEmBookmark;
class ZPuncta;
class ZDvidSparseStack;
class ZIntCuboidObj;
class ZSlicedPuncta;

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

  void addSelectedBody(
      std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);

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

  void selectBody(uint64_t bodyId);
  template <typename InputIterator>
  void selectBody(const InputIterator &first, const InputIterator &last);

  std::vector<ZPunctum*> getTbar(uint64_t bodyId);
  std::vector<ZPunctum*> getTbar(ZObject3dScan &body);

  std::pair<std::vector<ZPunctum *>, std::vector<ZPunctum *> >
  getSynapse(uint64_t bodyId);


  void downloadSynapseFunc();

  void recordAnnotation(uint64_t bodyId, const ZFlyEmBodyAnnotation &anno);
  void removeSelectedAnnotation(uint64_t bodyId);
  template <typename InputIterator>
  void removeSelectedAnnotation(
      const InputIterator &first, const InputIterator &last);

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();
  void notifyBodyIsolated(uint64_t bodyId);

public: //ROI functions
  ZIntCuboidObj* getSplitRoi() const;
  void updateSplitRoi(ZRect2d *rect);
  void selectBodyInRoi(int z, bool appending);

signals:
  void bodyMerged();
  void bodyUnmerged();
  void userBookmarkModified();
  void bodyIsolated(uint64_t bodyId);
  void bodySelectionChanged();
  void bodyMapReady();

public slots:
  void updateDvidLabelObject();
  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
  void saveCustomBookmarkSlot();
  void deprecateSplitSource();
  void prepareBodyMap(const ZJsonValue &bodyInfoObj);

protected:
  void autoSave();
  void customNotifyObjectModified(ZStackObject::EType type);

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

  void decorateTBar(ZSlicedPuncta *puncta);
  void decoratePsd(ZSlicedPuncta *puncta);

  void init();
  void initTimer();
  void initAutoSave();

private:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_dvidReader;

  bool m_isCustomBookmarkSaved;
  QTimer *m_bookmarkTimer;

  QString m_mergeAutoSavePath;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_bodyColorMap;
  QMap<uint64_t, ZFlyEmBodyAnnotation> m_annotationMap; //for Original ID

  mutable ZSharedPointer<ZDvidSparseStack> m_splitSource;
};

template <typename InputIterator>
void ZFlyEmProofDoc::selectBody(
    const InputIterator &first, const InputIterator &last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    selectBody(*iter);
  }
}

template <typename InputIterator>
void ZFlyEmProofDoc::removeSelectedAnnotation(
    const InputIterator &first, const InputIterator &last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    removeSelectedAnnotation(*iter);
  }
}

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
