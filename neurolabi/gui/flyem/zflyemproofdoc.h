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
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidsynapseensenmble.h"

class ZDvidSparseStack;
class ZFlyEmSupervisor;
class ZFlyEmBookmark;
class ZPuncta;
class ZDvidSparseStack;
class ZIntCuboidObj;
class ZSlicedPuncta;
class ZFlyEmSequencerColorScheme;

class ZFlyEmProofDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDoc(QObject *parent = 0);

  static ZFlyEmProofDoc* Make();

  enum EBodyColorMap {
    BODY_COLOR_NORMAL, BODY_COLOR_NAME, BODY_COLOR_SEQUENCER
  };

  void mergeSelected(ZFlyEmSupervisor *supervisor);

  void setDvidTarget(const ZDvidTarget &target);

  virtual void updateTileData();

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  ZDvidTileEnsemble* getDvidTileEnsemble() const;
  ZDvidLabelSlice* getDvidLabelSlice(NeuTube::EAxis axis) const;
//  QList<ZDvidLabelSlice*> getDvidLabelSlice() const;
  QList<ZDvidSynapseEnsemble*> getDvidSynapseEnsembleList() const;
  ZDvidSynapseEnsemble* getDvidSynapseEnsemble(NeuTube::EAxis axis) const;

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
  void downloadBodyMask();
  void clearBodyMerger();

  QList<uint64_t> getMergedSource(uint64_t bodyId) const;
  QSet<uint64_t> getMergedSource(const QSet<uint64_t> &bodySet) const;

  void importFlyEmBookmark(const std::string &filePath);
  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

//  void saveCustomBookmark();
  void downloadBookmark();
//  inline void setCustomBookmarkSaveState(bool state) {
//    m_isCustomBookmarkSaved = state;
//  }

  ZDvidSparseStack* getDvidSparseStack() const;

  void enhanceTileContrast(bool highContrast);

  void annotateBody(uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation);
//  void useBodyNameMap(bool on);

  void selectBody(uint64_t bodyId);
  template <typename InputIterator>
  void selectBody(const InputIterator &first, const InputIterator &last);

  void recordBodySelection();
  void processBodySelection();

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

  void verifyBodyAnnotationMap();

  /*!
   * \brief Remove unselected bodies from annotation map.
   *
   * This is a temporary solution to inconsistent selection update.
   */
  void cleanBodyAnnotationMap();

  void activateBodyColorMap(const QString &option);
  void activateBodyColorMap(EBodyColorMap colorMap);

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();
  void notifyBodyMergeEdited();
  void notifyBodyIsolated(uint64_t bodyId);

public: //ROI functions
  ZIntCuboidObj* getSplitRoi() const;
  void updateSplitRoi(ZRect2d *rect, bool appending);
  void selectBodyInRoi(int z, bool appending);

public: //Synapse functions
  std::set<ZIntPoint> getSelectedSynapse() const;
  bool hasDvidSynapseSelected() const;
  bool hasDvidSynapse() const;
  void tryMoveSelectedSynapse(const ZIntPoint &dest, NeuTube::EAxis axis);

  void removeSynapse(
      const ZIntPoint &pos, ZDvidSynapseEnsemble::EDataScope scope);
  void addSynapse(
      const ZDvidSynapse &synapse, ZDvidSynapseEnsemble::EDataScope scope);
  void moveSynapse(const ZIntPoint &from, const ZIntPoint &to);
  void updateSynapsePartner(const ZIntPoint &pos);
  void updateSynapsePartner(const std::set<ZIntPoint> &posArray);


public: //Bookmark functions
  void removeLocalBookmark(ZFlyEmBookmark *bookmark);
  void removeLocalBookmark(const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void addLocalBookmark(ZFlyEmBookmark *bookmark);
  void addLocalBookmark(const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void notifyBookmarkEdited(
      const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void notifyBookmarkEdited(const ZFlyEmBookmark *bookmark);
  void notifySynapseEdited(const ZDvidSynapse &synapse);
  void notifySynapseEdited(const ZIntPoint &synapse);
  void updateLocalBookmark(ZFlyEmBookmark *bookmark);
  void copyBookmarkFrom(const ZFlyEmProofDoc *doc);

  /*!
   * \brief Find a bookmark at a certain location
   *
   * Return the pointer of the bookmark with the coordinates (\a x, \a y, \a z).
   * It returns NULL if the bookmark cannot be found.
   *
   */
  ZFlyEmBookmark* getBookmark(int x, int y, int z) const;

public: //Commands
  void executeRemoveSynapseCommand();
  void executeLinkSynapseCommand();
  void executeUnlinkSynapseCommand();
  void executeAddSynapseCommand(const ZDvidSynapse &synapse);
  void executeMoveSynapseCommand(const ZIntPoint &dest);

  void executeRemoveBookmarkCommand();
  void executeRemoveBookmarkCommand(ZFlyEmBookmark *bookmark);
  void executeRemoveBookmarkCommand(const QList<ZFlyEmBookmark*> &bookmarkList);
  void executeAddBookmarkCommand(ZFlyEmBookmark *bookmark);

signals:
  void bodyMerged();
  void bodyUnmerged();
  void bodyMergeEdited();
  void userBookmarkModified();
  void bookmarkAdded(int x, int y, int z);
  void bookmarkEdited(int x, int y, int z);
  void synapseEdited(int x, int y, int z);
  void bodyIsolated(uint64_t bodyId);
  void bodySelectionChanged();
  void bodyMapReady();

public slots:
  void updateDvidLabelObject();
  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void downloadSynapse(int x, int y, int z);
  void downloadTodoList();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
//  void saveCustomBookmarkSlot();
  void deprecateSplitSource();
  void prepareNameBodyMap(const ZJsonValue &bodyInfoObj);
  void clearBodyMergeStage();
  void updateSequencerBodyMap(const ZFlyEmSequencerColorScheme &colorScheme);
  void deleteSelectedSynapse();
  void addSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind);

  void downloadBookmark(int x, int y, int z);
  void saveMergeOperation();

protected:
  void autoSave();
  void customNotifyObjectModified(ZStackObject::EType type);
  void updateDvidTargetForObject();
  virtual void prepareDvidData();
  void addDvidLabelSlice(NeuTube::EAxis axis);

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

  void decorateTBar(ZSlicedPuncta *puncta);
  void decoratePsd(ZSlicedPuncta *puncta);

  void init();
  void initTimer();
  void initAutoSave();

  /*!
   * \brief Create essential data instance if necessary
   */
  void initData(const ZDvidTarget &target);
  void initData(const std::string &type, const std::string &dataName);

  ZSharedPointer<ZFlyEmBodyColorScheme> getColorScheme(EBodyColorMap type);
  template<typename T>
  ZSharedPointer<T> getColorScheme(EBodyColorMap type);

  bool isActive(EBodyColorMap type);

  void updateBodyColor(EBodyColorMap type);

protected:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_dvidReader;
  ZDvidWriter m_dvidWriter;

//  bool m_isCustomBookmarkSaved;
  QTimer *m_bookmarkTimer;

  QString m_mergeAutoSavePath;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_activeBodyColorMap;
  QMap<EBodyColorMap, ZSharedPointer<ZFlyEmBodyColorScheme> > m_colorMapConfig;
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
