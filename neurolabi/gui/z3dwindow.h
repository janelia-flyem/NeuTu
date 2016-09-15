#ifndef Z3DWINDOW_H
#define Z3DWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <vector>
#include <set>
#include <map>
#include <QDir>
#include "zparameter.h"
#include "znumericparameter.h"
#include "zglmutils.h"
#include "z3dcameraparameter.h"
#include "zactionactivator.h"
#include "z3dvolumeraycasterrenderer.h"
#include "zsharedpointer.h"
#include "zactionfactory.h"
#include "z3ddef.h"
//#include "zstackviewparam.h"


class ZStackDoc;
class Z3DTrackballInteractionHandler;
class Z3DPunctaFilter;
class Z3DSwcFilter;
class Z3DVolumeSource;
class Z3DVolumeRaycaster;
class Z3DGraphFilter;
class Z3DSurfaceFilter;
class ZFlyEmTodoListFilter;
class ZPunctum;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;
class Z3DCompositor;
class Z3DCanvasRenderer;
class Z3DTakeScreenShotWidget;
class Z3DAxis;
class ZWidgetsGroup;
class Z3DCanvas;
class Z3DNetworkEvaluator;
class Z3DProcessorNetwork;
class Z3DTriangleList;
class QToolBar;
class ZStroke2d;
class ZStackViewParam;
class Z3DWindow;
class ZRect2d;
//class Z3DRendererBase;
class ZROIWidget;
class ZActionLibrary;
class ZMenuFactory;
class ZJsonObject;
class Z3DGeometryFilter;

class Z3DTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    Z3DTabWidget(QWidget* parent = 0);
    ~Z3DTabWidget();
    QTabBar* tabBar();

    void addWindow(int index, Z3DWindow *window, const QString &title);
    int getTabIndex(int index);
    int getRealIndex(int index);

public slots:
    void closeWindow(int index);
    void updateTabs(int index);
    void updateWindow(int index);
    void closeAllWindows();

public slots:
    void resetCamera();
    void setXZView();
    void setYZView();

    void settingsPanel(bool v);
    void objectsPanel(bool v);
    void roiPanel(bool v);
    void showGraph(bool v);

    void resetSettingsButton();
    void resetObjectsButton();
    void resetROIButton();

    void resetCameraCenter();

signals:
    void buttonShowGraphToggled(bool);
    void buttonSettingsToggled(bool);
    void buttonObjectsToggled(bool);
    void buttonROIsToggled(bool);
    void buttonROIsClicked();

    void tabIndexChanged(int);

private:
    bool buttonStatus[4][4]; // 0-coarsebody 1-body 2-skeleton 3-synapse 0-showgraph 1-settings 2-objects 3-rois
    bool windowStatus[4]; // 0-coarsebody 1-body 2-skeleton 3-synapse false-closed true-opened
    int tabLUT[4]; // tab index look up table
    int preIndex;

};

class Z3DMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Z3DMainWindow(QWidget* parent = 0);
    ~Z3DMainWindow();

    void closeEvent(QCloseEvent *event);

    void setCurrentWidow(Z3DWindow *window);

private:
    Z3DTabWidget* getCentralTab() const;

private slots:
    void stayOnTop(bool on);

public:
    QToolBar *toolBar;

public:
    QAction *resetCameraAction;
    QAction *xzViewAction;
    QAction *yzViewAction;
    QAction *recenterAction;
    QAction *showGraphAction;
    QAction *settingsAction;
    QAction *objectsAction;
    QAction *roiAction;

    QAction *m_stayOnTopAction;

public slots:
    void updateButtonShowGraph(bool v);
    void updateButtonSettings(bool v);
    void updateButtonObjects(bool v);
    void updateButtonROIs(bool v);

signals:
    void closed();
};

class Z3DWindow : public QMainWindow
{
  Q_OBJECT
public:
  enum EInitMode {
    INIT_NORMAL, INIT_EXCLUDE_VOLUME, INIT_FULL_RES_VOLUME
  };

  enum ERendererLayer {
    LAYER_SWC, LAYER_PUNCTA, LAYER_GRAPH, LAYER_SURFACE, LAYER_VOLUME,
    LAYER_TODO
  };

  explicit Z3DWindow(ZSharedPointer<ZStackDoc> doc, EInitMode initMode,
                     bool stereoView = false, QWidget *parent = 0);
  virtual ~Z3DWindow();

public: //Creators
  static Z3DWindow* Make(ZStackDoc* doc, QWidget *parent,
                         Z3DWindow::EInitMode mode = Z3DWindow::INIT_NORMAL);
  static Z3DWindow* Open(ZStackDoc* doc, QWidget *parent,
                         Z3DWindow::EInitMode mode = Z3DWindow::INIT_NORMAL);
  static Z3DWindow* Make(ZSharedPointer<ZStackDoc> doc, QWidget *parent,
                         Z3DWindow::EInitMode mode = Z3DWindow::INIT_NORMAL);
  static Z3DWindow* Open(ZSharedPointer<ZStackDoc> doc, QWidget *parent,
                         Z3DWindow::EInitMode mode = Z3DWindow::INIT_NORMAL);

public: //utilties
  static std::string GetLayerString(ERendererLayer layer);

public: //properties
  void setZScale(ERendererLayer layer, double scale);
  void setScale(ERendererLayer layer, double sx, double sy, double sz);
  void setZScale(double scale);
  void setScale(double sx, double sy, double sz);
  void setOpacity(ERendererLayer layer, double opacity);
  using QWidget::setVisible; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  void setVisible(ERendererLayer layer, bool visible);
  bool isVisible(ERendererLayer layer) const;

  void configure(const ZJsonObject &obj);

  NeuTube3D::EWindowType getWindowType() const {
    return m_windowType;
  }

  void setWindowType(NeuTube3D::EWindowType type) {
    m_windowType = type;
  }

  void writeSettings();
  void readSettings();

public: //Camera adjustment
  void gotoPosition(double x, double y, double z, double radius = 64);
  void gotoPosition(std::vector<double> bound, double minRadius = 64,
                    double range = 128);
  void zoomToSelectedSwcNodes();


public: //Components
  Z3DTrackballInteractionHandler* getInteractionHandler();
  Z3DCameraParameter* getCamera();
  inline Z3DCanvasRenderer* getCanvasRenderer() { return m_canvasRenderer; }
  inline Z3DPunctaFilter* getPunctaFilter() const { return m_punctaFilter; }
  inline Z3DSwcFilter* getSwcFilter() const { return m_swcFilter; }
  inline Z3DVolumeRaycaster* getVolumeRaycaster() { return m_volumeRaycaster; }
  inline Z3DCanvas* getCanvas() { return m_canvas; }
  inline const Z3DCanvas* getCanvas() const { return m_canvas; }

  Z3DRendererBase* getRendererBase(ERendererLayer layer);

  Z3DVolumeRaycasterRenderer* getVolumeRaycasterRenderer();

  Z3DGeometryFilter* getFilter(ERendererLayer layer) const;

  inline Z3DGraphFilter* getGraphFilter() const { return m_graphFilter; }
  inline Z3DSurfaceFilter* getSurfaceFilter() const { return m_surfaceFilter; }
  inline ZFlyEmTodoListFilter* getTodoFilter() const { return m_todoFilter; }
  inline Z3DCompositor* getCompositor() const { return m_compositor; }
  inline Z3DVolumeSource *getVolumeSource() const { return m_volumeSource; }
  inline Z3DAxis *getAxis() { return m_axis; }
  const std::vector<double>& getBoundBox() const { return m_boundBox; }

  QPointF getScreenProjection(double x, double y, double z, ERendererLayer layer);

public: //Bounding box
  void updateVolumeBoundBox();
  void updateSwcBoundBox();
  void updateGraphBoundBox();
  void updateSurfaceBoundBox();
  void updateTodoBoundBox();
//  void updateDecorationBoundBox();
  void updatePunctaBoundBox();
  void updateOverallBoundBox(std::vector<double> bound);
  void updateOverallBoundBox();       //get bounding box of all objects in world coordinate :[xmin xmax ymin ymax zmin zmax]

  /*!
   * \brief Get the document associated with the window
   */
  inline ZStackDoc* getDocument() const { return m_doc.get(); }
  template <typename T>
  T* getDocument() const {
    return dynamic_cast<T*>(m_doc.get());
  }

  void createToolBar();

  void setBackgroundColor(const glm::vec3 &color1, const glm::vec3 &color2);

  void hideControlPanel();
  void hideObjectView();

  bool hasRectRoi() const;
  ZRect2d getRectRoi() const;
  void removeRectRoi();

  QDockWidget * getSettingsDockWidget();
  QDockWidget * getObjectsDockWidget();
  ZROIWidget * getROIsDockWidget();

public:
  void setButtonStatus(int index, bool v);
  bool getButtonStatus(int index);

  QAction* getAction(ZActionFactory::EAction item);

public:
  void setROIs(size_t n);

public:
  //Control panel setup

protected:

private:
  // UI
  void createMenus();
  void createActions();
  void customizeContextMenu();
  void createContextMenu();
  void createStatusBar();
  void createDockWindows();
  void customizeDockWindows(QTabWidget *m_settingTab);
  void setWindowSize();

  //Configuration
  void configureLayer(ERendererLayer layer, const ZJsonObject &obj);
  ZJsonObject getConfigJson(ERendererLayer layer) const;

  // init 3D view
  void init(EInitMode mode = INIT_NORMAL);

  void cleanup();

  int channelNumber();

  void setupCamera(const std::vector<double> &bound, Z3DCamera::ResetCameraOptions options);

  bool hasVolume();

  //conditions for customization
  bool hasSwc() const;
  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  bool hasMultipleSelectedSwcNode() const;

signals:
  void closed();
  void locating2DViewTriggered(const ZStackViewParam &param);
  void croppingSwcInRoi();
  void addingTodoMarker(int x, int y, int z, bool checked);
  
public slots:
  void resetCamera();  // set up camera based on visible objects in scene, original position
  void resetCameraCenter();

  void flipView(); //Look from the oppsite side
  void setXZView();
  void setYZView();
  void recordView(); //Record the current view parameters
  void diffView(); //Output difference between current view and recorded view
  void saveView(); //Save the view parameters into a file
  void loadView();

  void resetCameraClippingRange(); // // Reset the camera clipping range to include this entire bounding box
  // redraw changed parts
  void volumeChanged();
  void swcChanged();
  void punctaChanged();
  void updateNetworkDisplay();
  void update3DGraphDisplay();
  void update3DCubeDisplay();
  void updateTodoDisplay();
//  void updateDecorationDisplay();
  void updateDisplay();

  void volumeScaleChanged();
  void swcCoordScaleChanged();
  void punctaCoordScaleChanged();
  void swcSizeScaleChanged();
  void punctaSizeScaleChanged();

  void selectdObjectChangedFrom3D(ZStackObject *p, bool append);
  void selectedPunctumChangedFrom3D(ZPunctum* p, bool append);
  void selectedSwcChangedFrom3D(ZSwcTree* p, bool append);
  void selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node* p, bool append);
  void addNewSwcTreeNode(double x, double y, double z, double r);
  void extendSwcTreeNode(double x, double y, double z, double r);
  void connectSwcTreeNode(Swc_Tree_Node *tn);
  void deleteSelectedSwcNode();
  void locateSwcNodeIn2DView();
  void removeSwcTurn();

  void convertSelectedChainToSwc();

  void punctaSelectionChanged();
  void swcSelectionChanged();
  void swcTreeNodeSelectionChanged();
  void updateObjectSelection(QList<ZStackObject*> selected,
                             QList<ZStackObject*> deselected);

  void swcDoubleClicked(ZSwcTree* tree);
  void swcNodeDoubleClicked(Swc_Tree_Node* node);
  void punctaDoubleClicked(ZPunctum* p);
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
  void addDoneMarker();
  void updateBody();

  void takeScreenShot(QString filename, int width, int height, Z3DScreenShotType sst);
  void takeScreenShot(QString filename, Z3DScreenShotType sst);

  void openAdvancedSetting(const QString &name);

  void takeSeriesScreenShot(const QDir& dir, const QString &namePrefix, glm::vec3 axis,
                            bool clockWise, int numFrame, int width, int height,
                            Z3DScreenShotType sst);
  void takeSeriesScreenShot(const QDir& dir, const QString &namePrefix, glm::vec3 axis,
                            bool clockWise, int numFrame, Z3DScreenShotType sst);

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

  void markSwcSoma();

  void selectSwcTreeNodeInRoi(bool appending);
  void cropSwcInRoi();


protected:
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent * event);
//  void paintEvent(QPaintEvent *event);

private:
  QTabWidget* createBasicSettingTabWidget();
  QTabWidget* createAdvancedSettingTabWidget();

  // update menu based on context information
  void updateContextMenu(const QString &group);
  void updateTodoList();

private:
  NeuTube3D::EWindowType m_windowType;

  QList<ERendererLayer> m_layerList;

  // menu
  std::map<QString, QMenu*> m_contextMenuGroup;
  QMenu *m_mergedContextMenu;
  QMenu *m_contextMenu;

  QMenu *m_viewMenu;
  QMenu *m_editMenu;

  ZActionLibrary *m_actionLibrary;
  ZMenuFactory *m_menuFactory;

  QAction *m_removeSelectedObjectsAction;
  QAction *m_openVolumeZoomInViewAction;
  QAction *m_exitVolumeZoomInViewAction;
  QAction *m_markPunctumAction;
  QAction *m_toogleAddSwcNodeModeAction;
  QAction *m_changeBackgroundAction;
  QAction *m_toggleMoveSelectedObjectsAction;
  //QAction *m_toogleExtendSelectedSwcNodeAction;
  QAction *m_toggleSmartExtendSelectedSwcNodeAction;
  QAction *m_locateSwcNodeIn2DAction;

  QAction *m_undoAction;
  QAction *m_redoAction;
  QAction *m_markSwcSomaAction;
  QAction *m_changeSwcNodeTypeAction;
  QAction *m_setSwcRootAction;
  QAction *m_breakSwcConnectionAction;
  QAction *m_connectSwcNodeAction;
  QAction *m_connectToSwcNodeAction;
  QAction *m_mergeSwcNodeAction;
  QAction *m_selectSwcNodeDownstreamAction;
  QAction *m_selectSwcConnectionAction;
  QAction *m_selectSwcNodeBranchAction;
  QAction *m_selectSwcNodeUpstreamAction;
  QAction *m_selectSwcNodeTreeAction;
  QAction *m_selectAllConnectedSwcNodeAction;
  QAction *m_selectAllSwcNodeAction;
  QAction *m_translateSwcNodeAction;
  QAction *m_changeSwcNodeSizeAction;

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

  /*
  QMenu *m_punctaContextMenu;
  QMenu *m_traceMenu;
  QMenu *m_volumeContextMenu;
  QMenu *m_swcContextMenu;
*/

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;


  ZSharedPointer<ZStackDoc> m_doc;
  Z3DNetworkEvaluator *m_networkEvaluator;
  Z3DCanvas *m_canvas;

  // processors
  Z3DCanvasRenderer *m_canvasRenderer;
  Z3DAxis *m_axis;
  Z3DVolumeRaycaster *m_volumeRaycaster;
  Z3DPunctaFilter *m_punctaFilter;
  Z3DCompositor *m_compositor;
  Z3DSwcFilter *m_swcFilter;
  Z3DVolumeSource *m_volumeSource;
  Z3DGraphFilter *m_graphFilter;
  Z3DSurfaceFilter *m_surfaceFilter;
  ZFlyEmTodoListFilter *m_todoFilter;
//  Z3DGraphFilter *m_decorationFilter;

  std::vector<double> m_volumeBoundBox;
  std::vector<double> m_swcBoundBox;
  std::vector<double> m_punctaBoundBox;
  std::vector<double> m_graphBoundBox;
  std::vector<double> m_surfaceBoundBox;
  std::vector<double> m_todoBoundBox;
  std::vector<double> m_decorationBoundBox;
  std::vector<double> m_boundBox;    //overall bound box

  bool m_buttonStatus[4]; // 0-showgraph, 1-setting, 2-objects, 3-rois

  bool m_isClean;   //already cleanup?

  bool m_blockingTraceMenu;

  glm::ivec3 m_lastClickedPosInVolume;

  Z3DTakeScreenShotWidget *m_screenShotWidget;

  ZWidgetsGroup *m_widgetsGroup;

  QDockWidget *m_settingsDockWidget;
  QDockWidget *m_objectsDockWidget;
  QDockWidget *m_advancedSettingDockWidget;
  ZROIWidget *m_roiDockWidget;

  bool m_isStereoView;

  Z3DCamera m_cameraRecord;

  QString m_lastOpenedFilePath;

  QToolBar *m_toolBar;
};

#endif // Z3DWINDOW_H
