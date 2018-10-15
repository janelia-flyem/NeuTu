#ifndef ZFLYEMBODY3DDOC_H
#define ZFLYEMBODY3DDOC_H

#include "zstackdoc.h"

#include <QSet>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QColor>
#include <QList>
#include <QTime>

#ifdef _DEBUG_
#include "zqslog.h"
#endif

#include "neutube_def.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidwriter.h"
#include "zthreadfuturemap.h"
#include "zflyembodyevent.h"
#include "zflyembodymanager.h"
#include "protocols/protocoltaskconfig.h"

class ZFlyEmProofDoc;
class ZFlyEmBodyMerger;
class ZFlyEmBodyComparisonDialog;
class ZFlyEmBody3dDocKeyProcessor;
class ZMesh;
class ZFlyEmBodySplitter;
class ZArbSliceViewParam;
class ZFlyEmToDoItem;
class ZFlyEmBodyAnnotationDialog;
class ZStackDoc3dHelper;
class ZFlyEmBodyEnv;

/*!
 * \brief The class of managing body update in 3D.
 *
 * The class has a work thread to process a queue of body update events. Each
 * instance has an associated visualization mode, which can be flyem::BODY_SPHERE,
 * flyem::BODY_SKELETON and flyem::BODY_MESH. The body updating process is further
 * contolled by the minimal and maximal downsampling levels. For example, only
 * coarse bodies will be shown when both min and max downsampling levels are equal
 * to the coarse body level defined in the current database.
 *
 * Example of creating an instance for showing coarse bodies with sphere surfaces:
 *
 *   ZFlyEmBody3dDoc *doc = new ZFlyEmBody3dDoc;
 *   doc->setDvidTarget(dvidTarget);
 *   doc->setBodyType(flyem::BODY_SPHERE);
 *   doc->useCoarseOnly(); //This must be called after setting the DVID environment
 *
 */
class ZFlyEmBody3dDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBody3dDoc(QObject *parent = 0);
  ~ZFlyEmBody3dDoc();


  void setDataDoc(ZSharedPointer<ZStackDoc> doc);

public:
  class ObjectStatus {
  public:
    explicit ObjectStatus(int timeStamp = 0);
    void setRecycable(bool on);
    void setTimeStamp(int t);
    void setResLevel(int level);

    bool isRecycable() const;
    int getTimeStamp() const;
    int getResLevel() const;

  private:
    void init(int timeStatus);

  private:
    bool m_recycable;
    int m_timeStamp;
    int m_resLevel;
  };

  void setBodyType(flyem::EBodyType type);
  flyem::EBodyType getBodyType() const { return m_bodyType; }

//  QSet<uint64_t> getBodySet() const { return m_bodySet; }
//  QSet<uint64_t> getUnencodedBodySet() const;
  QSet<uint64_t> getNormalBodySet() const;

  /*!
   * \brief Get all normal bodies, including those contributing orphan supervoxels.
   */
  QSet<uint64_t> getInvolvedNormalBodySet() const;

  uint64_t getMappedId(uint64_t bodyId) const;
  bool isAgglo(uint64_t bodyId) const;

  void addBody(const ZFlyEmBodyConfig &config);
  void updateBody(ZFlyEmBodyConfig &config);
  void removeBody(uint64_t bodyId);
  void updateBody(uint64_t bodyId, const QColor &color);
  void updateBody(uint64_t bodyId, const QColor &color, flyem::EBodyType type);

  void addSynapse(uint64_t bodyId);
  void addTodo(uint64_t bodyId);
  void addTodo(int x, int y, int z, bool checked, uint64_t bodyId);
  void addTosplit(int x, int y, int z, bool checked, uint64_t bodyId);
  bool addTodo(const ZFlyEmToDoItem &item, uint64_t bodyId);
  void addTodoSliently(const ZFlyEmToDoItem &item);
  void addTodo(const QList<ZFlyEmToDoItem> &itemList);
  void updateSegmentation();
  void removeTodo(ZFlyEmToDoItem &item, uint64_t bodyId);
  void removeTodo(const QList<ZFlyEmToDoItem> &itemList);
  void removeTodo(int x, int y, int z);
  void removeTodoSliently(const ZFlyEmToDoItem &item);
  ZFlyEmToDoItem makeTodoItem(
      int x, int y, int z, bool checked, uint64_t bodyId);
  ZFlyEmToDoItem readTodoItem(int x, int y, int z) const;

  void loadSplitTask(uint64_t bodyId);
  void removeSplitTask(uint64_t bodyId);
  void enableSplitTaskLoading(bool enable);
  bool splitTaskLoadingEnabled() const;

  void addEvent(ZFlyEmBodyEvent::EAction action, uint64_t bodyId,
                ZFlyEmBodyEvent::TUpdateFlag flag = 0, QMutex *mutex = NULL);
  void addEvent(const ZFlyEmBodyEvent &event, QMutex *mutex = NULL);

  template <typename InputIterator>
  void addBodyChangeEvent(const InputIterator &first, const InputIterator &last);

//  bool hasBody(uint64_t bodyId, bool encoded) const;

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  const ZDvidReader& getMainDvidReader() const;
  const ZDvidReader& getWorkDvidReader() const;

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidInfo& getDvidInfo() const;

  bool isDvidMutable() const;

//  void updateFrame();

  ZFlyEmProofDoc* getDataDocument() const;

  ZDvidGraySlice* getArbGraySlice() const;
//  void updateArbGraySlice(const ZArbSliceViewParam &viewParam);
  void hideArbGrayslice();
  void setArbGraySliceVisible(bool v);

  void printEventQueue() const;

  void dumpAllBody(bool recycable);

  void dumpGarbage(ZStackObject *obj, bool recycable);
  void dumpGarbageUnsync(ZStackObject *obj, bool recycable);

  template<typename InputIterator>
  void dumpGarbage(const InputIterator &first, const InputIterator &last,
                   bool recycable);
  void mergeBodyModel(const ZFlyEmBodyMerger &merger);

  void processEventFunc();
  void cancelEventThread();

  void setTodoItemSelected(ZFlyEmToDoItem *item, bool select);
//  void setTodoVisible(ZFlyEmToDoItem::EToDoAction action, bool visible);

  bool hasTodoItemSelected() const;

  ZFlyEmToDoItem* getOneSelectedTodoItem() const;

  void forceBodyUpdate();
  void compareBody();
  void compareBody(const std::string &uuid);
  void compareBody(const ZFlyEmBodyComparisonDialog *dlg);
  void compareBody(ZDvidReader &diffReader);
  void compareBody(ZDvidReader &diffReader, const ZIntPoint &pt);

  std::vector<std::string> getParentUuidList() const;
  std::vector<std::string> getAncestorUuidList() const;

  void waitForAllEvent();

  bool updating() const;

  void enableNodeSeeding(bool on);
  void enableBodySelectionSync(bool on);

  void enableGarbageLifetimeLimit(bool on);
  bool garbageLifetimeLimitEnabled() const;

  ZMesh *readMesh(
      const ZDvidReader &reader, const ZFlyEmBodyConfig &config,
      int *acturalMeshZoom);
  ZMesh *readMesh(
      const ZDvidReader &reader, uint64_t bodyId, int zoom, int *acturalZoom);
//  ZMesh *readSupervoxelMesh(const ZDvidReader &reader, uint64_t bodyId, int zoom);

#if 0
  // The instances referred to by ZDvidUrl::getMeshesTarsUrl() represent data that
  // uses the body's identifier in multiple ways: for multiple meshes, at different
  // levels in the agglomeration history, and as a key whose associated value is a
  // tar file of meshes.  These distinct identifiers are created by encoding a
  // raw body identifier.

  static uint64_t encode(uint64_t rawId, unsigned int level = 0, bool tar = true);

  static bool encodesTar(uint64_t id);
  static unsigned int encodedLevel(uint64_t id);
#endif

  static uint64_t decode(uint64_t encodedId);

  // TODO: Remove this function (and ZDvidUrl::getMeshesTarsUrl()) when the
  // old alternative to the DVID "tarsupervoxels" data type is completely retired.
  // This function returns true only if the user has set the the NEU3_USE_TARSUPERVOXELS
  // environment variable to "no".
  bool usingOldMeshesTars() const;

  bool fromTar(uint64_t id) const;
//  bool isTarMode() const;

  void setMinDsLevel(int res) {
    m_minDsLevel = res;
  }
  void setMaxDsLevel(int res) {
    m_maxDsLevel = res;
  }

  void useCoarseOnly();
  bool showingCoarseOnly() const;
  bool isCoarseLevel(int level) const;

  int getMaxDsLevel() const;
  int getMinDsLevel() const;

  void makeAction(ZActionFactory::EAction item) override;

  static void SetObjectClass(ZStackObject *obj, uint64_t bodyId);

  void configure(const ProtocolTaskConfig &config);

public:
  void executeAddTodoCommand(
      int x, int y, int z, bool checked,  neutube::EToDoAction action,
      uint64_t bodyId) override;
  void executeAddTodoCommand(
      int x, int y, int z, bool checked, uint64_t bodyId);
  virtual void executeRemoveTodoCommand() override;

  //override to disable the swc commands
  virtual bool executeDeleteSwcNodeCommand() override {
    return false;
  }
  virtual bool executeDeleteUnselectedSwcNodeCommand() override {
    return false;
  }
  virtual bool executeConnectSwcNodeCommand() override {
    return false;
  }
  virtual bool executeConnectSwcNodeCommand(Swc_Tree_Node */*tn*/) override {
    return false;
  }
  virtual bool executeConnectSwcNodeCommand(
      Swc_Tree_Node */*tn1*/, Swc_Tree_Node */*tn2*/) override {
    return false;
  }
  virtual bool executeSmartConnectSwcNodeCommand(
      Swc_Tree_Node */*tn1*/, Swc_Tree_Node */*tn2*/) override {
    return false;
  }
  virtual bool executeSmartConnectSwcNodeCommand() override {
    return false;
  }
  virtual bool executeBreakSwcConnectionCommand() override {
    return false;
  }
  virtual bool executeMergeSwcNodeCommand() override {
    return false;
  }

public:
//  ZDvidSparseStack* loadDvidSparseStack();
  ZDvidSparseStack* loadDvidSparseStack(
      uint64_t bodyId, flyem::EBodyLabelType type);
  ZDvidSparseStack* loadDvidSparseStackForSplit();

  /* Disabled for simplifying the splitting workflow. Might be used again in
   * the future.*/
  uint64_t protectBodyForSplit();

  bool protectBody(uint64_t bodyId);
  void releaseBody(uint64_t bodyId, flyem::EBodyLabelType labelType);
  bool isBodyProtected(uint64_t bodyId) const;

  void activateSplit(uint64_t bodyId);
  void activateSplit(uint64_t bodyId, flyem::EBodyLabelType type, bool fromTar);
  void deactivateSplit();
  bool isSplitActivated() const;
  bool isSplitFinished() const;

  ZMesh* getMeshForSplit() const;
  ZMesh* readSupervoxelMesh(const ZDvidReader &reader, uint64_t svId) const;

  void hideNoSplitMesh();
  void showAllMesh();

  uint64_t getSelectedSingleNormalBodyId() const;
  void startBodyAnnotation(ZFlyEmBodyAnnotationDialog *dlg);


  void registerBody(uint64_t bodyId);
  void deregisterBody(uint64_t bodyId);

  /*!
   * \brief Specify how a body should be updated.
   *
   * Initial update of the body in \a config will follow the specification in
   * \a config.
   */
  void addBodyConfig(const ZFlyEmBodyConfig &config);

  bool isSupervoxel(uint64_t bodyId);

  void diagnose() const override;

  ZStackDoc3dHelper* getHelper() const;

public slots:
  void showSynapse(bool on);// { m_showingSynapse = on; }
  bool showingSynapse() const;
  void addSynapse(bool on);
  void showTodo(bool on);
  bool showingTodo() const;
  void addTodo(bool on);
  void updateTodo(uint64_t bodyId);
  void updateSynapse(uint64_t bodyId);
  void setUnrecycable(const QSet<uint64_t> &bodySet);
  void setNormalTodoVisible(bool visible);
  void setSelectedTodoItemChecked(bool on);
  void checkSelectedTodoItem();
  void uncheckSelectedTodoItem();
  void setTodoItemAction(neutube::EToDoAction action);

  void showMoreDetail(uint64_t bodyId, const ZIntCuboid &range);

  void recycleObject(ZStackObject *obj) override;
  void killObject(ZStackObject *obj) override;

  void setSeedType(int type);

  void setBodyModelSelected(const QSet<uint64_t> &bodySet);
  void setBodyModelSelected(const QSet<uint64_t> &select,
                            const QSet<uint64_t> &deselect);

  void saveSplitTask();
  void deleteSplitSeed();
  void deleteSelectedSplitSeed();

  void processObjectModifiedFromDataDoc(const ZStackObjectInfoSet &objInfo);

  void cacheObject(ZStackObject *obj);

  void processBodySelectionChange();

  void runLocalSplit();
  void runSplit();
  void runFullSplit();
  void commitSplitResult();

  void waitForSplitToBeDone();
  void activateSplitForSelected();

  void clearGarbage(bool force = false);

  void startBodyAnnotation();
  void showMeshForSplitOnly(bool on);
//  void updateCurrentTask(const QString &taskType);

signals:
  void bodyRemoved(uint64_t bodyId);
  void bodyRecycled(uint64_t bodyId);
  void interactionStateChanged();

  //Signals for triggering external body control
  void addingBody(uint64_t bodyId);
  void removingBody(uint64_t bodyId);
  void splitCommitted();

protected:
  void autoSave() override {}
  void makeKeyProcessor() override;

private:
  ZStackObject* retriveBodyObject(
      uint64_t bodyId, int zoom,
      flyem::EBodyType bodyType, ZStackObject::EType objType);
  ZStackObject* retriveBodyObject(uint64_t bodyId, int zoom);
  ZSwcTree* retrieveBodyModel(uint64_t bodyId, int zoom, flyem::EBodyType bodyType);
  ZSwcTree* getBodyModel(uint64_t bodyId, int zoom, flyem::EBodyType bodyType);
  ZMesh* getBodyMesh(uint64_t bodyId, int zoom);
  ZMesh* retrieveBodyMesh(uint64_t bodyId, int zoom);

  ZMesh *readMesh(const ZFlyEmBodyConfig &config, int *actualMeshZoom);

//  ZSwcTree* makeBodyModel(uint64_t bodyId, int zoom);
  ZSwcTree* makeBodyModel(uint64_t bodyId, int zoom, flyem::EBodyType bodyType);

  std::vector<ZMesh*> getTarCachedMeshes(uint64_t bodyId);

  std::vector<ZMesh*> getCachedMeshes(uint64_t bodyId, int zoom);
  std::vector<ZMesh *> makeBodyMeshModels(ZFlyEmBodyConfig &config);
  std::vector<ZMesh*> makeTarMeshModels(uint64_t bodyId, int t);
  std::vector<ZMesh*> makeTarMeshModels(
      const ZDvidReader &reader, uint64_t bodyId, int t);

  std::vector<ZSwcTree*> makeDiffBodyModel(
      uint64_t bodyId1, ZDvidReader &diffReader, int zoom,
      flyem::EBodyType bodyType);

  std::vector<ZSwcTree*> makeDiffBodyModel(
      uint64_t bodyId1, uint64_t bodyId2, ZDvidReader &diffReader, int zoom,
      flyem::EBodyType bodyType);

  std::vector<ZSwcTree*> makeDiffBodyModel(
      const ZIntPoint &pt, ZDvidReader &diffReader,
      int zoom, flyem::EBodyType bodyType);
  QColor getBodyColor(uint64_t bodyId);

  void updateDvidInfo();

  void addBodyFunc(ZFlyEmBodyConfig &config);
  void addBodyMeshFunc(ZFlyEmBodyConfig &config);
//  void addBodyFunc(uint64_t bodyId, const QColor &color, int resLevel);
//  void addBodyMeshFunc(uint64_t bodyId, const QColor &color, int resLevel);

  void removeBodyFunc(uint64_t bodyId, bool removingAnnotation);
  void updateBodyFunc(uint64_t bodyId, ZStackObject *bodyObject);
  void updateMeshFunc(ZFlyEmBodyConfig &config, const std::vector<ZMesh*> meshes);
//  void updateBodyMeshFunc(uint64_t bodyId, ZMesh *mesh);

  void connectSignalSlot();
  void updateBodyFunc();

  void processBodySetBuffer();

  QMap<uint64_t, ZFlyEmBodyEvent> makeEventMap(bool synced, ZFlyEmBodyManager *bm);
  QMap<uint64_t, ZFlyEmBodyEvent> makeEventMapUnsync(ZFlyEmBodyManager *bm);

  bool synapseLoaded(uint64_t bodyId) const;
  void addSynapse(
      std::vector<ZPunctum*> &puncta,
      uint64_t bodyId, const std::string &source, double radius,
      const QColor &color);

  template<typename T>
  T* recoverFromGarbage(const std::string &source);

  ZSwcTree *getBodyQuickly(uint64_t bodyId);
  /*
  ZFlyEmBodyEvent makeHighResBodyEvent(const ZFlyEmBodyConfig &config);
  ZFlyEmBodyEvent makeHighResBodyEvent(
      const ZFlyEmBodyConfig &config, const ZIntCuboid &range);
      */

  ZDvidReader& getBodyReader();
//  const ZDvidReader& getBodyReader() const;
  void updateBodyModelSelection();

  ZStackObject::EType getBodyObjectType() const;

  flyem::EBodyLabelType getBodyLabelType(uint64_t bodyId) const;

  static bool IsOverSize(const ZStackObject *obj);

  int getCoarseBodyZoom() const;


  /*!
   * \brief A safe way to get the only body in the document.
   */
  uint64_t getSingleBody() const;

  const ZFlyEmBodyManager& getBodyManager() const;
  ZFlyEmBodyManager& getBodyManager();

//  bool allowingSplit(uint64_t bodyId) const;

signals:
  void todoVisibleChanged();
  void bodyMeshLoaded(int);
  void bodyMeshesAdded(int);

  void meshArchiveLoadingStarted();
  void meshArchiveLoadingProgress(float fraction);
  void meshArchiveLoadingEnded();

private slots:
//  void updateBody();
  void processEvent();
  void processEvent(const ZFlyEmBodyEvent &event);

private:
  void processEventFunc(const ZFlyEmBodyEvent &event);
  ZSwcTree* recoverFullBodyFromGarbage(
      uint64_t bodyId, int resLevel);
  ZMesh* recoverMeshFromGarbage(uint64_t bodyId, int resLevel);

  void removeDiffBody();

  ZStackObject* takeObjectFromCache(
      ZStackObject::EType type, const std::string &source);

  void notifyBodyUpdate(uint64_t bodyId, int resLevel);
  void notifyBodyUpdated(uint64_t bodyId, int resLevel);

  void initArbGraySlice();
  bool toBeRemoved(uint64_t bodyId) const;

  bool isSupervoxel(const ZStackObject *obj) const;
  uint64_t getBodyId(const ZStackObject *obj) const;

  flyem::EBodyLabelType getLabelType(uint64_t bodyId) const;

  void loadSynapseFresh(uint64_t bodyId);
  void loadTodoFresh(uint64_t bodyId);

  ZFlyEmBodyAnnotationDialog* getBodyAnnotationDlg();

  void constructBodyMesh(ZMesh *mesh, uint64_t bodyId, bool fromTar);
  void retrieveSegmentationMesh(QMap<std::string, ZMesh*> *meshMap);

private:
  ZFlyEmBodyManager m_bodyManager;
//  QSet<uint64_t> m_bodySet; //Normal body set. All the IDs are unencoded.
//  std::map<uint64_t, std::set<uint64_t>> m_tarIdToMeshIds;

  mutable QMutex m_BodySetMutex;

  flyem::EBodyType m_bodyType = flyem::EBodyType::SPHERE;
  QSet<uint64_t> m_selectedBodySet;
  QSet<uint64_t> m_protectedBodySet;
  mutable QMutex m_protectedBodySetMutex;

  bool m_quitting = false;
  bool m_showingSynapse = true;
  bool m_showingTodo = true;
  bool m_nodeSeeding = false;
  bool m_syncyingBodySelection = false;

  int m_minDsLevel = 0;
  int m_maxDsLevel = 5; //Start resolution level; bigger value means lower resolution

//  QSet<uint64_t> m_bodySetBuffer;
//  bool m_isBodySetBufferProcessed;

  ZDvidTarget m_dvidTarget;
  ZDvidReader m_workDvidReader;
  ZDvidWriter m_mainDvidWriter;
  ZDvidReader m_bodyReader;

  ZDvidInfo m_dvidInfo;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;
  QTimer *m_garbageTimer;
  QTime m_objectTime;

  ZSharedPointer<ZStackDoc> m_dataDoc;
  ZSharedPointer<ZStackDoc3dHelper> m_helper;

  ProtocolTaskConfig m_taskConfig;

//  QList<ZStackObject*> m_garbageList;
  QMap<ZStackObject*, ObjectStatus> m_garbageMap;
  ZStackObjectGroup m_objCache;
  int m_objCacheCapacity;
  QMap<uint64_t, int> m_bodyUpdateMap;

  ZFlyEmBodySplitter *m_splitter;

  ZFlyEmBodyAnnotationDialog *m_annotationDlg = nullptr;
//  QSet<uint64_t> m_unrecycableSet;

  bool m_garbageJustDumped = false;

  QQueue<ZFlyEmBodyEvent> m_eventQueue;

  mutable QMutex m_eventQueueMutex;
  QMutex m_garbageMutex;

  bool m_limitGarbageLifetime = true;
  bool m_splitTaskLoadingEnabled = true;

  const static int OBJECT_GARBAGE_LIFE;
  const static int OBJECT_ACTIVE_LIFE;
//  const static int MAX_RES_LEVEL;

  const static char *THREAD_SPLIT_KEY;
};

template <typename InputIterator>
void ZFlyEmBody3dDoc::addBodyChangeEvent(
    const InputIterator &first, const InputIterator &last)
{
  std::cout << "Locking mutex ..." << std::endl;
  QMutexLocker locker(&m_eventQueueMutex);

  //Update event map and body set; the current event queue is cleared
  QMap<uint64_t, ZFlyEmBodyEvent> actionMap = makeEventMap(false, NULL);
  QSet<uint64_t> oldBodySet = getNormalBodySet();

//  m_eventQueue.clear();

  QSet<uint64_t> newBodySet;
  QSet<uint64_t> tarSet;
  QSet<uint64_t> supervoxelSet;
//  QMap<uint64_t, uint64_t> bodyEncodeMap;

#ifdef _DEBUG_
  std::string bodyStr;
  for (InputIterator iter = first; iter != last; ++iter) {
    bodyStr += std::to_string(*iter) + " ";
  }
  LDEBUG() << "Selection recieved:" << bodyStr;
#endif

  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    if (ZFlyEmBodyManager::encodesTar(bodyId)) {
      tarSet.insert(bodyId);
    } else if (ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
      supervoxelSet.insert(bodyId);
    } else {
      newBodySet.insert(decode(bodyId));
    }
//    bodyEncodeMap[decode(bodyId)] = bodyId;
  }

  QSet<uint64_t> addedBodySet = newBodySet - oldBodySet;
  QSet<uint64_t> removedBodySet = oldBodySet - newBodySet;
  QSet<uint64_t> commonBodySet = newBodySet.intersect(oldBodySet);

  //Remove bodies not in the current set
  foreach (uint64_t bodyId, removedBodySet) {
    addEvent(ZFlyEmBodyEvent::ACTION_REMOVE, bodyId, 0, NULL);
  }

  //Keep the event of common bodies if it's not removing the body
  foreach (uint64_t bodyId, commonBodySet) {
    if (actionMap.contains(bodyId)) {
      const ZFlyEmBodyEvent &bodyEvent = actionMap[bodyId];
      if (bodyEvent.getAction() != ZFlyEmBodyEvent::ACTION_REMOVE) {
        addEvent(bodyEvent);
      }
    }
  }

  /*
  for (QMap<uint64_t, ZFlyEmBodyEvent>::iterator
       iter = actionMap.begin(); iter != actionMap.end(); ++iter) {
    uint64_t bodyId = iter.key();
    if (newBodySet.contains(bodyId)) {
      if (iter.value().getAction() != ZFlyEmBodyEvent::ACTION_REMOVE) {
        //In the new body set had the bodyID and not remove, add event
        addEvent(iter.value());
      }
    }
  }
  */

  //Add new bodies
  foreach (uint64_t bodyId, addedBodySet) {
    if (!getBodyManager().isSupervoxel(bodyId)) {
      addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
    }
  }

  //Process tar set
  foreach (uint64_t bodyId, tarSet) {
    if (ZFlyEmBodyManager::encodedLevel(bodyId) == 0) { //supervoxel tar
      if (!getBodyManager().hasMapping(bodyId)) {
        addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
      }
    } else { //normal body
      if (!getBodyManager().contains(bodyId)) {
        addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
      }
    }
  }

  /*
  foreach (uint64_t bodyId, tarSet) {
    if (!getBodyManager().contains(bodyId)) {
      addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
    }
  }
  */

  //Process supervoxel set
  QSet<uint64_t> addedSupervoxelSet =
      getBodyManager().getSupervoxelToAdd(supervoxelSet, true);
  foreach (uint64_t bodyId, addedSupervoxelSet) {
    addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
  }

  QSet<uint64_t> removedSupervoxelSet =
      getBodyManager().getSupervoxelToRemove(supervoxelSet, true);
  foreach (uint64_t bodyId, removedSupervoxelSet) {
    addEvent(ZFlyEmBodyEvent::ACTION_REMOVE, bodyId, 0, NULL);
  }

  QSet<uint64_t> commonSupervoxelSet = supervoxelSet - addedSupervoxelSet;
  foreach (uint64_t bodyId, commonSupervoxelSet) {
    if (actionMap.contains(bodyId)) {
      const ZFlyEmBodyEvent &bodyEvent = actionMap[bodyId];
      if (bodyEvent.getAction() != ZFlyEmBodyEvent::ACTION_REMOVE) {
        addEvent(bodyEvent);
      }
    }
  }



#if 0
  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    if (!actionMap.contains(bodyId)) { //If the action map has no such body id
      addEvent(ZFlyEmBodyEvent::ACTION_ADD, bodyId, 0, NULL);
    }
  }
#endif
}

#endif // ZFLYEMBODY3DDOC_H
