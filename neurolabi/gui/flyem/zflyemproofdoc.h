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
#include "zflyembodymergeproject.h"
#include "flyem/zflyembodycoloroption.h"

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

  /*
  enum EBodyColorMap {
    BODY_COLOR_NORMAL, BODY_COLOR_NAME, BODY_COLOR_SEQUENCER, BODY_COLOR_FOCUSED
  };
  */

//  void makeAction(ZActionFactory::EAction item);

  const ZDvidVersionDag& getVersionDag() const {
    return m_versionDag;
  }

  void mergeSelected(ZFlyEmSupervisor *supervisor);
  void mergeSelectedWithoutConflict(ZFlyEmSupervisor *supervisor);
  void unmergeSelected();

  void setDvidTarget(const ZDvidTarget &target);

  virtual void updateTileData();

  const ZDvidTarget& getDvidTarget() const;

  const ZDvidInfo& getGrayScaleInfo() const {
    return m_grayScaleInfo;
  }

  const ZDvidInfo& getLabelInfo() const {
    return m_labelInfo;
  }

  ZDvidTileEnsemble* getDvidTileEnsemble() const;
  ZDvidLabelSlice* getDvidLabelSlice(NeuTube::EAxis axis) const;
  ZDvidGraySlice* getDvidGraySlice() const;
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
  void setSelectedBody(const std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);
  void setSelectedBody(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  void toggleBodySelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  /*!
   * \brief Deselect bodies
   *
   * Deselect bodies that have the same mapped ID as that of \a bodyId.
   */
  void deselectMappedBody(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  void deselectMappedBody(const std::set<uint64_t> &bodySet, NeuTube::EBodyLabelType labelType);

  void addSelectedBody(
      const std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);

  bool isSplittable(uint64_t bodyId) const;

  ZFlyEmBodyMerger* getBodyMerger() {
    return &m_bodyMerger;
  }

  const ZFlyEmBodyMerger* getBodyMerger() const {
    return &m_bodyMerger;
  }

  ZFlyEmBodyMergeProject* getMergeProject() {
    return m_mergeProject;
  }

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
  void syncBodySelection(ZDvidLabelSlice *labelSlice);

  std::vector<ZPunctum*> getTbar(uint64_t bodyId);
  std::vector<ZPunctum*> getTbar(ZObject3dScan &body);

  std::pair<std::vector<ZPunctum *>, std::vector<ZPunctum *> >
  getSynapse(uint64_t bodyId);

  std::vector<ZPunctum*> getTodoPuncta(uint64_t bodyId);
  std::vector<ZFlyEmToDoItem*> getTodoItem(uint64_t bodyId);

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

  void activateBodyColorMap(const QString &colorMapName);
  bool isActive(ZFlyEmBodyColorOption::EColorOption option);

  ZDvidReader& getDvidReader() {
    return m_dvidReader;
  }

  const ZDvidReader& getDvidReader() const {
    return m_dvidReader;
  }

  ZDvidWriter& getDvidWriter() {
    return m_dvidWriter;
  }

  ZDvidReader* getSparseVolReader() {
    return &m_sparseVolReader;
  }

public:
  void runSplit();
  bool isSplitRunning() const;
  void runLocalSplit();
  void refreshDvidLabelBuffer(unsigned long delay);
//  void setLabelSliceAlpha(int alpha);

public:
  void notifyBodyMerged();
  void notifyBodyUnmerged();
  void notifyBodyMergeSaved();
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
  void setTodoItemAction(ZFlyEmToDoItem::EToDoAction action);
  void setTodoItemToNormal();
  void setTodoItemToMerge();
  void setTodoItemToSplit();

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

public:
  bool isDataValid(const std::string &data) const;
  void notifyBodySelectionChanged();

  /*!
   * \brief Fetch DVID label slice data and set body selections
   */
  void updateDvidLabelSlice(NeuTube::EAxis axis);
//  void updateDvidLabelSlice();

  /*!
   * \brief Update sparsevol based on current body selections
   */
//  void updateDvidSparsevolSlice();

  /*!
   * \brief Factory function to make a new ZDvidSparsevolSlice object
   *
   * \a labelSlice is used to determine some properties including color and axis.
   *
   * \param labelSlice Base slice for the sparsevol object
   * \param bodyId Body ID of the returned object.
   *
   * \return A new object or NULL if \a bodyId is invalid.
   */
  ZDvidSparsevolSlice* makeDvidSparsevol(
      const ZDvidLabelSlice *labelSlice, uint64_t bodyId);

  /*!
   * \brief Factory function to make new ZDvidSparsevolSlice objects
   *
   * The objects created are those selected in \a labelSlice, which is also used
   * to determine some properties including color and axis.
   *
   * \param labelSlice Base slice for the sparsevol object
   *
   * \return A list of new objects.
   */
  std::vector<ZDvidSparsevolSlice*> makeSelectedDvidSparsevol(
      const ZDvidLabelSlice *labelSlice);
  std::vector<ZDvidSparsevolSlice*> makeSelectedDvidSparsevol();

  /*!
   * \brief Remove certain dvid sparsevol objects
   *
   * \param axis Axis of the objects to remove
   * \return Number of objects removed
   */
  int removeDvidSparsevol(NeuTube::EAxis axis);

  void loadSplitFromService();
  void commitSplitFromService();

signals:
  void bodyMerged();
  void bodyUnmerged();
  void bodyMergeSaved();
  void bodyMergeEdited();
  void bodyMergeUploaded();
  void bodyMergeUploadedExternally();

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

  void executeAddTodoItemCommand(int x, int y, int z, bool checked, uint64_t bodyId = 0);
  void executeAddTodoItemCommand(const ZIntPoint &pt, bool checked, uint64_t bodyId = 0);
  void executeAddTodoItemCommand(
      int x, int y, int z, ZFlyEmToDoItem::EToDoAction action, uint64_t bodyId = 0);
  void executeAddTodoItemCommand(ZFlyEmToDoItem &item);
  void executeAddToMergeItemCommand(int x, int y, int z, uint64_t bodyId = 0);
  void executeAddToMergeItemCommand(const ZIntPoint &pt, uint64_t bodyId = 0);
  void executeAddToSplitItemCommand(int x, int y, int z, uint64_t bodyId = 0);
  void executeAddToSplitItemCommand(const ZIntPoint &pt, uint64_t bodyId = 0);
  void executeRemoveTodoItemCommand();


public slots:
  void syncMergeWithDvid();
  void saveMergeOperation();
  void processExternalBodyMergeUpload();
  void clearBodyMergeStage();
  void uploadMergeResult();

//  void updateDvidLabelObject();

  void updateDvidLabelObject(EObjectModifiedMode updateMode);
  void updateDvidLabelObjectSliently();
  void updateDvidLabelObject(NeuTube::EAxis axis);


  void loadSynapse(const std::string &filePath);
  void downloadSynapse();
  void downloadSynapse(int x, int y, int z);
  void downloadTodo(int x, int y, int z);
  void downloadTodoList();
  void processBookmarkAnnotationEvent(ZFlyEmBookmark *bookmark);
//  void saveCustomBookmarkSlot();
  void deprecateSplitSource();
  void prepareNameBodyMap(const ZJsonValue &bodyInfoObj);

  void updateSequencerBodyMap(const ZFlyEmSequencerColorScheme &colorScheme);
  void updateFocusedColorMap(const ZFlyEmSequencerColorScheme &colorScheme);

  void deleteSelectedSynapse();
  void addSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind,
                  ZDvidSynapseEnsemble::EDataScope scope);
  void verifySelectedSynapse();
  void unverifySelectedSynapse();

  void deselectMappedBodyWithOriginalId(const std::set<uint64_t> &bodySet);
  void checkInSelectedBody();
  void checkInSelectedBodyAdmin();
  void checkOutBody();
  bool checkOutBody(uint64_t bodyId);
  bool checkInBody(uint64_t bodyId);
  bool checkInBodyWithMessage(uint64_t bodyId);
  bool checkBodyWithMessage(uint64_t bodyId, bool checkingOut);

  void downloadBookmark(int x, int y, int z);  
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
  uint64_t getBodyIdForSplit() const;
  QColor getSeedColor(int label) const;

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

  void decorateTBar(ZSlicedPuncta *puncta);
  void decoratePsd(ZSlicedPuncta *puncta);
  void loadRoiFunc();

  std::set<uint64_t> getCurrentSelectedBodyId(NeuTube::EBodyLabelType type) const;

  void init();
  void initTimer();
  void initAutoSave();
  void startTimer();

  QString getBodySelectionMessage() const;

  /*!
   * \brief Create essential data instance if necessary
   */
  void initData(const ZDvidTarget &target);
  void initData(const std::string &type, const std::string &dataName);

  void readInfo();
  void updateMaxLabelZoom();
  void updateMaxGrayscaleZoom();

  ZSharedPointer<ZFlyEmBodyColorScheme> getColorScheme(
      ZFlyEmBodyColorOption::EColorOption type);
  template<typename T>
  ZSharedPointer<T> getColorScheme(ZFlyEmBodyColorOption::EColorOption type);

  void updateBodyColor(ZFlyEmBodyColorOption::EColorOption type);

  void runSplitFunc();
  void localSplitFunc();
  ZIntCuboid estimateSplitRoi();
  ZIntCuboid estimateLocalSplitRoi();

  void readBookmarkBodyId(QList<ZFlyEmBookmark*> &bookmarkArray);


  void updateSequencerBodyMap(
      const ZFlyEmSequencerColorScheme &colorScheme,
      ZFlyEmBodyColorOption::EColorOption option);

  void activateBodyColorMap(ZFlyEmBodyColorOption::EColorOption option);

  QString getAnnotationNameWarningDetail(
      const QMap<uint64_t, QVector<QString> > &nameMap) const;
  QString getAnnotationFinalizedWarningDetail(
      const std::vector<uint64_t> &finalizedBodyArray) const;

protected:
  ZFlyEmBodyMerger m_bodyMerger;
//  ZDvidTarget m_dvidTarget;
  ZDvidReader m_dvidReader;
  ZDvidReader m_routineReader;
  ZDvidReader m_synapseReader;
  ZDvidReader m_todoReader;
  ZDvidReader m_roiReader;
  ZDvidReader m_sparseVolReader;
  ZDvidWriter m_dvidWriter;
  ZFlyEmSupervisor *m_supervisor;

  ZFlyEmBodyMergeProject *m_mergeProject;

  mutable QMutex m_synapseReaderMutex;
  mutable QMutex m_todoReaderMutex;

  //Dvid info
  ZDvidInfo m_grayScaleInfo;
  ZDvidInfo m_labelInfo;
  ZDvidVersionDag m_versionDag;
  ZJsonObject m_infoJson;

  QTimer *m_routineTimer;

  QString m_mergeAutoSavePath;
  bool m_loadingAssignedBookmark; //temporary solution for updating bookmark table
  bool m_routineCheck;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_activeBodyColorMap;
  QMap<ZFlyEmBodyColorOption::EColorOption,
  ZSharedPointer<ZFlyEmBodyColorScheme> > m_colorMapConfig;
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
