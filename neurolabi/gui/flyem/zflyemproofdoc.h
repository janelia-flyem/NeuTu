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
class ZStackArray;


class ZFlyEmProofDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDoc(QObject *parent = 0);
  ~ZFlyEmProofDoc();

  static ZFlyEmProofDoc* Make();

//  void makeAction(ZActionFactory::EAction item);

  const ZDvidVersionDag& getVersionDag() const {
    return m_versionDag;
  }

  void mergeSelected(ZFlyEmSupervisor *supervisor);
  void mergeSelectedWithoutConflict(ZFlyEmSupervisor *supervisor);
  void unmergeSelected();

  virtual void setDvidTarget(const ZDvidTarget &target);

//  virtual void updateTileData();

  const ZDvidTarget& getDvidTarget() const;

  const ZDvidInfo& getGrayScaleInfo() const {
    return m_grayScaleInfo;
  }

  const ZDvidInfo& getLabelInfo() const {
    return m_labelInfo;
  }

  void setGraySliceCenterCut(int width, int height);
  void setSegmentationCenterCut(int width, int height);

  ZDvidTileEnsemble* getDvidTileEnsemble() const;
  ZDvidLabelSlice* getDvidLabelSlice(neutube::EAxis axis) const;
  ZDvidGraySlice* getDvidGraySlice() const;
  ZDvidGraySlice* getDvidGraySlice(neutube::EAxis axis) const;
//  QList<ZDvidLabelSlice*> getDvidLabelSlice() const;
  QList<ZDvidSynapseEnsemble*> getDvidSynapseEnsembleList() const;
  ZDvidSynapseEnsemble* getDvidSynapseEnsemble(neutube::EAxis axis) const;

  const ZDvidSparseStack* getBodyForSplit() const;
  ZDvidSparseStack* getBodyForSplit();
  void clearBodyForSplit();

  const ZSparseStack* getSparseStack() const override;
  ZSparseStack* getSparseStack() override;

  ZStackBlockGrid* getStackGrid();

  //bool hasSparseStack() const;
  bool hasVisibleSparseStack() const override;

  ZFlyEmSupervisor* getSupervisor() const;

  void updateBodyObject();

  void clearData() override;

  /*!
   * \brief Get brief information of the document
   *
   * \return A string that contains information about the document.
   */
  QString getInfo() const;

  /*!
   * \brief Get body ID at a certain location
   *
   * \return The body ID mapped by merge operations.
   */
  uint64_t getBodyId(int x, int y, int z);
  uint64_t getBodyId(const ZIntPoint &pt);

  uint64_t getLabelId(int x, int y, int z) override;
  uint64_t getSupervoxelId(int x, int y, int z) override;

  bool hasBodySelected() const;

  std::set<uint64_t> getSelectedBodySet(neutube::EBodyLabelType labelType) const;
  void setSelectedBody(const std::set<uint64_t> &selected, neutube::EBodyLabelType labelType);
  void setSelectedBody(uint64_t bodyId, neutube::EBodyLabelType labelType);
  void toggleBodySelection(uint64_t bodyId, neutube::EBodyLabelType labelType);
  /*!
   * \brief Deselect bodies
   *
   * Deselect bodies that have the same mapped ID as that of \a bodyId.
   */
  void deselectMappedBody(uint64_t bodyId, neutube::EBodyLabelType labelType);
  void deselectMappedBody(const std::set<uint64_t> &bodySet, neutube::EBodyLabelType labelType);

  void addSelectedBody(
      const std::set<uint64_t> &selected, neutube::EBodyLabelType labelType);

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

//  virtual ZDvidSparseStack* getDvidSparseStack() const;
  ZDvidSparseStack* getDvidSparseStack(
      const ZIntCuboid &roi, flyem::EBodySplitMode mode) const;

  ZDvidSparseStack* getDvidSparseStack() const override;

  ZDvidSparseStack* getCachedBodyForSplit(uint64_t bodyId) const;

  void enhanceTileContrast(bool highContrast);

  void annotateBody(uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation);
//  void useBodyNameMap(bool on);

  void selectBody(uint64_t bodyId);
  template <typename InputIterator>
  void selectBody(const InputIterator &first, const InputIterator &last);

  void deselectBody(uint64_t bodyId);

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

  const ZDvidInfo& getDvidInfo() const;

  void startTimer();

public:
  //The split mode may affect some data loading behaviors, but the result should
  //be the same.
  void runSplit(flyem::EBodySplitMode mode);
  void runFullSplit(flyem::EBodySplitMode mode);
  void runLocalSplit(flyem::EBodySplitMode mode);
  void exitSplit();

  bool isSplitRunning() const;
  void refreshDvidLabelBuffer(unsigned long delay);
//  void setLabelSliceAlpha(int alpha);

  void updateMeshForSelected();

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
  void tryMoveSelectedSynapse(const ZIntPoint &dest, neutube::EAxis axis);
  void annotateSelectedSynapse(ZJsonObject propJson, neutube::EAxis axis);
  void annotateSelectedSynapse(ZFlyEmSynapseAnnotationDialog *dlg,
                               neutube::EAxis axis);

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
  void setTodoItemAction(neutube::EToDoAction action);
  void setTodoItemToNormal();
  void setTodoItemIrrelevant();
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

  static void enableBodySelectionMessage(bool enable = true);
  static bool bodySelectionMessageEnabled();

  void notifyBodySelectionChanged();

  /*!
   * \brief Fetch DVID label slice data and set body selections
   */
  void updateDvidLabelSlice(neutube::EAxis axis);
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
  int removeDvidSparsevol(neutube::EAxis axis);

  void loadSplitFromService();
  void loadSplitTaskFromService();
  void commitSplitFromService();

  const ZSharedPointer<ZFlyEmBodyColorScheme>& getActiveBodyColorMap() const {
    return m_activeBodyColorMap;
  }
  const QMap<ZFlyEmBodyColorOption::EColorOption,
  ZSharedPointer<ZFlyEmBodyColorScheme> > &getColorMapConfig() const {
    return m_colorMapConfig;
  }

  void setActiveBodyColorMap(const ZSharedPointer<ZFlyEmBodyColorScheme>& colorMap) {
    m_activeBodyColorMap = colorMap;
  }
  void setColorMapConfig(const QMap<ZFlyEmBodyColorOption::EColorOption,
                         ZSharedPointer<ZFlyEmBodyColorScheme> > &config) {
    m_colorMapConfig = config;
  }

  void updateBodyColor(ZFlyEmBodyColorOption::EColorOption type);
  void updateBodyColor(ZSharedPointer<ZFlyEmBodyColorScheme> colorMap);

  ZJsonArray getMergeOperation() const;

  void prepareDvidLabelSlice(
      const ZStackViewParam &viewParam,
      int zoom, int centerCutX, int centerCutY, bool usingCenterCut);
  void prepareDvidGraySlice(const ZStackViewParam &viewParam,
      int zoom, int centerCutX, int centerCutY, bool usingCenterCut);

  ZWidgetMessage getAnnotationFailureMessage(uint64_t bodyId) const;

  void diagnose() const override;

public:
  virtual void executeAddTodoCommand(
      int x, int y, int z, bool checked,  neutube::EToDoAction action,
      uint64_t bodyId) override;
  virtual void executeRemoveTodoCommand() override;

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
  void bodyColorUpdated(ZFlyEmProofDoc*);

  void updatingLabelSlice(ZArray *array, const ZStackViewParam &viewParam,
                          int zoom, int centerCutX, int centerCutY,
                          bool usingCenterCut);
  void updatingGraySlice(ZStack *array, const ZStackViewParam &viewParam,
                         int zoom, int centerCutX, int centerCutY,
                         bool usingCenterCut);


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
      int x, int y, int z, neutube::EToDoAction action, uint64_t bodyId = 0);
  void executeAddTodoItemCommand(ZFlyEmToDoItem &item);
  void executeAddToMergeItemCommand(int x, int y, int z, uint64_t bodyId = 0);
  void executeAddToMergeItemCommand(const ZIntPoint &pt, uint64_t bodyId = 0);
  void executeAddToSplitItemCommand(int x, int y, int z, uint64_t bodyId = 0);
  void executeAddToSplitItemCommand(const ZIntPoint &pt, uint64_t bodyId = 0);
  void executeRemoveTodoItemCommand();

  void executeRotateRoiPlaneCommand(int z, double theta);
  void executeScaleRoiPlaneCommand(int z, double sx, double sy);


public slots:
  void syncMergeWithDvid();
  void saveMergeOperation();
  void processExternalBodyMergeUpload();
  void clearBodyMergeStage();
//  void uploadMergeResult();

//  void updateDvidLabelObject();

  void updateDvidLabelObject(EObjectModifiedMode updateMode);
  void updateDvidLabelObjectSliently();
  void updateDvidLabelObject(neutube::EAxis axis);


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
  void updateProtocolColorMap(const ZFlyEmSequencerColorScheme &colorScheme);

  void deleteSelectedSynapse();
  void addSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind,
                  ZDvidSynapseEnsemble::EDataScope scope);
  void verifySelectedSynapse();
  void unverifySelectedSynapse();

  void deselectMappedBodyWithOriginalId(const std::set<uint64_t> &bodySet);
  void checkInSelectedBody(flyem::EBodySplitMode mode);
  void checkInSelectedBodyAdmin();
  void checkOutBody(flyem::EBodySplitMode mode);
  bool checkOutBody(uint64_t bodyId, flyem::EBodySplitMode mode);
//  bool checkInBody(uint64_t bodyId);
  bool checkInBodyWithMessage(
      uint64_t bodyId, flyem::EBodySplitMode mode);


  bool checkBodyWithMessage(
      uint64_t bodyId, bool checkingOut, flyem::EBodySplitMode mode);

  void downloadBookmark(int x, int y, int z);  
  void rewriteSegmentation();

  void syncSynapse(const ZIntPoint &pt);
  void syncMoveSynapse(const ZIntPoint &from, const ZIntPoint &to);

  void runRoutineCheck();
  void scheduleRoutineCheck();

  void updateLabelSlice(ZArray *array, const ZStackViewParam &viewParam,
                        int zoom, int centerCutX, int centerCutY,
                        bool usingCenterCut);
  void updateGraySlice(ZStack *array, const ZStackViewParam &viewParam,
                       int zoom, int centerCutX, int centerCutY,
                       bool usingCenterCut);


protected:
  void autoSave() override;
  void customNotifyObjectModified(ZStackObject::EType type) override;
  void updateDvidTargetForObject();
  void updateDvidInfoForObject();
  virtual void prepareDvidData();
  void addDvidLabelSlice(neutube::EAxis axis);
  void annotateSynapse(
      const ZIntPoint &pt, ZJsonObject propJson, neutube::EAxis axis);
  void setRoutineCheck(bool on);
  uint64_t getBodyIdForSplit() const;
  QColor getSeedColor(int label) const;
  void readInfo();
  void prepareGraySlice(ZDvidGraySlice *slice);
  void prepareLabelSlice();

private:
  void connectSignalSlot();

  void decorateTBar(ZPuncta *puncta);
  void decoratePsd(ZPuncta *puncta);

  void decorateTBar(ZSlicedPuncta *puncta);
  void decoratePsd(ZSlicedPuncta *puncta);
  void loadRoiFunc();

  std::set<uint64_t> getCurrentSelectedBodyId(neutube::EBodyLabelType type) const;

  void init();
  void initTimer();
  void initAutoSave();

  QString getBodySelectionMessage() const;

  /*!
   * \brief Create essential data instance if necessary
   */
  void initData(const ZDvidTarget &target);
  void initData(const std::string &type, const std::string &dataName);

  void initTileData();
  void initGrayscaleSlice();


  void updateMaxLabelZoom();
  void updateMaxGrayscaleZoom();

  ZSharedPointer<ZFlyEmBodyColorScheme> getColorScheme(
      ZFlyEmBodyColorOption::EColorOption type);
  template<typename T>
  ZSharedPointer<T> getColorScheme(ZFlyEmBodyColorOption::EColorOption type);

  void runSplitFunc(flyem::EBodySplitMode mode, flyem::EBodySplitRange range);
  void runSplitFunc(flyem::EBodySplitMode mode);
  void localSplitFunc(flyem::EBodySplitMode mode);
  void runFullSplitFunc(flyem::EBodySplitMode mode);
  ZIntCuboid estimateSplitRoi();
  ZIntCuboid estimateSplitRoi(const ZStackArray &seedMask);
  ZIntCuboid estimateLocalSplitRoi();

  void readBookmarkBodyId(QList<ZFlyEmBookmark*> &bookmarkArray);

  std::string getSynapseName(const ZDvidSynapse &synapse) const;

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
  ZDvidReader m_grayscaleReader;
  ZDvidWriter m_dvidWriter;
  ZFlyEmSupervisor *m_supervisor;

  ZDvidWriter m_workWriter;
  ZDvidReader m_grayscaleWorkReader;

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

  //Data settings
  int m_graySliceCenterCutWidth = 256;
  int m_graySliceCenterCutHeight = 256;

  int m_labelSliceCenterCutWidth = 256;
  int m_labelSliceCenterCutHeight = 256;

  ZSharedPointer<ZFlyEmBodyColorScheme> m_activeBodyColorMap;
  QMap<ZFlyEmBodyColorOption::EColorOption,
  ZSharedPointer<ZFlyEmBodyColorScheme> > m_colorMapConfig;

  QMap<uint64_t, ZFlyEmBodyAnnotation> m_annotationMap; //for Original ID

  mutable ZFlyEmMB6Analyzer m_analyzer;

  mutable ZSharedPointer<ZDvidSparseStack> m_splitSource;

  static const char *THREAD_SPLIT;
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
