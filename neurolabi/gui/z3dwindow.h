#ifndef Z3DWINDOW_H
#define Z3DWINDOW_H

#include <vector>
#include <set>
#include <map>
#include <memory>

#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QMutex>
#include <QDir>

#include "z3dview.h"
//#include "zparameter.h"
#include "widgets/znumericparameter.h"
#include "zglmutils.h"
#include "z3dcameraparameter.h"
#include "zactionactivator.h"
#include "z3dvolumeraycasterrenderer.h"
#include "common/zsharedpointer.h"
#include "zactionfactory.h"
#include "z3ddef.h"
#include "geometry/zintpointarray.h"

class QSlider;
class QDoubleSpinBox;
class ZStackDoc;
class Z3DTrackballInteractionHandler;
class Z3DPunctaFilter;
class Z3DSwcFilter;
//class Z3DVolumeSource;
//class Z3DVolumeRaycaster;
class Z3DGraphFilter;
class Z3DSurfaceFilter;
class ZFlyEmTodoListFilter;
class ZPunctum;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;
class Z3DCompositor;
class Z3DCanvasRenderer;
class Z3DAxis;
class ZWidgetsGroup;
class Z3DCanvas;
class Z3DNetworkEvaluator;
class Z3DProcessorNetwork;
class QToolBar;
class ZStroke2d;
class ZStackViewParam;
class Z3DWindow;
class ZRect2d;
class ZSwcIsolationDialog;
class HelpDialog;
//class Z3DRendererBase;
class ZROIWidget;
class ZActionLibrary;
//class ZMenuFactory;
class ZJsonObject;
class Z3DGeometryFilter;
class Z3DBoundedFilter;
class ZComboEditDialog;
class ZFlyEmBodyComparisonDialog;
class ZStackDocMenuFactory;
class ZLineSegment;
class ZObject3d;
class ZWidgetMessage;
class ZFlyEmBodyEnv;
class ZFlyEmTodoFilterDialog;
class ZRoiMesh;
class ZRoiProvider;
class ZFlyEmBody3dDoc;

class Z3DWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit Z3DWindow(ZSharedPointer<ZStackDoc> doc, Z3DView::EInitMode initMode,
                     neutu3d::EWindowType windowType = neutu3d::EWindowType::GENERAL,
                     bool stereoView = false, QWidget *parent = 0);
  virtual ~Z3DWindow();

public: //Creators
  static Z3DWindow* Make(ZStackDoc* doc, QWidget *parent,
                         Z3DView::EInitMode mode = Z3DView::EInitMode::NORMAL);
  static Z3DWindow* Open(ZStackDoc* doc, QWidget *parent,
                         Z3DView::EInitMode mode = Z3DView::EInitMode::NORMAL);
  static Z3DWindow* Make(ZSharedPointer<ZStackDoc> doc, QWidget *parent,
                         Z3DView::EInitMode mode = Z3DView::EInitMode::NORMAL);
  static Z3DWindow* Open(ZSharedPointer<ZStackDoc> doc, QWidget *parent,
                         Z3DView::EInitMode mode = Z3DView::EInitMode::NORMAL);
public:
  void configure(const ZJsonObject &obj);

  neutu3d::EWindowType getWindowType() const {
    return m_windowType;
  }

  void setWindowType(neutu3d::EWindowType type) {
    m_windowType = type;
  }

  void writeSettings();
  void readSettings();
  void setCutBox(neutu3d::ERendererLayer layer, const ZIntCuboid &box);
  void resetCutBox(neutu3d::ERendererLayer layer);

  bool isLayerVisible(neutu3d::ERendererLayer layer) const;

  void setZScale(double s);
  void setLayerVisible(neutu3d::ERendererLayer layer, bool visible);
  void setOpacity(neutu3d::ERendererLayer layer, double opacity);
  void setOpacityQuietly(neutu3d::ERendererLayer layer, double opacity);
  void setFront(neutu3d::ERendererLayer layer, bool on);
  void setColorMode(neutu3d::ERendererLayer layer, const std::string &mode);

  void configureMenuForNeu3();

public: //Camera adjustment
  void gotoPosition(const ZCuboid& bound);
  void gotoPosition(const ZPoint& position, double radius);
  void zoomToSelectedSwcNodes();
  void zoomToSelected();

public: //Components
  Z3DTrackballInteractionHandler* getInteractionHandler() {
    return &m_view->interactionHandler(); }
  Z3DCameraParameter* getCamera() { return &m_view->camera(); }
//  Z3DTransformParameter getTransformPara() const;
  inline Z3DPunctaFilter* getPunctaFilter() const { return m_view->getPunctaFilter(); }
  inline Z3DMeshFilter* getMeshFilter() const { return m_view->getMeshFilter(); }
  inline Z3DSwcFilter* getSwcFilter() const { return m_view->getSwcFilter(); }
  inline Z3DVolumeFilter* getVolumeFilter() const { return m_view->getVolumeFilter(); }
  inline Z3DCanvas* getCanvas() { return &m_view->canvas(); }
  inline const Z3DCanvas* getCanvas() const { return &m_view->canvas(); }

  Z3DGeometryFilter* getFilter(neutu3d::ERendererLayer layer) const;
  Z3DBoundedFilter* getBoundedFilter(neutu3d::ERendererLayer layer) const;

  inline Z3DGraphFilter* getGraphFilter() const { return m_view->getGraphFilter(); }
  inline Z3DSurfaceFilter* getSurfaceFilter() const { return m_view->getSurfaceFilter(); }
  inline ZFlyEmTodoListFilter* getTodoFilter() const { return m_view->getTodoFilter(); }
  inline Z3DCompositor* getCompositor() const { return &m_view->compositor(); }


  /*!
   * \brief Get the document associated with the window
   */
  inline ZStackDoc* getDocument() const { return m_doc.get(); }
  template <typename T>
  T* getDocument() const {
    return qobject_cast<T*>(m_doc.get());
  }

  inline ZSharedPointer<ZStackDoc> getSharedDocument() const {
    return m_doc;
  }

  ZFlyEmBodyEnv *getBodyEnv() const;

public:
  void setMenuFactory(ZStackDocMenuFactory *factory);

public:
  void setBackgroundColor(const glm::vec3 &color1, const glm::vec3 &color2);

  bool hasRectRoi() const;
  ZRect2d getRectRoi() const;
  void removeRectRoi();

  bool isProjectedInRectRoi(const ZIntPoint &pt) const;

//  void initRois(const std::vector<std::shared_ptr<ZRoiMesh>> &roiList);
  void initRoiView(const std::shared_ptr<ZRoiProvider> &roiProvider);
  void registerRoiWidget(ZROIWidget *widget);

public: //controls
  void createToolBar();
  void hideControlPanel();
  void showControlPanel();
  void hideObjectView();
  void hideStatusBar();

  QToolBar* getToolBar() const;

  void setButtonStatus(int index, bool v);
  bool getButtonStatus(int index);

  QAction* getAction(ZActionFactory::EAction item);
  void setActionChecked(ZActionFactory::EAction item, bool on);

  QDockWidget * getSettingsDockWidget();
  QDockWidget * getObjectsDockWidget();
  ZROIWidget * getROIsDockWidget();

  //Configuration
  void configureLayer(neutu3d::ERendererLayer layer, const ZJsonObject &obj);
//  ZJsonObject getConfigJson(neutube3d::ERendererLayer layer) const;

  void skipKeyEvent(bool on);

  void syncAction();

public:
  bool readyForAction(ZActionFactory::EAction action) const;

public:
  //Control panel setup

public: //external signal call
  void emitAddTodoMarker(int x, int y, int z, bool checked, uint64_t bodyId);
  void emitAddToMergeMarker(int x, int y, int z, uint64_t bodyId);
  void emitAddToSplitMarker(int x, int y, int z, uint64_t bodyId);
  void emitAddTodoMarker(const ZIntPoint &pt, bool checked, uint64_t bodyId);
  void emitAddToMergeMarker(const ZIntPoint &pt, uint64_t bodyId);
  void emitAddToSplitMarker(const ZIntPoint &pt, uint64_t bodyId);
  void emitAddToSupervoxelSplitMarker(int x, int y, int z, uint64_t bodyId);
  void emitAddToSupervoxelSplitMarker(const ZIntPoint &pt, uint64_t bodyId);
  void emitAddTraceToSomaMarker(const ZIntPoint &pt, uint64_t bodyId);
  void emitAddNoSomaMarker(const ZIntPoint &pt, uint64_t bodyId);

signals:
  void closed();
//  void locating2DViewTriggered(const ZStackViewParam &param);
  void locating2DViewTriggered(int x, int y, int z, int width);
  void croppingSwcInRoi();
  void savingSplitTask();
  void deletingSplitSeed();
  void deletingSelectedSplitSeed();
  void savingSplitTask(uint64_t bodyId);
  void viewingDataExternally();

  void addingTodoMarker(int x, int y, int z, bool checked, uint64_t bodyId);
  void addingToMergeMarker(int x, int y, int z, uint64_t bodyId);
  void addingToSplitMarker(int x, int y, int z, uint64_t bodyId);
  void addingToSupervoxelSplitMarker(int x, int y, int z, uint64_t bodyId);
  void addingTraceToSomaMarker(int x, int y, int z, uint64_t bodyId);
  void addingNoSomaMarker(int x, int y, int z, uint64_t bodyId);

  void deselectingBody(const std::set<uint64_t> bodyId);
  void settingNormalTodoVisible(bool);
  void settingDoneItemVisible(bool);
  void showingPuncta(bool);
  void showingTodo(bool);
  void keyPressed(QKeyEvent *event);
  void testing();
  void browsing(double x, double y, double z);
  void runningLocalSplit();
  void runningSplit();
  void runningFullSplit();

  void settingTriggered();
  void neutuTriggered();

  void cameraRotated();
  void messageGenerated(const ZWidgetMessage &msg);

  void diagnosing();

public slots:
  void resetCamera()
  { m_view->resetCamera(); }
  void resetCameraCenter()
  { m_view->resetCameraCenter(); }
  void flipView() //Look from the oppsite side
  { m_view->flipView(); }
  void setXZView()
  { m_view->setXZView(); }
  void setYZView()
  { m_view->setYZView(); }
  void recordView(); //Record the current view parameters
  void diffView(); //Output difference between current view and recorded view
  void saveView(); //Save the view parameters into a file
  void loadView();
  void copyView();
  void pasteView();
  void saveAllVisibleMesh();

  void resetCameraClippingRange() // // Reset the camera clipping range to include this entire bounding box
  { m_view->resetCameraClippingRange(); }

  void zoomToSelectedMeshes();
  void zoomToRoiMesh(const QString &name);
  void selectMeshByID();
  void selectAllMeshes();

//  void updateDecorationDisplay();

  void selectedObjectChangedFrom3D(ZStackObject *p, bool append);
  void selectedPunctumChangedFrom3D(ZPunctum* p, bool append);
  void selectedTodoChangedFrom3D(ZStackObject *p, bool append);
  void selectedGraphChangedFrom3D(ZStackObject *p, bool append);
  void selectedMeshChangedFrom3D(ZMesh* p, bool append);
  void selectedSwcChangedFrom3D(ZSwcTree* p, bool append);
  void selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node* p, bool append);
  void selectedSwcTreeNodeChangedFrom3D(
      QList<Swc_Tree_Node*> nodeArray, bool append);
  void addNewSwcTreeNode(double x, double y, double z, double r);
  void extendSwcTreeNode(double x, double y, double z, double r);
  void connectSwcTreeNode(Swc_Tree_Node *tn);
  void deleteSelectedSwcNode();
  void locateSwcNodeIn2DView();
  void removeSwcTurn();
  void deleteSelected();
  void annotateTodo(ZStackObject* obj);
  void removeTodoBatch();

  void convertSelectedChainToSwc();

  void swcDoubleClicked(ZSwcTree* tree);
  void swcNodeDoubleClicked(Swc_Tree_Node* node);
  void punctaDoubleClicked(ZPunctum* p);
  void meshDoubleClicked(ZMesh* p);
  void pointInVolumeLeftClicked(QPoint pt, glm::ivec3 pos,
                                Qt::KeyboardModifiers modifiers);

  void changeSelectedSwcNodeType();
  void setRootAsSelectedSwcNode();
  void breakSelectedSwcNode();
  void connectSelectedSwcNode();
  void startConnectingSwcNode();
  void mergeSelectedSwcNode();
  void tranlateSelectedSwcNode();
  void changeSelectedSwcNodeSize();
  void showSeletedSwcNodeLength();
  void showSeletedSwcNodeDist();

  void showPuncta(bool on);
  void showTodo(bool on);
  void activateTodoAction(bool on);
//  void activateTosplitAction(bool on);
  void activateBookmarkAction(bool on);
  void activateLocateAction(bool on);

  void syncActionToNormalMode();

  void saveSelectedSwc();
  void changeSelectedSwcType();
  void changeSelectedSwcSize();
  void transformSelectedSwc();
  void breakSelectedSwc();
  void groupSelectedSwc();
  void showSelectedSwcInfo();
  void refreshTraceMask();
  void test();
  void changeSelectedSwcColor();
  void changeSelectedSwcAlpha();

  void transformSelectedPuncta();
  void transformAllPuncta();
  void convertPunctaToSwc();
  void changeSelectedPunctaColor();
  void hideSelectedPuncta();
  void hideUnselectedPuncta();
  void showUnselectedPuncta();
  void showSelectedPuncta();
  void setSelectPunctaVisible(bool on);
  void setUnselectPunctaVisible(bool on);
  void addPunctaSelection();

  void saveSplitTask();
  void deleteSplitSeed();
  void deleteSelectedSplitSeed();
  void viewDataExternally(bool on);
  void viewDetail(bool on);
  //
  void show3DViewContextMenu(QPoint pt);

  // trace menu action
  void traceTube();

  void openZoomInView();
  void exitZoomInView();

  void removeSelectedObject();

  // puncta context menu action
  void markSelectedPunctaProperty1();
  void markSelectedPunctaProperty2();
  void markSelectedPunctaProperty3();
  void unmarkSelectedPunctaProperty1();
  void unmarkSelectedPunctaProperty2();
  void unmarkSelectedPunctaProperty3();
  void saveSelectedPunctaAs();
  void saveAllPunctaAs();
  void markPunctum();
  void locatePunctumIn2DView();
  void locate2DView(const ZPoint &center, double radius);
  void changeSelectedPunctaName();
  void addTodoMarker();
  void addToMergeMarker();
  void addToSplitMarker();
  void addToSupervoxelSplitMarker();
  void addTraceToSomaMarker();
  void addNoSomaMarker();
  void setTodoItemToSplit();
  void setTodoItemToNormal();
  void setTodoItemIrrelevant();
  void setTodoItemTraceToSoma();
  void setTodoItemNoSoma();
  void addDoneMarker();
  void updateBody();
  void syncBodyColor();
  void compareBody();
  void deselectBody();
  void copyPosition();
  void setNormalTodoVisible(bool visible);
  void setDoneItemVisible(bool visible);
//  void updateTodoVisibility();
  void toggleSetting();
  void toggleObjects();
//  void removeAllTodo();


  void takeScreenShot(QString filename, int width, int height, Z3DScreenShotType sst)
  { m_view->takeFixedSizeScreenShot(filename, width, height, sst); }
  void takeScreenShot(QString filename, Z3DScreenShotType sst)
  { m_view->takeScreenShot(filename, sst); }

  void openAdvancedSetting(const QString &name);

  void updateSettingsDockWidget();

  void toogleAddSwcNodeMode(bool checked);
  //void toogleExtendSelectedSwcNodeMode(bool checked);
  void toogleSmartExtendSelectedSwcNodeMode(bool checked);
  void changeBackground();
  bool isBackgroundOn() const;

  void toogleMoveSelectedObjectsMode(bool checked);
  void moveSelectedObjects(double x, double y, double z);
  void notifyUser(const QString &message);

  void addStrokeFrom3dPaint(ZStroke2d*stroke);
  void addPolyplaneFrom3dPaint(ZStroke2d*stroke);
  void processStroke(ZStroke2d *stroke);

  void markSwcSoma();
  void help();
  void diagnose();

  void selectSwcTreeNodeInRoi(bool appending);
  void selectSwcTreeNodeTreeInRoi(bool appending);
  void selectTerminalBranchInRoi(bool appending);
  void cropSwcInRoi();

  void updateCuttingBox();
  void shootTodo(int x, int y);
  void locateWithRay(int x, int y);
  void browseWithRay(int x, int y);
  void showDetail(int x, int y);
  void checkSelectedTodo();
  void uncheckSelectedTodo();

  void setMeshOpacity(double opacity);

  void processMessage(const ZWidgetMessage &msg);

protected:
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent * event);

protected:

private:
  // UI
  void createMenus();
  void createActions();
  void customizeContextMenu();
  void createContextMenu();
  void createStatusBar();
  void createDockWindows();
  void fillDockWindows();
  void customizeDockWindows(QTabWidget *m_settingTab);
  void setWindowSize();
  // init 3D view
  void init();

  void cleanup();

  int channelNumber();

  bool hasVolume();

  //conditions for customization
  bool hasSwc() const;
  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  bool hasMultipleSelectedSwcNode() const;

  bool addingSwcNode() const;
  bool movingObject() const;
  bool extendingSwc() const;
  void exitAddingSwcNode();
  void exitMovingObject();
  void exitExtendingSwc();

  bool exitEditMode();
  bool canSelectObject() const;

  void selectSwcNodeFromStroke(const ZStroke2d *stroke);
  void labelSwcNodeFromStroke(const ZStroke2d *stroke);
  //Experimental function
  void addTodoMarkerFromStroke(const ZStroke2d *stroke);

  ZLineSegment getStackSeg(const ZLineSegment &seg, const ZCuboid &rbox) const;

  std::vector<ZPoint> getRayIntersection(int x, int y, uint64_t *id = NULL);

  ZObject3d* createPolyplaneFrom3dPaintForMesh(ZStroke2d *stroke);
  ZObject3d* createPolyplaneFrom3dPaintForVolume(ZStroke2d *stroke);
  std::string updatePolyLinePairList(
      const ZStroke2d *stroke,
      std::vector<std::pair<ZIntPointArrayPtr, ZIntPointArrayPtr> > &polylinePairList);

  std::vector<ZPoint> shootMesh(const ZMesh *mesh, int x, int y);

  void onSelectionChangedFrom3D(Z3DGeometryFilter *filter,
      ZStackObject *p, ZStackObject::EType type, bool append);

private:
  ZCuboid getRayBoundbox() const;
  ZLineSegment getRaySegment(int x, int y, std::string &source) const;

private:
  QTabWidget* createBasicSettingTabWidget();
  QTabWidget* createAdvancedSettingTabWidget();

  QTabWidget* getSettingsTabWidget() const;

  // update menu based on context information
  void updateContextMenu(const QString &group);


private slots:
  void about();
  void notifyCameraRotation();
  void startBodySplit();

private:
  neutu3d::EWindowType m_windowType;

  // menu
  std::map<QString, QMenu*> m_contextMenuGroup;
  QMenu *m_mergedContextMenu;
  QMenu *m_contextMenu;

  QMenu *m_viewMenu;
  QMenu *m_editMenu;

  ZActionLibrary *m_actionLibrary;
  ZStackDocMenuFactory *m_menuFactory;
  QMenu *m_helpMenu;

  QAction *m_removeSelectedObjectsAction;
  QAction *m_openVolumeZoomInViewAction;
  QAction *m_exitVolumeZoomInViewAction;
  QAction *m_markPunctumAction;
  QAction *m_toggleAddSwcNodeModeAction;
  QAction *m_changeBackgroundAction;
  QAction *m_toggleObjectsAction;
  QAction *m_toggleSettingsAction;
  QAction *m_toggleMoveSelectedObjectsAction;
  //QAction *m_toogleExtendSelectedSwcNodeAction;
  QAction *m_toggleSmartExtendSelectedSwcNodeAction;
  QAction *m_locateSwcNodeIn2DAction;

//  QAction *m_undoAction;
//  QAction *m_redoAction;
  QAction *m_markSwcSomaAction;
  QAction *m_changeSwcNodeTypeAction;
  QAction *m_setSwcRootAction;
  QAction *m_breakSwcConnectionAction;
  QAction *m_connectSwcNodeAction;
  QAction *m_connectToSwcNodeAction;
  QAction *m_mergeSwcNodeAction;
//  QAction *m_selectSwcNodeDownstreamAction;
  QAction *m_selectSwcConnectionAction;
//  QAction *m_selectSwcNodeBranchAction;
//  QAction *m_selectSwcNodeUpstreamAction;
  QAction *m_selectSwcNodeTreeAction;
  QAction *m_selectAllConnectedSwcNodeAction;
  QAction *m_selectAllSwcNodeAction;
  QAction *m_translateSwcNodeAction;
  QAction *m_changeSwcNodeSizeAction;
  QAction *m_helpAction;
  QAction *m_diagnoseAction;

  QAction *m_refreshTraceMaskAction;

  QAction *m_saveSwcAction;
  QAction *m_changeSwcTypeAction;
  QAction *m_changeSwcSizeAction;
  QAction *m_transformSwcAction;
  QAction *m_groupSwcAction;
  QAction *m_breakForestAction;
  QAction *m_changeSwcColorAction;
  QAction *m_changeSwcAlphaAction;
  QAction *m_swcInfoAction;
  QAction *m_swcNodeLengthAction;
  QAction *m_removeSwcTurnAction;
  QAction *m_resolveCrossoverAction;

  QAction *m_saveSelectedPunctaAsAction;
  QAction *m_changePunctaNameAction;
  QAction *m_saveAllPunctaAsAction;
  QAction *m_locatePunctumIn2DAction;

  QActionGroup *m_interactActionGroup = nullptr;

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;


  ZSharedPointer<ZStackDoc> m_doc;
  Z3DView* m_view;

  ZSharedPointer<ZFlyEmBodyEnv> m_bodyEnv;

  bool m_buttonStatus[4]; // 0-showgraph, 1-setting, 2-objects, 3-rois

  bool m_isClean;   //already cleanup?

  bool m_blockingTraceMenu;
  bool m_skippingKeyEvent = false;

  glm::ivec3 m_lastClickedPosInVolume;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;

  QDockWidget *m_settingsDockWidget;
  QDockWidget *m_objectsDockWidget;
  QDockWidget *m_advancedSettingDockWidget;
  ZROIWidget *m_roiDockWidget;
  QDockWidget *m_bodyWidget;

  bool m_cuttingStackBound;

  Z3DCamera m_cameraRecord;

  QString m_lastOpenedFilePath;

  QToolBar *m_toolBar = NULL;
//  QSlider *m_meshOpacitySlider = NULL;
  QDoubleSpinBox *m_meshOpacitySpinBox = NULL;

  mutable QMutex m_filterMutex;
  ZSwcIsolationDialog *m_swcIsolationDlg;
  HelpDialog *m_helpDlg;
  ZComboEditDialog *m_dvidDlg;
  ZFlyEmBodyComparisonDialog *m_bodyCmpDlg;
  ZFlyEmTodoFilterDialog *m_todoDlg = nullptr;
};

#endif // Z3DWINDOW_H
