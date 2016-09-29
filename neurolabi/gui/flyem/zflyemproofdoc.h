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
#include "flyem/zflyemtodolist.h"
#include "flyem/zflyemmb6analyzer.h"
#include "dvid/zdvidversiondag.h"

class ZDvidSparseStack;
class ZFlyEmSupervisor;
class ZFlyEmBookmark;
class ZPuncta;
class ZDvidSparseStack;
class ZIntCuboidObj;
class ZSlicedPuncta;
class ZFlyEmSequencerColorScheme;
class ZFlyEmSynapseAnnotationDialog;


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
  void unmergeSelected();

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

  ZStackBlockGrid* getStackGrid();

  //bool hasSparseStack() const;
  bool hasVisibleSparseStack() const;

  ZFlyEmBodyMerger* getBodyMerger() {
    return &m_bodyMerger;
  }

  const ZFlyEmBodyMerger* getBodyMerger() const {
    return &m_bodyMerger;
  }

  ZFlyEmSupervisor* getSupervisor() const;

  void updateBodyObject();

  void clearData();

  /*!
   * \brief Get body ID at a certain location
   *
   * \return The body ID mapped by merge operations.
   */
  uint64_t getBodyId(int x, int y, int z);
  uint64_t getBodyId(const ZIntPoint &pt);

  bool hasBodySelected() const;

  std::set<uint64_t> getSelectedBodySet(NeuTube::EBodyLabelType labelType) const;
  void setSelectedBody(
      std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);
  void setSelectedBody(uint64_t bodyId, NeuTube::EBodyLabelType labelType);

  void addSelectedBody(
      std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);

  bool isSplittable(uint64_t bodyId) const;

  void backupMergeOperation();
//  void downloadBodyMask();
  void clearBodyMerger();

  QList<uint64_t> getMergedSource(uint64_t bodyId) const;
  QSet<uint64_t> getMergedSource(const QSet<uint64_t> &bodySet) const;

  QList<ZFlyEmBookmark*> importFlyEmBookmark(const std::string &filePath);
  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

//  void saveCustomBookmark();
  void downloadBookmark();
//  inline void setCustomBookmarkSaveState(bool state) {
//    m_isCustomBookmarkSaved = state;
//  }

  ZDvidSparseStack* getDvidSparseStack() const;
  ZDvidSparseStack* getDvidSparseStack(const ZIntCuboid &roi) const;

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
  getSynapse(uint64_t bodyId) const;

  std::vector<ZPunctum*> getTodoPuncta(uint64_t bodyId) const;
  std::vector<ZFlyEmToDoItem*> getTodoItem(uint64_t bodyId) const;

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

  ZDvidReader& getDvidReader() {
    return m_dvidReader;
  }

  const ZDvidReader& getDvidReader() const {
    return m_dvidReader;
  }

  ZDvidWriter& getDvidWriter() {
    return m_dvidWriter;
  }

public:
  void runSplit();
  void runLocalSplit();
  void refreshDvidLabelBuffer(unsigned long delay);

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();
  void notifyBodyMergeEdited();
  void notifyBodyIsolated(uint64_t bodyId);
  void notifyBodyLock(uint64_t bodyId, bool locking);

public: //ROI functions
  ZIntCuboidObj* getSplitRoi() const;
  void updateSplitRoi(ZRect2d *rect, bool appending);
  void selectBodyInRoi(int z, bool appending, bool removingRoi);

public: //Synapse functions
  std::set<ZIntPoint> getSelectedSynapse() const;
  bool hasDvidSynapseSelected() const;
  bool hasDvidSynapse() const;
  void tryMoveSelectedSynapse(const ZIntPoint &dest, NeuTube::EAxis axis);
  void annotateSelectedSynapse(ZJsonObject propJson, NeuTube::EAxis axis);
  void annotateSelectedSynapse(ZFlyEmSynapseAnnotationDialog *dlg,
                               NeuTube::EAxis axis);

  /*!
   * \brief Sync the synapse with DVID
   *
   * If \a pt does not exist in DVID, it will be removed from all synapse
   * ensembles. If it exists, the function checks the partners of the synapses
   * and try to remove all 'ghost' partners. For each of actual partners, the
   * function checks the relationship consistency and apply fixes based on the
   * following rules:
   *   1. If the host synapse (the one at \a pt) rels to its partner but not
   * vice versa, the missing relationship will be added to the partner.
   *   2. If the host synapse rels to its partner and vice versa, but
   * relationships are not consistent, the partner synapse is changed to have
   * the consistency.
   *
   * \param pt Synapse position to sync.
   */
  void repairSynapse(const ZIntPoint &pt);

  void removeSynapse(
      const ZIntPoint &pos, ZDvidSynapseEnsemble::EDataScope scope);
  void addSynapse(
      const ZDvidSynapse &synapse, ZDvidSynapseEnsemble::EDataScope scope);
  void moveSynapse(
      const ZIntPoint &from, const ZIntPoint &to,
      ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL);
  void updateSynapsePartner(const ZIntPoint &pos);
  void updateSynapsePartner(const std::set<ZIntPoint> &posArray);
  void highlightPsd(bool on);

public: //Todo list functions
  void removeTodoItem(
      const ZIntPoint &pos, ZFlyEmToDoList::EDataScope scope);
  void addTodoItem(const ZIntPoint &pos);
  void addTodoItem(const ZFlyEmToDoItem &item, ZFlyEmToDoList::EDataScope scope);
  bool hasTodoItemSelected() const;
  void checkTodoItem(bool checking);

  void notifyTodoItemModified(
      const std::vector<ZIntPoint> &ptArray, bool emitingEdit = false);
  void notifyTodoItemModified(const ZIntPoint &pt, bool emitingEdit = false);

  std::set<ZIntPoint> getSelectedTodoItemPosition() const;

public: //Bookmark functions
  void removeLocalBookmark(ZFlyEmBookmark *bookmark);
  void removeLocalBookmark(const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void addLocalBookmark(ZFlyEmBookmark *bookmark);
  void addLocalBookmark(const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void notifyBookmarkEdited(
      const std::vector<ZFlyEmBookmark *> &bookmarkArray);
  void notifyBookmarkEdited(const ZFlyEmBookmark *bookmark);
  void notifyAssignedBookmarkModified();
  void notifySynapseEdited(const ZDvidSynapse &synapse);
  void notifySynapseEdited(const ZIntPoint &synapse);
  void notifySynapseMoved(const ZIntPoint &from, const ZIntPoint &to);

  void notifyTodoEdited(const ZIntPoint &item);
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

signals:
  void bodyMerged();
  void bodyUnmerged();
  void bodyMergeEdited();
  void userBookmarkModified();
  void assignedBookmarkModified();
  void bookmarkAdded(int x, int y, int z);
  void bookmarkEdited(int x, int y, int z);
  void bookmarkDeleted(int x, int y, int z);
  void bookmarkModified(int x, int y, int z);

  void synapseEdited(int x, int y, int z);
  void synapseVerified(int x, int y, int z, bool verified);
  void synapseMoved(const ZIntPoint &from, const ZIntPoint &to);
//  void synapseUnverified(int x, int y, int z);
  void todoEdited(int x, int y, int z);
  void bodyIsolated(uint64_t bodyId);
  void bodySelectionChanged();
  void bodyMapReady();
  void todoModified(uint64_t bodyId);
  void requestingBodyLock(uint64_t bodyId, bool locking);

public slots: //Commands
  void repairSelectedSynapses();
  void executeRemoveSynapseCommand();
  void executeLinkSynapseCommand();
  void executeUnlinkSynapseCommand();
  void executeAddSynapseCommand(const ZDvidSynapse &synapse, bool tryingLink);
  void executeMoveSynapseCommand(const ZIntPoint &dest);

  void executeRemoveBookmarkCommand();
  void executeRemoveBookmarkCommand(ZFlyEmBookmark *bookmark);
  void executeRemoveBookmarkCommand(const QList<ZFlyEmBookmark*> &bookmarkList);
  void executeAddBookmarkCommand(ZFlyEmBookmark *bookmark);

  void executeAddTodoItemCommand(int x, int y, int z, bool checked);
  void executeAddTodoItemCommand(const ZIntPoint &pt, bool checked);
  void executeAddTodoItemCommand(ZFlyEmToDoItem &item);
  void executeRemoveTodoItemCommand();


public slots:
  void updateDvidLabelObject();
  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void downloadSynapse(int x, int y, int z);
  void downloadTodo(int x, int y, int z);
  void downloadTodoList();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
//  void saveCustomBookmarkSlot();
  void deprecateSplitSource();
  void prepareNameBodyMap(const ZJsonValue &bodyInfoObj);
  void clearBodyMergeStage();
  void updateSequencerBodyMap(const ZFlyEmSequencerColorScheme &colorScheme);
  void deleteSelectedSynapse();
  void addSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind,
                  ZDvidSynapseEnsemble::EDataScope scope);
  void verifySelectedSynapse();
  void unverifySelectedSynapse();

  void downloadBookmark(int x, int y, int z);
  void saveMergeOperation();
  void rewriteSegmentation();

  void syncSynapse(const ZIntPoint &pt);
  void syncMoveSynapse(const ZIntPoint &from, const ZIntPoint &to);

  void runRoutineCheck();

protected:
  void autoSave();
  void customNotifyObjectModified(ZStackObject::EType type);
  void updateDvidTargetForObject();
  virtual void prepareDvidData();
  void addDvidLabelSlice(NeuTube::EAxis axis);
  void annotateSynapse(
      const ZIntPoint &pt, ZJsonObject propJson, NeuTube::EAxis axis);
  void setRoutineCheck(bool on);

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

  void runSplitFunc();
  void localSplitFunc();
  ZIntCuboid estimateSplitRoi();
  ZIntCuboid estimateLocalSplitRoi();

  void readBookmarkBodyId(QList<ZFlyEmBookmark*> &bookmarkArray);

protected:
  ZFlyEmBodyMerger m_bodyMerger;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_dvidReader;
  ZDvidReader m_routineReader;
  ZDvidWriter m_dvidWriter;
  ZFlyEmSupervisor *m_supervisor;

  //Dvid info
  ZDvidInfo m_dvidInfo;
  ZDvidVersionDag m_versionDag;

//  bool m_isCustomBookmarkSaved;
//  QTimer *m_bookmarkTimer;
  QTimer *m_routineTimer;

  QString m_mergeAutoSavePath;
  bool m_loadingAssignedBookmark; //temporary solution for updating bookmark table
  bool m_routineCheck;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_activeBodyColorMap;
  QMap<EBodyColorMap, ZSharedPointer<ZFlyEmBodyColorScheme> > m_colorMapConfig;
  QMap<uint64_t, ZFlyEmBodyAnnotation> m_annotationMap; //for Original ID

  mutable ZFlyEmMB6Analyzer m_analyzer;

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



#endif // ZFLYEMPROOFDOC_H
