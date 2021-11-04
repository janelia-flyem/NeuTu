/**@file zstackpresenter.h
 * @brief Stack presenter
 * @author Ting Zhao
 */

#ifndef _ZSTACKPRESENTER_H_
#define _ZSTACKPRESENTER_H_

#include <QPoint>
#include <QSize>
#include <QObject>
#include <vector>
#include <QMap>

#include "zinteractivecontext.h"
#include "zstroke2d.h"
#include "swctreenode.h"
#include "zactionactivator.h"
#include "zkeyeventswcmapper.h"
#include "zmouseeventmapper.h"
#include "zmouseeventprocessor.h"
#include "zthreadfuturemap.h"
#include "common/zsharedpointer.h"
#include "zkeyoperationmap.h"
#include "zstackball.h"
#include "zactionfactory.h"
#include "zjsonobject.h"
#include "zstackdocselector.h"

class ZStackView;
class ZStackDoc;
class ZStackFrame;
class QMouseEvent;
class QKeyEvent;
class QAction;
class QMenu;
class ZInteractionEvent;
class ZStackOperator;
class ZStackMvc;
class ZKeyOperationConfig;
class ZStackDocMenuFactory;
class ZStackObjectInfoSet;
class ZMenuConfig;
class ZMouseCursorGlyph;

class ZStackPresenter : public QObject {
  Q_OBJECT

protected:
  explicit ZStackPresenter(QWidget *parent = 0);

public:
  ~ZStackPresenter();

  static ZStackPresenter* Make(QWidget *parent);

  ZStackDoc* buddyDocument() const;
  ZStackView* getDefaultView() const;
  ZStackView* getMainView() const;
  ZStackView* getContextView() const;
  std::vector<ZStackView*> getViewList() const;
  ZStackView* getView(int viewId) const;
  ZStackView* getMouseHoveredView() const;
  void forEachView(std::function<void(ZStackView*)> f) const;
  ZSharedPointer<ZStackDoc> getSharedBuddyDocument() const;

  void updateView() const;

  enum MouseButtonAction {
    LEFT_RELEASE, RIGHT_RELEASE, LEFT_PRESS, RIGHT_PRESS, LEFT_DOUBLE_CLICK,
    MOVE
  };

  enum EMouseEventProcessStatus {
    MOUSE_EVENT_PASSED, CONTEXT_MENU_POPPED, MOUSE_HIT_OBJECT,
    MOUSE_COMMAND_EXECUTED, MOUSE_EVENT_CAPTURED
  };

  /*
  enum EObjectRole {
    ROLE_STROKE, ROLE_SWC, ROLE_SYNAPSE, ROLE_BOOKMARK, ROLE_TODO_ITEM
  };
  */

  /*
  enum EActionItem {
    ACTION_EXTEND_SWC_NODE, ACTION_SMART_EXTEND_SWC_NODE,
    ACTION_CONNECT_TO_SWC_NODE, ACTION_ADD_SWC_NODE,
    ACTION_TOGGLE_SWC_SKELETON,
    ACTION_LOCK_SWC_NODE_FOCUS, ACTION_CHANGE_SWC_NODE_FOCUS,
    ACTION_MOVE_SWC_NODE,
    ACTION_ESTIMATE_SWC_NODE_RADIUS,
    ACTION_PAINT_STROKE, ACTION_ERASE_STROKE,
    ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D,
    ACTION_SPLIT_DATA, ACTION_SHOW_BODY_IN_3D,
    ACTION_BODY_SPLIT_START, ACTION_ADD_SPLIT_SEED,
    ACTION_BODY_ANNOTATION, ACTION_BODY_CHECKIN, ACTION_BODY_CHECKOUT,
    ACTION_BODY_FORCE_CHECKIN, ACTION_BODY_DECOMPOSE
  };
  */

  inline double greyScale(int c = 0) const {return m_grayScale[c];}
  inline double greyOffset(int c = 0) const {return m_grayOffset[c];}


  bool usingHighContrastProtocal() const {
    return m_usingHighContrast;
  }

  void useHighContrastProtocal(bool on) {
    m_usingHighContrast = on;
  }

  //inline int zoomRatio() const { return m_zoomRatio; }
  //int zoomRatio() const;
  inline QList<ZStackObject*>* decorations() { return &m_decorationList; }
  QList<ZStackObject*> getActiveDecorationList() const;

  /*
  QList<ZStackObject*> getActiveDecorationList(
      std::function<bool(const ZStackObject*)> pred) const;
      */

  void updateMouseCursorGlyphPos();

  inline const QList<ZStackObject*>& getHighlightDecorationList() const {
    return m_highlightDecorationList;
  }
  inline ZStackObject::EDisplayStyle objectStyle() { return m_objStyle; }
  inline ZInteractiveContext& interactiveContext() {
    return m_interactiveContext;
  }

  void clearData();
  void updateViewLayout();

  bool hasObjectToShow() const;
  void setObjectVisible(bool v);
  void toggleObjectVisible();
  void suppressObjectVisible(bool v);
  bool isObjectVisible();
  void setObjectStyle(neutu::data3d::EDisplayStyle style);
  void setSliceMode(int viewId, neutu::data3d::EDisplaySliceMode mode);
  neutu::data3d::EDisplaySliceMode getSliceMode(int viewId) const;

  bool hasDrawable(neutu::data3d::ETarget target) const;

  void initInteractiveContext();

  bool hightlightOn() const { return m_highlight; }
  void setHighlight(bool state) { m_highlight = state; }
  void highlight(int x, int y, int z);
  void flashHighlight(int x, int y, int z);

  void setSliceAxis(int viewId, neutu::EAxis axis);
  void setMainSliceAxis(neutu::EAxis axis);
  void setViewCount(int n);

  bool paintingRoi() const {
    return m_paintingRoi;
  }

  void setPaintingRoi(bool on) {
    m_paintingRoi = on;
  }

  /*
  void updateZoomOffset(int cx, int cy, int r0);
  void updateZoomOffset(int cx, int cy, int wx, int wy);
  void setZoomOffset(int x, int y);
*/
  void processMouseReleaseEvent(QMouseEvent *event, int viewId);
  void processMouseMoveEvent(QMouseEvent *event, int viewId);
  void processMousePressEvent(QMouseEvent *event, int viewId);
  void processMouseDoubleClickEvent(QMouseEvent *event, int viewId);

  virtual bool processKeyPressEvent(QKeyEvent *event, int viewId);
  bool processKeyPressEventOther(QKeyEvent *event, int viewId);

  virtual bool customKeyProcess(QKeyEvent *event);
  virtual bool processCustomOperator(const ZStackOperator &op,
                                     ZInteractionEvent *e  = NULL);

//  void createActions();
//  void createTraceActions();
//  void createPunctaActions();
//  void createSwcActions();
  //void createTubeActions();
//  void createStrokeActions();
//  void createDocDependentActions();
//  void createBodyActions();
//  void createMiscActions();
  void createMainWindowActions();

  QAction* getAction(ZActionFactory::EAction item) const;

  QAction* makeAction(ZActionFactory::EAction item);
  virtual bool connectAction(QAction *action, ZActionFactory::EAction item);
//{
//    return m_actionMap[item];
//  }

  void createSwcNodeContextMenu();
  QMenu* getSwcNodeContextMenu();

  void createStrokeContextMenu();
  QMenu* getStrokeContextMenu();

  void createStackContextMenu();
  QMenu* getStackContextMenu();

  void createBodyContextMenu();
  QMenu* getBodyContextMenu();

  virtual QMenu* getContextMenu();

  bool isContextMenuOn();

  void setContextMenuFactory(std::unique_ptr<ZStackDocMenuFactory> factory);

  void setStackBc(double scale, double offset, int c = 0);
  double getGrayScale(int c = 0) const;
  double getGrayOffset(int c = 0) const;

  /* optimize stack brightness and contrast */
  void optimizeStackBc();

  void autoThreshold();
  void binarizeStack();
  void solidifyStack();

 // void autoTrace();

  void prepareView();

  void updateLeftMenu(ZStackView *view, QAction *action, bool clear = true);
  void updateRightMenu(ZStackView *view, QAction *action, bool clear = true);
  void updateRightMenu(ZStackView *view, QMenu *submenu, bool clear = true);
  void updateLeftMenu(ZStackView *view);

  //void addTubeEditFunctionToRightMenu();
  void addPunctaEditFunctionToRightMenu(ZStackView *view);
  //void addSwcEditFunctionToRightMenu();

//  void setViewPortCenter(int x, int y, int z);

//  const QPointF stackPositionFromMouse(MouseButtonAction mba);

  ZPoint getModelPositionFromMouse(MouseButtonAction mba) const;
//  ZPoint getModelPositionFromGlobalCursor(const QPoint &pos) const;

  ZPoint getLastMousePosInStack();

  QStringList toStringList() const;

  //void enterSwcEditMode();

  void updateCursor();

//  ZStackObject* getFirstOnActiveObject() const;
  /*
  ZStackObject* getActiveObject(EObjectRole role) const;
  template<typename T>
  T* getActiveObject(EObjectRole role) const;
  */
//  inline const ZStroke2d* getStroke() const { return m_stroke; }

  void setZoomRatio(ZStackView *view, double ratio);
//  void setZoomRatio(double ratio);

  neutu::EAxis getSliceAxis() const;

  ZStackFrame* getParentFrame() const;
  ZStackMvc* getParentMvc() const;
  QWidget* getParentWidget() const;

  void setViewMode(ZInteractiveContext::ViewMode mode);

  //void updateInteractiveContext();

  //void moveImage(int mouseX, int mouseY);
  void moveViewPort(ZStackView *view, int dx, int dy);
  void moveViewPort(ZStackView *view, const ZPoint &src, int a, int b);

  /*!
   * \brief Move the viewport to a certain position.
   *
   * Move the viewport to (\a x, \a y) if possible.
   */
//  void moveViewPortTo(int x, int y);

  /*!
   * \brief Move a data point to the specified mouse position.
   *
   * Move the point at the canvas coordinates (\a srcX, \a srcY) under
   * the mouse point at (\a mouseX, \a mouseY), which are widget coordinates.
   */
//  void moveImageToMouse(double srcX, double srcY, int mouseX, int mouseY);

  void moveCrossHairToMouse(int mouseX, int mouseY);

  void increaseZoomRatio(ZStackView *view);
  void decreaseZoomRatio(ZStackView *view);

  /*!
   * \brief Zoom at a certain point.
   *
   * (\a x, \a y) is the reference point.
   */
  void increaseZoomRatio(ZStackView *view, int x, int y);
  void decreaseZoomRatio(ZStackView *view, int x, int y);

  /*!
   * \brief Get the current slice index.
   *
   * It returns the index of the current active slice. When no slice is active,
   * such as in the senario of the projection mode, the index is set to -1.
   */
//  int getSliceIndex() const;

//  ZStackOperator makeOperator(ZStackOperator::EOperation op);

  bool isOperatable(ZStackOperator::EOperation op);
//  bool isOperatable(const ZStackOperator &op) const;

  bool isSwcFullSkeletonVisible() const;

  virtual ZKeyOperationConfig* getKeyConfig();
  virtual void configKeyMap();

  virtual ZStackDocMenuFactory* getMenuFactory();

  virtual ZMenuConfig getMenuConfig() const;

  bool hasHighContrastProtocal() const;
  ZJsonObject getHighContrastProtocal() const;

  void setHighContrastProtocal(const ZJsonObject &obj);

  void enterSwcAddNodeMode(double x, double y, ZStackObjectRole::TRole role);
  void enterSwcAddNodeMode(ZStackObjectRole::TRole role);

public: //test functions
  void testBiocytinProjectionMask();

  void processEvent(ZInteractionEvent &event);

public slots:
  void addDecoration(ZStackObject *obj, bool tail = true);
  void removeLastDecoration(ZStackObject *obj);
  void removeDecoration(ZStackObject *obj, bool redraw = true);
  void removeAllDecoration();
  void traceTube();
  void fitSegment();
  void fitEllipse();
  void dropSegment();
  void enterMouseCapturingMode();
  void markPuncta();
  void deleteSelected();
  //void deleteAllPuncta();
  void enlargePuncta();
  void narrowPuncta();
  void meanshiftPuncta();
  void meanshiftAllPuncta();
  void updateStackBc();

  void enterSwcConnectMode();
  bool enterSwcExtendMode();
  void exitSwcExtendMode();
  //void enterSwcSmartExtendMode();
  void enterSwcMoveMode();
  void enterSwcSelectMode();
  void enterDrawStrokeMode(double x, double y);
  void enterEraseStrokeMode(double x, double y);
  void exitStrokeEdit();
  void exitEdit();
//  void exitSwcEdit();
  void deleteSwcNode();
  void lockSelectedSwcNodeFocus();
  void changeSelectedSwcNodeFocus();
  void processSliceChangeEvent(int z);
  void estimateSelectedSwcRadius();
  void connectSelectedSwcNode();
  void breakSelectedSwcNode();
  void selectAllSwcTreeNode();
  void toggleSwcSkeleton(bool state);

//  void trySwcAddNodeMode(double x, double y);
  void trySwcAddNodeMode();
  void tryPaintStrokeMode();
  void tryEraseStrokeMode();
  void tryDrawStrokeMode(double x, double y, bool isEraser);

  void tryDrawRectMode();
  void enterDrawRectMode();
//  void tryDrawRectMode();
  void cancelRectRoi();
  void exitRectEdit();
  void exitBookmarkEdit();
  void exitTodoEdit();
  void exitSynapseEdit();

  void selectDownstreamNode();
  void selectSwcNodeConnection(Swc_Tree_Node *lastSelected = NULL);
  void selectUpstreamNode();
  void selectBranchNode();
  void selectTreeNode();
  void selectConnectedNode();

  void notifyBodySplitTriggered();
  void notifyActivateMergeLinkTriggered();
  void notifyBodyDecomposeTriggered();
  void notifyBodyCropTriggered();
  void notifyBodyChopTriggered();
  void notifyBodyMergeTriggered();
  void notifyBodyUnmergeTriggered();
  void notifyBodyAnnotationTriggered();
  void notifyExpertBodyStatus();
  void notifyBodyConnectionTriggered();
  void notifyBodyProfileTriggered();
  void notifyBodyCheckinTriggered();
  void notifyBodyForceCheckinTriggered();
  void notifyBodyCheckoutTriggered();

//  void notifyOrthoViewTriggered();
//  void notifyOrthoViewBigTriggered();

  void slotTest();

  void copyCurrentPosition();
  void copyLabelId();
  void copySupervoxelId();
  void copyNeuroglancerLink();
  void copyNeuroglancerLinkAtRectRoi();

  void notifyUser(const QString &msg);

#if 0
  /*!
   * \brief Turn on the active stroke
   */
//  void turnOnStroke();
  void turnOnActiveObject(EObjectRole role);
  void turnOnActiveObject(
      EObjectRole role, std::function<void(ZStackObject*)> prepare);
  void turnOnActiveObject(
      EObjectRole role, std::function<void(ZStackObject*, EObjectRole)> prepare);

  void setActiveObjectSize(EObjectRole role, double radius);
  void setDefaultActiveObjectSize(EObjectRole role);
#endif
  /*!
   * \brief Turn off the active stroke
   */
//  void turnOffStroke();
  void turnOffActiveObject();
//  void turnOffActiveObject(EObjectRole role);

//  bool isActiveObjectOn(EObjectRole role) const;
//  bool isActiveObjectOn() const;

  /*
  inline bool isStrokeOn() const; { return m_stroke.isVisible(); }
  inline bool isStrokeOff() const; { return isStrokeOn(); }
  */

  const Swc_Tree_Node* getSelectedSwcNode() const;

//  void updateSwcExtensionHint();

  void processObjectModified(const ZStackObjectInfoSet &objSet);

  void setSliceViewTransform(int viewId, const ZSliceViewTransform &transform);
//  ZSliceViewTransform getSliceViewTransform() const;
  void notifyActionTriggered();

signals:
  void mousePositionCaptured(double x, double y, double z);
  void bodySplitTriggered();
  void activateMergeLinkTriggered();
  void bodyAnnotationTriggered();
  void bodyExpertStatusTriggered();
  void bodyConnectionTriggered();
  void bodyProfileTriggered();
  void bodyCheckinTriggered(neutu::EBodySplitMode mode);
  void bodyForceCheckinTriggered();
  void bodyCheckoutTriggered(neutu::EBodySplitMode mode);
//  void labelSliceSelectionChanged();
  void objectVisibleTurnedOn();
//  void exitingRectEdit();
//  void acceptingRectRoi();
  void rectRoiUpdated();
  void bodyDecomposeTriggered();
  void bodyCropTriggered();
  void bodyChopTriggered();
  void bodyMergeTriggered();
  void bodyUnmergeTriggered();
  void orthoViewTriggered(double x, double y, double z);
  void orthoViewBigTriggered(double x, double y, double z);
  void checkingBookmark();
  void uncheckingBookmark();
  void savingStack();
  void movingCrossHairTo(int x, int y);
  void updatingViewLayout(std::vector<int> viewLayoutIndices);
  void updatingViewData();
  void actionTriggered(ZActionFactory::EAction action);

protected:
  void init();
//  void initActiveObject();
//  void prepareActiveDecoration(ZStackObject *obj, EObjectRole role);
  void prepareSwcGlyph(ZStackObject *obj);

  EMouseEventProcessStatus processMouseReleaseForPuncta(
      QMouseEvent *event, const ZPoint &positionInStack);
  /*
  EMouseEventProcessStatus processMouseReleaseForTube(
      QMouseEvent *event, double *positionInStack);
      */
  EMouseEventProcessStatus processMouseReleaseForSwc(
      QMouseEvent *event, const ZPoint &positionInStack);
  EMouseEventProcessStatus processMouseReleaseForStroke(
      QMouseEvent *event, const ZPoint &positionInStack);

  bool processKeyPressEventForActiveStroke(QKeyEvent *event);
  bool processKeyPressEventForSwc(QKeyEvent *event);
  bool processKeyPressEventForStroke(QKeyEvent *event);
  bool processKeyPressEventForStack(QKeyEvent *event);
  bool processKeyPressEventForObject(QKeyEvent *event);

  bool isPointInStack(double x, double y);
//  QPointF mapFromWidgetToStack(const QPoint &pos);
//  QPointF mapFromGlobalToStack(const QPoint &pos);

//  bool estimateActiveStrokeWidth();

  bool process(ZStackOperator &op);
  bool process(ZStackOperator::EOperation op);

  bool paintingStroke() const;
  void acceptActiveStroke();
  void acceptRectRoi(bool appending);
  virtual void processRectRoiUpdate(ZRect2d *rect, bool appending);

//  void addActiveObject(EObjectRole role, ZStackObject *obj);

  ZPoint getMousePositionInStack(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;

  virtual void copyLink(const QString &option) const;

  bool addActiveDecorationAsSwc();
  ZStackBall getActiveDecorationForSwc() const;
  void addActiveMouseGlyphSize(double dr);

  void moveSwcNode(ZStackView *view, double du, double dv);

  template<typename T>
  void startSwcEdit(T option, std::function<void(ZStackObject*)> prepare = nullptr);

  ZPoint getCurrentMousePosition(neutu::data3d::ESpace space);

  void setViewCursor(const QCursor &cursor);
  void updateCutPlane(neutu::EAxis a1, neutu::EAxis a2, neutu::EAxis a3);

protected:
  //ZStackFrame *m_parent;
  QList<ZStackObject*> m_decorationList;
//  QList<ZStackObject*> m_activeDecorationList;
  QList<ZStackObject*> m_highlightDecorationList;

  std::shared_ptr<ZMouseCursorGlyph> m_mouseCursorGlyph;

  bool m_showObject;
  bool m_oldShowObject;
  bool m_paintingRoi;
  std::vector<double> m_grayScale;
  std::vector<double> m_grayOffset;
  int m_threshold;
  neutu::data3d::EDisplayStyle m_objStyle;

  //MouseState m_mouseState;
  bool m_mouseLeftButtonPressed;
  bool m_mouseRightButtonPressed;
  ZInteractiveContext m_interactiveContext;
  int m_cursorRadius;
  int m_contextViewId = -1;
  int m_viewCount = 1;
  neutu::EAxis m_mainViewAxis = neutu::EAxis::Z;

  //  Action map
  QMap<ZActionFactory::EAction, QAction*> m_actionMap;

  QMenu *m_swcNodeContextMenu;
  QMenu *m_strokePaintContextMenu;
  QMenu *m_stackContextMenu;
  QMenu *m_bodyContextMenu;
  QMenu *m_contextMenu = NULL;

  //recorded information
  int m_mouseMovePosition[3];
//  int m_mouseLeftReleasePosition[3];
  int m_mouseRightReleasePosition[3];
  int m_mouseLeftPressPosition[3];
  int m_mouseRightPressPosition[3];
  int m_mouseLeftDoubleClickPosition[3];
//  QPointF m_grabPosition;
//  ZPoint m_lastMouseDataCoord;

//  QMap<EObjectRole, ZStackObject*> m_activeObjectMap;
//  QMap<EObjectRole, double> m_defaultDecorationSize;
//  ZStroke2d m_stroke;
//  ZStroke2d m_swcStroke;
//  bool m_isStrokeOn;

  ZStackBall m_highlightDecoration;
  bool m_highlight;
  uint64_t m_highlightTimestamp = 0;
  int m_highlightInterval = 5000;

  bool m_usingHighContrast;
  ZJsonObject m_highContrastProtocal;

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;
  int m_skipMouseReleaseEvent;

  ZKeyOperationMap m_activeStrokeOperationMap;
  ZKeyOperationMap m_swcKeyOperationMap;
  ZKeyOperationMap m_stackKeyOperationMap;
  ZKeyOperationMap m_objectKeyOperationMap;
  ZKeyOperationConfig *m_keyConfig;

//  ZKeyEventSwcMapper m_swcKeyMapper;
  //ZMouseEventLeftButtonReleaseMapper m_leftButtonReleaseMapper;
  //ZMouseEventMoveMapper m_moveMapper;

  ZMouseEventProcessor m_mouseEventProcessor;
  ZStackDocSelector m_docSelector;

  ZActionFactory *m_actionFactory;
  std::unique_ptr<ZStackDocMenuFactory> m_menuFactory;

  int m_zOrder;

  ZThreadFutureMap m_futureMap;

signals:
  void viewModeChanged();
};

/*
template<typename T>
T* ZStackPresenter::getActiveObject(EObjectRole role) const
{
  return dynamic_cast<T*>(getActiveObject(role));
}
*/

#endif
