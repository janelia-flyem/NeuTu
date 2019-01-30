#ifndef ZFLYEMPROOFMVC_H
#define ZFLYEMPROOFMVC_H

#include <vector>
#include <QString>
#include <QMetaType>
#include <QSharedPointer>
#include <QMap>

#include "common/neutube_def.h"
#include "zstackmvc.h"
#include "flyem/zflyembodysplitproject.h"
#include "flyem/zflyembodymergeproject.h"
#include "zthreadfuturemap.h"
#include "flyem/zflyembookmark.h"
#include "zwindowfactory.h"
#include "common/neutube_def.h"
#include "zactionfactory.h"

class QWidget;
class ZFlyEmProofDoc;
class ZDvidTileEnsemble;
class ZDvidTarget;
class ZDvidTargetProviderDialog;
class ZFlyEmProofPresenter;
class ZFlyEmSupervisor;
class ZPaintLabelWidget;
class FlyEmBodyInfoDialog;
class ProtocolSwitcher;
class ZFlyEmSplitCommitDialog;
class ZFlyEmOrthoWindow;
class ZFlyEmDataFrame;
class FlyEmTodoDialog;
class ZClickableColorLabel;
class ZColorLabel;
class ZFlyEmSynapseDataFetcher;
class ZFlyEmSynapseDataUpdater;
class ZDvidPatchDataFetcher;
class ZDvidPatchDataUpdater;
class ZFlyEmRoiToolDialog;
class QSortFilterProxyModel;
class ZFlyEmBookmarkView;
class ZFlyEmSplitUploadOptionDialog;
class ZFlyEmBodyChopDialog;
class ZInfoDialog;
class ZRandomGenerator;
class ZFlyEmSkeletonUpdateDialog;
class ZFlyEmBody3dDoc;
class ZDvidLabelSlice;
class ZFlyEmGrayscaleDialog;
class FlyEmBodyIdDialog;
class ZFlyEmMergeUploadDialog;
class ZFlyEmProofSettingDialog;
class ZROIWidget;
class ZFlyEmBodyAnnotationDialog;
class NeuPrintQueryDialog;
class ZActionLibrary;
class NeuPrintReader;
class NeuprintSetupDialog;
class ZContrastProtocalDialog;

/*!
 * \brief The MVC class for flyem proofreading
 */
class ZFlyEmProofMvc : public ZStackMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofMvc(QWidget *parent = 0);
  ~ZFlyEmProofMvc();

  static ZFlyEmProofMvc* Make(
      QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc,
      neutube::EAxis axis = neutube::EAxis::Z, ERole role = ROLE_WIDGET);
  static ZFlyEmProofMvc* Make(
      const ZDvidTarget &target, ERole role = ROLE_WIDGET);
  static ZFlyEmProofMvc* Make(ERole role = ROLE_WIDGET);

  ZFlyEmProofDoc* getCompleteDocument() const;
  ZFlyEmProofPresenter* getCompletePresenter() const;

  template <typename T>
  void connectControlPanel(T *panel);

  template <typename T>
  void connectSplitControlPanel(T *panel);

  ZDvidTileEnsemble* getDvidTileEnsemble();

  const ZDvidInfo& getDvidInfo() const;
  const ZDvidInfo& getGrayScaleInfo() const;

  virtual void setDvidTarget(const ZDvidTarget &target);
  void setDvidTargetFromDialog();

  void clear();

  void exitCurrentDoc();

  void enableSplit(flyem::EBodySplitMode mode);
  void disableSplit();

  void processViewChangeCustom(const ZStackViewParam &viewParam);

  ZFlyEmSupervisor* getSupervisor() const;

//  bool checkInBody(uint64_t bodyId);
  bool checkOutBody(uint64_t bodyId, flyem::EBodySplitMode mode);

  virtual ZDvidTarget getDvidTarget() const;

  void setDvidDialog(ZDvidTargetProviderDialog *dlg);
  ZDvidTargetProviderDialog* getDvidDialog() const;

  uint64_t getBodyIdForSplit() const;
  void setBodyIdForSplit(uint64_t id);

  void updateContrast();

  void diagnose();
  void profile();
  void startTestTask(const std::string &taskKey);
  void startTestTask(const ZJsonObject &config);
  void showSetting();
  void setExiting(bool exiting) {
    m_quitting = exiting;
  }

  Z3DWindow* makeExternalSkeletonWindow(neutube3d::EWindowType windowType);
  Z3DWindow* makeExternalMeshWindow(neutube3d::EWindowType windowType);
  Z3DWindow* makeNeu3Window();


  void updateRoiWidget(ZROIWidget *widget, Z3DWindow *win) const;

  static void showAnnotations(bool show);
  static bool showingAnnotations();

  uint64_t getRandomBodyId(ZRandomGenerator &rand, ZIntPoint *pos = NULL);

  FlyEmBodyInfoDialog *getBodyInfoDlg() const {
    return m_bodyInfoDlg;
  }

  bool is3DEnabled() const {
    return m_3dEnabled;
  }

  void disable3D() {
    m_3dEnabled = false;
  }

  bool hasSequencer() const;

  void disableSequencer();

  void notifyStateUpdate();

  void configure();

//  bool hasNeuPrint() const;
  neutube::EServerStatus getNeuPrintStatus() const;

public: //bookmark functions
    ZFlyEmBookmarkListModel* getAssignedBookmarkModel(
       flyem::EProofreadingMode mode) const {
      return m_assignedBookmarkModel[mode];
    }

    ZFlyEmBookmarkListModel* getUserBookmarkModel(
       flyem::EProofreadingMode mode) const {
      return m_userBookmarkModel[mode];
    }

    ZFlyEmBookmarkListModel* getUserBookmarkModel() const;
    ZFlyEmBookmarkListModel* getAssignedBookmarkModel() const;

    void registerBookmarkView(ZFlyEmBookmarkView *view);

public: //Export functions
  void exportGrayscale();
  void exportGrayscale(const ZIntCuboid &box, int dsIntv, const QString &fileName);
  void exportBodyStack();

  //exploratory code
  void exportNeuronScreenshot(
      const std::vector<uint64_t> &bodyIdArray, int width, int height,
      const QString &outDir);

  void exportNeuronMeshScreenshot(
      const std::vector<uint64_t> &bodyIdArray, int width, int height,
      const QString &outDir);

signals:
  void launchingSplit(const QString &message);
  void launchingSplit(uint64_t bodyId);
  void exitingSplit();
  void messageGenerated(const QString &message, bool appending = true);
  void errorGenerated(const QString &message, bool appending = true);
  void messageGenerated(const ZWidgetMessage &message);
  void splitBodyLoaded(uint64_t bodyId, flyem::EBodySplitMode mode);
  void bookmarkUpdated(ZFlyEmBodyMergeProject *m_project);
  void bookmarkUpdated(ZFlyEmBodySplitProject *m_project);
  void bookmarkDeleted(ZFlyEmBodyMergeProject *m_project);
  void bookmarkDeleted(ZFlyEmBodySplitProject *m_project);
  void dvidTargetChanged(ZDvidTarget);
  void userBookmarkUpdated(ZStackDoc *doc);
  void nameColorMapReady(bool ready);
  void bodyMergeEdited();
  void updatingLatency(int);
//  void highlightModeEnabled(bool);
  void highlightModeChanged();
  void roiLoaded();
  void locating2DViewTriggered(int x, int y, int z, int width);
  void dvidReady();
  void stateUpdated(ZFlyEmProofMvc *mvc);

public slots:
  void mergeSelected();
  void unmergeSelected();
  void undo();
  void redo();

  void setSegmentationVisible(bool visible);
  void setDvidTarget();
  void launchSplit(uint64_t bodyId, flyem::EBodySplitMode mode);
  void processMessageSlot(const QString &message);
  void processMessage(const ZWidgetMessage &msg);
  void notifySplitTriggered();
  void annotateBody();
  void setExpertBodyStatus();
  void showBodyConnection();
  void showBodyProfile();
  void annotateSynapse();
  void annotateTodo();
  void checkInSelectedBody(flyem::EBodySplitMode mode);
  void checkInSelectedBodyAdmin();
  void checkOutBody(flyem::EBodySplitMode mode);
//  bool checkInBody(uint64_t bodyId);
  bool checkInBodyWithMessage(
      uint64_t bodyId, flyem::EBodySplitMode mode = flyem::EBodySplitMode::NONE);
  bool checkBodyWithMessage(
      uint64_t bodyId, bool checkingOut,
      flyem::EBodySplitMode mode = flyem::EBodySplitMode::NONE);
  void exitSplit();
  void switchSplitBody(uint64_t bodyId);
  void showBodyQuickView();
  void showSplitQuickView();
  void presentBodySplit(uint64_t bodyId, flyem::EBodySplitMode mode);
  void updateBodySelection();
  void saveSeed();
  void saveMergeOperation();
  void commitMerge();
  void commitCurrentSplit();
  bool locateBody(uint64_t bodyId, bool appending);
  bool locateBody(uint64_t bodyId);
//  void locateBody(QList<uint64_t> bodyIdList); //obsolete function
  void addLocateBody(uint64_t bodyId);
  void selectBody(uint64_t bodyId, bool postponeWindowUpdates = false);
  void deselectBody(uint64_t bodyId, bool postponeWindowUpdates = false);
  void selectBodyInRoi(bool appending = true);
  void selectBody(QList<uint64_t> bodyIdList);
  void notifyBodyMergeEdited();
  void updateProtocolRangeGlyph(
      const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);

  void showBody3d();
  void showSplit3d();
  void showCoarseBody3d();
  void showFineBody3d();
  void showSkeletonWindow();
  void showMeshWindow();
  void showCoarseMeshWindow();
  void showExternalNeuronWindow();
  void showObjectWindow();
  void showRoi3dWindow();
  void showQueryTable();
  void showOrthoWindow(double x, double y, double z);
  void showBigOrthoWindow(double x, double y, double z);

  void closeSkeletonWindow();

  void setDvidLabelSliceSize(int width, int height);
  void showFullSegmentation();

  void tuneGrayscaleContrast();
  void enhanceTileContrast(bool state);
  void smoothDisplay(bool state);

  void goToBody();
  void goToBodyBottom();
  void goToBodyTop();
  void selectBody();
  void processLabelSliceSelectionChange();
  void decomposeBody();
  void cropBody();
  void chopBodyZ();
  void chopBody();

  void loadBookmark(const QString &filePath);
  void addSelectionAt(int x, int y, int z);
  void xorSelectionAt(int x, int y, int z);
  void deselectAllBody(bool asking);
  void selectSeed();
  void setMainSeed();
  void selectAllSeed();
  void recoverSeed();
  void exportSeed();
  void importSeed();
  void runSplit();
  void runFullSplit();
  void runLocalSplit();
  void saveSplitTask();
  void saveSplitTask(uint64_t bodyId);
  void loadSplitResult();
  void loadSplitTask();
  void uploadSplitResult();
  void reportBodyCorruption();

  void loadSynapse();
  void goToTBar();
  void showSynapseAnnotation(bool visible);
  void showBookmark(bool visible);
  void showSegmentation(bool visible);
  void showRoiMask(bool visible);
  void showData(bool visible);
  void setHighContrast(bool on);
  void toggleSegmentation();
  void showTodo(bool visible);
  void toggleData();

  void loadBookmark();
  void openSequencer();
  void openProtocol();
  void openTodo();
  void openRoiTool();
  void openNeuPrint();

  void goToNearestRoi();
  void loadRoiProject();
  void closeRoiProject();
  void updateRoiGlyph();

  void estimateRoi();
//  void expandPlaneRoi();
//  void shrinkPlaneRoi();
  void movePlaneRoi(double dx, double dy);
  void rotatePlaneRoi(double theta);
  void scalePlaneRoi(double sx, double sy);

  void checkSelectedBookmark(bool checking);
  void checkSelectedBookmark();
  void uncheckSelectedBookmark();
  void recordCheckedBookmark(const QString &key, bool checking);
  void recordBookmark(ZFlyEmBookmark *bookmark);
  void processSelectionChange(const ZStackObjectSelector &selector);

  void annotateBookmark(ZFlyEmBookmark *bookmark);

  void updateBookmarkTable();
  void updateUserBookmarkTable();
  void updateAssignedBookmarkTable();
  void appendAssignedBookmarkTable(const QList<ZFlyEmBookmark*> &bookmarkList);
  void appendUserBookmarkTable(const QList<ZFlyEmBookmark*> &bookmarkList);
//  void updateAssignedBookmarkTable();
  void sortUserBookmarkTable();
  void sortAssignedBookmarkTable();

  void processCheckedUserBookmark(ZFlyEmBookmark *bookmark);

  void changeColorMap(const QString &option);
  void changeColorMap(QAction *action);

  void removeLocalBookmark(ZFlyEmBookmark *bookmark);
  void addLocalBookmark(ZFlyEmBookmark *bookmark);

  void removeBookmark(ZFlyEmBookmark *bookmark);
  void removeBookmark(const QList<ZFlyEmBookmark*> &bookmarkList);

  void highlightSelectedObject(ZDvidLabelSlice *labelSlice, bool hl);
  void highlightSelectedObject(bool hl);

//  void syncMergeWithDvid();

  void retrieveRois();
  void updateLatencyWidget(int t);

  void suppressObjectVisible();
  void recoverObjectVisible();

  void updateRoiWidget();

  void setLabelAlpha(int alpha);
//  void toggleEdgeMode(bool edgeOn);

  void testBodyMerge();
  void testBodyVis();
  void testBodySplit();

  void endTestTask();

protected slots:
  void detachCoarseBodyWindow();
  void detachBodyWindow();
  void detachSplitWindow();
  void detachSkeletonWindow();
  void detachMeshWindow();
  void detachCoarseMeshWindow();
  void detachObjectWindow();
  void detachRoiWindow();
  void detachExternalNeuronWindow();
  void detachOrthoWindow();
  void detachQueryWindow();
//  void closeBodyWindow(int index);
  void closeOrthoWindow();
  void close3DWindow(Z3DWindow *window);
  void closeBodyWindow(Z3DWindow *window);
  void closeAllBodyWindow();
  void closeAllAssociatedWindow();
  void updateCoarseBodyWindow();
  void updateCoarseBodyWindowDeep();
  void updateBodyWindow();
  void updateBodyWindowDeep();
  void updateSkeletonWindow();
  void updateMeshWindow();
  void updateMeshWindowDeep();
  void updateCoarseMeshWindow();
  void updateCoarseMeshWindowDeep();
  void cropCoarseBody3D();
  void showBodyGrayscale();
  void updateSplitBody();
  void updateBodyMerge();
  void updateCoarseBodyWindowColor();
  void prepareBodyMap(const ZJsonValue &bodyInfoObj);
  void clearBodyMergeStage();
//  void queryBodyByRoi();
//  void findSimilarNeuron();
//  void queryBodyByName();
//  void queryBodyByStatus();
//  void queryAllNamedBody();
  void exportSelectedBody();
  void exportSelectedBodyLevel();
  void exportSelectedBodyStack();
  void skeletonizeSelectedBody();
  void skeletonizeSynapseTopBody();
  void skeletonizeBodyList();
  void updateMeshForSelected();
  void processSynapseVerification(int x, int y, int z, bool verified);
  void processSynapseMoving(const ZIntPoint &from, const ZIntPoint &to);
  void showInfoDialog();

  void syncBodySelectionFromOrthoWindow();
  void syncBodySelectionToOrthoWindow();
//  void notifyBookmarkDeleted();

protected:
  void customInit();
  void createPresenter();
  virtual void dropEvent(QDropEvent *event);
  void enableSynapseFetcher();
  virtual void prepareStressTestEnv(ZStressTestOptionDialog *optionDlg);

private slots:
//  void updateDvidLabelObject();
  void roiToggled(bool on);
  void setProtocolRangeVisible(bool on);
  void showSupervoxelList();
  void goToPosition();
  void enableNameColorMap(bool on);
  void toggleBodyColorMap();
  void updateTmpContrast();
  void resetContrast();
  void saveTmpContrast();

private:
  void init();
  void initBodyWindow();

  void updateContrast(const ZJsonObject &protocolJson, bool hc);

  void launchSplitFunc(uint64_t bodyId, flyem::EBodySplitMode mode);
  uint64_t getMappedBodyId(uint64_t bodyId);
  std::set<uint64_t> getCurrentSelectedBodyId(neutube::EBodyLabelType type) const;
  void runSplitFunc();
  void runFullSplitFunc();
  void runLocalSplitFunc();

  void mergeSelectedWithoutConflict();
//  void notifyBookmarkUpdated();

//  void syncDvidBookmark();
  void loadBookmarkFunc(const QString &filePath);
  void loadROIFunc();
  void loadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::string &key, const std::string &source);
  void loadRoi(
      const ZDvidReader &reader, const std::string &roiName,
      const std::vector<std::string> &keyList, const std::string &source);

  void loadRoiFromRoiData(const ZDvidReader &reader);
  void loadRoiFromRefData(const ZDvidReader &reader, const std::string &roiName);
  void loadRoiMesh(ZMesh *mesh, const std::string &roiName);

  void makeCoarseBodyWindow();
  void makeBodyWindow();
  void makeSkeletonWindow();
  void makeSplitWindow();
  void makeExternalNeuronWindow();
  void makeMeshWindow();
  void makeCoarseMeshWindow();
  void makeOrthoWindow();
  void makeBigOrthoWindow();
  void makeOrthoWindow(int width, int height, int depth);

  void log3DWindowEvent(const std::string &windowName, const std::string &action);

  void showWindow(Z3DWindow *&window, std::function<void(void)> _makeWindow,
                  int tab, const QString &title);

  void makeMeshWindow(bool coarse);

  void updateBodyWindow(Z3DWindow *window);
  void updateBodyWindowDeep(Z3DWindow *window);

  ZWindowFactory makeExternalWindowFactory(neutube3d::EWindowType windowType);

  ZFlyEmBody3dDoc *makeBodyDoc(flyem::EBodyType bodyType);

  void prepareBodyWindowSignalSlot(Z3DWindow *window, ZFlyEmBody3dDoc *doc);

  void mergeCoarseBodyWindow();

//  void updateCoarseBodyWindow(bool showingWindow, bool resettingCamera,
//                              bool isDeep);
  void updateBodyWindowForSplit();

  void setWindowSignalSlot(Z3DWindow *window);
  void updateBodyWindowPlane(
      Z3DWindow *window, const ZStackViewParam &viewParam);
  ZDvidLabelSlice* getDvidLabelSlice() const;

  void clearAssignedBookmarkModel();
  void clearUserBookmarkModel();
  void exitHighlightMode();
  ZDvidSparseStack* getCachedBodyForSplit(uint64_t bodyId);
  ZDvidSparseStack* updateBodyForSplit(uint64_t bodyId, ZDvidReader &reader);

  void prepareTile(ZDvidTileEnsemble *te);
  void applySettings();
  void connectSignalSlot();
//  void prepareBookmarkModel(ZFlyEmBookmarkListModel *model,
//                            QSortFilterProxyModel *proxy);

  void startMergeProfile();
  void startMergeProfile(const uint64_t bodyId, int msec);
  void endMergeProfile();

  FlyEmBodyInfoDialog* getBodyQueryDlg();
  FlyEmBodyInfoDialog* getNeuPrintBodyDlg();
  ZFlyEmBodyAnnotationDialog* getBodyAnnotationDlg();
//  NeuPrintQueryDialog* getNeuPrintRoiQueryDlg();
  NeuprintSetupDialog* getNeuPrintSetupDlg();
  ZContrastProtocalDialog* getContrastDlg();

  template<typename T>
  FlyEmBodyInfoDialog* makeBodyInfoDlg(const T &flag);

  void updateBodyMessage(
      uint64_t bodyId, const ZFlyEmBodyAnnotation &annot);

  void submitSkeletonizationTask(uint64_t bodyId);

  QMenu* makeControlPanelMenu();
  QAction* getAction(ZActionFactory::EAction item);
  void addBodyColorMenu(QMenu *menu);
  void addBodyMenu(QMenu *menu);
//  NeuPrintReader *getNeuPrintReader();

protected:
  bool m_showSegmentation;
  ZFlyEmBodySplitProject m_splitProject;

  QMap<flyem::EProofreadingMode, ZFlyEmBookmarkListModel*>
  m_assignedBookmarkModel;
  QMap<flyem::EProofreadingMode, ZFlyEmBookmarkListModel*>
  m_userBookmarkModel;
//  ZFlyEmBookmarkListModel *m_assignedBookmarkList;
//  ZFlyEmBookmarkListModel *m_userBookmarkList;

//  QSortFilterProxyModel *m_assignedBookmarkProxy;
//  QSortFilterProxyModel *m_userBookmarkProxy;
//  ZFlyEmBookmarkArray m_bookmarkArray;

  ZThreadFutureMap m_futureMap;

//  ZColorLabel *m_latencyLabelWidget;
  ZPaintLabelWidget *m_paintLabelWidget;
  ZActionLibrary *m_actionLibrary = nullptr;

  ZDvidTargetProviderDialog *m_dvidDlg;
  FlyEmBodyInfoDialog *m_bodyInfoDlg;
  FlyEmBodyInfoDialog *m_bodyQueryDlg = nullptr;
  FlyEmBodyInfoDialog *m_neuprintBodyDlg = nullptr;
  ProtocolSwitcher *m_protocolSwitcher;
  ZFlyEmSplitCommitDialog *m_splitCommitDlg;
  FlyEmTodoDialog *m_todoDlg;
  ZFlyEmRoiToolDialog *m_roiDlg;
  ZFlyEmSplitUploadOptionDialog *m_splitUploadDlg;
  ZFlyEmBodyChopDialog *m_bodyChopDlg;
  ZInfoDialog *m_infoDlg;
  ZFlyEmSkeletonUpdateDialog *m_skeletonUpdateDlg;
  ZFlyEmGrayscaleDialog *m_grayscaleDlg;
  FlyEmBodyIdDialog *m_bodyIdDialog;
  ZFlyEmMergeUploadDialog *m_mergeUploadDlg;
  ZFlyEmProofSettingDialog *m_settingDlg;
  ZFlyEmBodyAnnotationDialog *m_annotationDlg = nullptr;
  NeuPrintQueryDialog *m_neuprintQueryDlg = nullptr;
  NeuprintSetupDialog *m_neuprintSetupDlg = nullptr;
  ZContrastProtocalDialog *m_contrastDlg = nullptr;

  QAction *m_prevColorMapAction = nullptr;
  QAction *m_currentColorMapAction = nullptr;

  Z3DMainWindow *m_bodyViewWindow;
  Z3DTabWidget *m_bodyViewers;
  Z3DWindow *m_coarseBodyWindow;
  Z3DWindow *m_bodyWindow;
  Z3DWindow *m_skeletonWindow;
  Z3DWindow *m_meshWindow;
  Z3DWindow *m_coarseMeshWindow;
  Z3DWindow *m_externalNeuronWindow;
  Z3DWindow *m_splitWindow;
  Z3DWindow *m_objectWindow;
  Z3DWindow *m_roiWindow;
  ZFlyEmOrthoWindow *m_orthoWindow;
//  ZFlyEmDataFrame *m_queryWindow;
  QSharedPointer<ZWindowFactory> m_bodyWindowFactory;

  ZStackViewParam m_currentViewParam;

//  ZDvidInfo m_grayScaleInfo;
//  ZDvidInfo m_labelInfo;
  bool m_ROILoaded;

  std::vector<std::string> m_roiList;
  std::vector<ZSharedPointer<ZMesh> > m_loadedROIs;
//  std::vector<ZObject3dScan> m_loadedROIs;
  std::vector<std::string> m_roiSourceList;

  //Data fetching
  ZFlyEmSynapseDataFetcher *m_seFetcher;
  ZFlyEmSynapseDataUpdater *m_seUpdater;

  bool m_quitting = false;
  std::string m_taskKey; //For testing tasks
  bool m_3dEnabled = true;

  QTimer *m_profileTimer = nullptr;
//  ZDvidPatchDataFetcher *m_patchFetcher;
//  ZDvidPatchDataUpdater *m_patchUpdater;
};

template <typename T>
void ZFlyEmProofMvc::connectControlPanel(T *panel)
{
  panel->setMainMenu(makeControlPanelMenu());

  connect(panel, SIGNAL(segmentVisibleChanged(bool)),
          this, SLOT(setSegmentationVisible(bool)));
  connect(panel, SIGNAL(mergingSelected()), this, SLOT(mergeSelected()));
//  connect(panel, SIGNAL(edgeModeToggled(bool)),
//          this, SLOT(toggleEdgeMode(bool)));
  connect(panel, SIGNAL(dvidSetTriggered()), this, SLOT(setDvidTarget()));
  connect(this, SIGNAL(dvidTargetChanged(ZDvidTarget)),
          panel, SLOT(updateWidget(ZDvidTarget)));
  connect(this, SIGNAL(launchingSplit(uint64_t)),
          panel, SIGNAL(splitTriggered(uint64_t)));
  connect(panel, SIGNAL(labelSizeChanged(int, int)),
          this, SLOT(setDvidLabelSliceSize(int, int)));
  connect(panel, SIGNAL(showingFullSegmentation()),
          this, SLOT(showFullSegmentation()));
  connect(panel, SIGNAL(coarseBodyViewTriggered()),
          this, SLOT(showCoarseBody3d()));
  connect(panel, SIGNAL(bodyViewTriggered()),
          this, SLOT(showFineBody3d()));
  connect(panel, SIGNAL(skeletonViewTriggered()),
          this, SLOT(showSkeletonWindow()));
  connect(panel, SIGNAL(meshViewTriggered()),
          this, SLOT(showMeshWindow()));
  connect(panel, SIGNAL(coarseMeshViewTriggered()),
          this, SLOT(showCoarseMeshWindow()));
  connect(panel, SIGNAL(savingMerge()), this, SLOT(saveMergeOperation()));
  connect(panel, SIGNAL(committingMerge()), this, SLOT(commitMerge()));
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(locatingBody(uint64_t)),
          this, SLOT(locateBody(uint64_t)));
  connect(panel, SIGNAL(goingToBody()), this, SLOT(goToBody()));
  connect(panel, SIGNAL(selectingBody()), this, SLOT(selectBody()));
  connect(panel, SIGNAL(showingInfo()), this, SLOT(showInfoDialog()));
//  connect(this, SIGNAL(bookmarkUpdated(ZFlyEmBodyMergeProject*)),
//          panel, SLOT(updateBookmarkTable(ZFlyEmBodyMergeProject*)));
//  connect(this, SIGNAL(bookmarkDeleted(ZFlyEmBodyMergeProject*)),
//          panel, SLOT(clearBookmarkTable(ZFlyEmBodyMergeProject*)));
//  connect(panel, SIGNAL(bookmarkChecked(QString, bool)),
//          this, SLOT(recordCheckedBookmark(QString, bool)));
//  connect(panel, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
//          this, SLOT(recordBookmark(ZFlyEmBookmark*)));
//  connect(this, SIGNAL(userBookmarkUpdated(ZStackDoc*)),
//          panel, SLOT(updateUserBookmarkTable(ZStackDoc*)));
//  connect(panel, SIGNAL(userBookmarkChecked(ZFlyEmBookmark*)),
//          this, SLOT(processCheckedUserBookmark(ZFlyEmBookmark*)));
//  connect(panel, SIGNAL(removingBookmark(ZFlyEmBookmark*)),
//          this, SLOT(removeBookmark(ZFlyEmBookmark*)));
//  connect(panel, SIGNAL(removingBookmark(QList<ZFlyEmBookmark*>)),
//          this, SLOT(removeBookmark(QList<ZFlyEmBookmark*>)));
  connect(panel, SIGNAL(changingColorMap(QString)),
          this, SLOT(changeColorMap(QString)));
//  connect(this, SIGNAL(nameColorMapReady(bool)),
//          panel, SLOT(enableNameColorMap(bool)));
  connect(panel, SIGNAL(clearingBodyMergeStage()),
          this, SLOT(clearBodyMergeStage()));
//  connect(panel, SIGNAL(queryingBody()),
//          this, SLOT(queryBodyByRoi()));
  connect(panel, SIGNAL(exportingSelectedBody()),
          this, SLOT(exportSelectedBody()));
  connect(panel, SIGNAL(exportingSelectedBodyLevel()),
          this, SLOT(exportSelectedBodyLevel()));
  connect(panel, SIGNAL(exportingSelectedBodyStack()),
          this, SLOT(exportSelectedBodyStack()));
  connect(panel, SIGNAL(skeletonizingSelectedBody()),
          this, SLOT(skeletonizeSelectedBody()));
  connect(panel, SIGNAL(skeletonizingTopBody()),
          this, SLOT(skeletonizeSynapseTopBody()));
  connect(panel, SIGNAL(skeletonizingBodyList()),
          this, SLOT(skeletonizeBodyList()));
  connect(panel, SIGNAL(updatingMeshForSelectedBody()),
          this, SLOT(updateMeshForSelected()));
  connect(panel, SIGNAL(reportingBodyCorruption()),
          this, SLOT(reportBodyCorruption()));
  connect(this, SIGNAL(updatingLatency(int)), panel, SLOT(updateLatency(int)));
  connect(this, SIGNAL(stateUpdated(ZFlyEmProofMvc*)),
          panel, SLOT(updateWidget(ZFlyEmProofMvc*)));
}

template <typename T>
void ZFlyEmProofMvc::connectSplitControlPanel(T *panel)
{
  connect(panel, SIGNAL(quickViewTriggered()), this, SLOT(showBodyQuickView()));
  connect(panel, SIGNAL(coarseBodyViewTriggered()),
          this, SLOT(showCoarseBody3d()));
  connect(panel, SIGNAL(splitQuickViewTriggered()),
          this, SLOT(showSplitQuickView()));
  connect(panel, SIGNAL(bodyViewTriggered()), this, SLOT(showBody3d()));
  connect(panel, SIGNAL(meshViewTriggered()), this, SLOT(showMeshWindow()));
//  connect(panel, SIGNAL(splitViewTriggered()), this, SLOT(showSplit3d()));

  connect(panel, SIGNAL(exitingSplit()), this, SLOT(exitSplit()));
  connect(panel, SIGNAL(changingSplit(uint64_t)), this,
          SLOT(switchSplitBody(uint64_t)));
  connect(panel, SIGNAL(savingSeed()), this, SLOT(saveSeed()));
  connect(panel, SIGNAL(committingResult()), this, SLOT(commitCurrentSplit()));
  connect(panel, SIGNAL(loadingBookmark(QString)),
          this, SLOT(loadBookmark(QString)));
//  connect(this, SIGNAL(bookmarkUpdated(ZFlyEmBodySplitProject*)),
//          panel, SLOT(updateBookmarkTable(ZFlyEmBodySplitProject*)));
//  connect(this, SIGNAL(bookmarkDeleted(ZFlyEmBodySplitProject*)),
//          panel, SLOT(clearBookmarkTable(ZFlyEmBodySplitProject*)));
//  connect(this, SIGNAL(userBookmarkUpdated(ZStackDoc*)),
//          panel, SLOT(updateUserBookmarkTable(ZStackDoc*)));
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(settingMainSeed()), this, SLOT(setMainSeed()));
  connect(panel, SIGNAL(selectingSeed()), this, SLOT(selectSeed()));
  connect(panel, SIGNAL(selectingAllSeed()), this, SLOT(selectAllSeed()));
  connect(panel, SIGNAL(recoveringSeed()), this, SLOT(recoverSeed()));
  connect(panel, SIGNAL(exportingSeed()), this, SLOT(exportSeed()));
  connect(panel, SIGNAL(importingSeed()), this, SLOT(importSeed()));
  connect(this, SIGNAL(splitBodyLoaded(uint64_t, flyem::EBodySplitMode)),
          panel, SLOT(updateBodyWidget(uint64_t)));
  connect(panel, SIGNAL(savingTask()), this, SLOT(saveSplitTask()));
  connect(panel, SIGNAL(loadingSplitResult()), this, SLOT(loadSplitResult()));
  connect(panel, SIGNAL(loadingSplitTask()), this, SLOT(loadSplitTask()));
  connect(panel, SIGNAL(uploadingSplitResult()), this, SLOT(uploadSplitResult()));
  connect(panel, SIGNAL(loadingSynapse()), this, SLOT(loadSynapse()));
  connect(panel, SIGNAL(bookmarkChecked(QString, bool)),
          this, SLOT(recordCheckedBookmark(QString, bool)));
  connect(panel, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
          this, SLOT(recordBookmark(ZFlyEmBookmark*)));
  connect(panel, SIGNAL(croppingCoarseBody3D()), this, SLOT(cropCoarseBody3D()));
  connect(panel, SIGNAL(showingBodyGrayscale()), this, SLOT(showBodyGrayscale()));
}


#endif // ZFLYEMPROOFMVC_H
