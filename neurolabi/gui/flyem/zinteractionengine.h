#ifndef ZINTERACTIONENGINE_H
#define ZINTERACTIONENGINE_H

#include <QObject>
#include <QPointF>
#include <QWidget>
//#include "zstackdrawable.h"
#include "zinteractivecontext.h"
#include "zstroke2d.h"
#include "zuncopyable.h"
#include "zstackdocreader.h"
#include "zrect2d.h"
#include "zstackball.h"

class Z3DTrackballInteractionHandler;
class ZStackDocKeyProcessor;
class ZStackOperator;

/*!
 * \brief An experimental class of handling GUI interaction, especially for
 * editing and exploration in 3D. The current implementation wraps
 * ZInteractiveContext and Z3DTrackballInteractionHandler to simplify APIs
 * for interaction management.
 */
class ZInteractionEngine : public QObject, ZUncopyable
{
  Q_OBJECT

public:
  ZInteractionEngine(QObject *parent = NULL);
  ~ZInteractionEngine();

public:
  enum MouseButtonAction {
    MOUSE_LEFT_RELEASE, MOUSE_RIGHT_RELEASE, MOUSE_LEFT_PRESS,
    MOUSE_RIGHT_PRESS, MOUSE_LEFT_DOUBLE_CLICK, MOUSE_MOVE
  };

  enum EMouseEventProcessStatus {
    MOUSE_EVENT_PASSED, CONTEXT_MENU_POPPED, MOUSE_HIT_OBJECT,
    MOUSE_COMMAND_EXECUTED, MOUSE_EVENT_CAPTURED
  };

  enum EState {
    STATE_DRAW_STROKE, STATE_DRAW_LINE, STATE_MARK, STATE_LEFT_BUTTON_PRESSED,
    STATE_RIGHT_BUTTON_PRESSED, STATE_MOVE_OBJECT, STATE_SWC_SMART_EXTEND,
    STATE_SWC_EXTEND, STATE_SWC_CONNECT, STATE_SWC_ADD_NODE,
    STATE_DRAW_RECT, STATE_SWC_SELECTION, STATE_LOCATE, STATE_BROWSE,
    STATE_SHOW_DETAIL
  };

  enum EKeyMode {
    KM_NORMAL, KM_SWC_SELECTION
  };

  QList<ZStackObject*> getDecorationList() const;
  inline ZStackObject::EDisplayStyle getObjectStyle() const { return m_objStyle; }
  inline const ZInteractiveContext& getInteractiveContext() const {
    return m_interactiveContext;
  }
  inline ZInteractiveContext& getInteractiveContext() {
    return m_interactiveContext;
  }

  void set3DInteractionHandler(Z3DTrackballInteractionHandler *handler);

  bool hasObjectToShow() const;
  void setObjectVisible(bool v);
  bool isObjectVisible();
  void setObjectStyle(ZStackObject::EDisplayStyle style);

  void initInteractiveContext();

  bool processMouseReleaseEvent(QMouseEvent *event, int sliceIndex = 0);
  bool processKeyPressEvent(QKeyEvent *event);
  void processMouseMoveEvent(QMouseEvent *event);
  void processMousePressEvent(QMouseEvent *event, int sliceIndex = 0);
  void processMouseDoubleClickEvent(QMouseEvent *eventint, int sliceIndex = 0);

  bool process(const ZStackOperator &op);

  bool lockingMouseMoveEvent() const;

  bool isStateOn(EState status) const;

  bool hasRectDecoration() const;
  const ZRect2d& getRectDecoration() const { return m_rect; }

  void removeRectDecoration();

  Qt::CursorShape getCursorShape() const;
  //void setCursor(const QCursor &c);

  inline void setKeyEventEnabled(bool enabled) {
    m_isKeyEventEnabled = enabled;
  }

  void setKeyMode(EKeyMode mode) {
    m_keyMode = mode;
  }

  EKeyMode getKeyMode() const {
    return m_keyMode;
  }

  void setKeyProcessor(ZStackDocKeyProcessor *processor);

  void showContextMenu();
  void enterPaintStroke();
  void enterMarkTodo();
  void enterMarkBookmark();
  void enterPaintRect();
  void enterLocateMode();
  void enterBrowseMode();
  void enterDetailMode();

  void exitBrowseMode();
  void exitDetailMode();
  void exitMarkTodo();
  void exitMarkBookmark();
  void exitLocateMode();

signals:
  void decorationUpdated();
  void strokePainted(ZStroke2d*);
  void showingContextMenu();
  void selectingSwcNodeInRoi(bool appending);
  void selectingSwcNodeTreeInRoi(bool appending);
  void selectingTerminalBranchInRoi(bool appending);
  void selectingDownstreamSwcNode();
  void selectingUpstreamSwcNode();
  void selectingConnectedSwcNode();
  void croppingSwc();
  void splittingBodyLocal();
  void splittingBody();
  void splittingFullBody();
  void shootingTodo(int x, int y);
  void deletingSelected();
  void locating(int x, int y);
  void browsing(int x, int y);
  void showingDetail(int x, int y);
  void cameraRotated();
  void exitingEdit();

private:
  void exitPaintStroke();
  void exitExplore();
  void exitPaintRect();
  void exitSwcEdit();
  void exitEditMode();
  void saveStroke();
  void commitData();

  void enableRayMarker();

  void suppressMouseRelease(bool s);
  bool mouseReleaseSuppressed() const;

private:
  QList<ZStackObject*> m_unnamedDecorationList; //need to free up
  QList<ZStackObject*> m_namedDecorationList; //no need to free up

//  bool m_showObject;
  ZStackObject::EDisplayStyle m_objStyle;

  bool m_mouseLeftButtonPressed;
  bool m_mouseRightButtonPressed;
  ZInteractiveContext m_interactiveContext;
  int m_cursorRadius;

  int m_mouseMovePosition[3];
//  int m_mouseLeftReleasePosition[3];
//  int m_mouseRightReleasePosition[3];
//  int m_mouseLeftPressPosition[3];
//  int m_mouseRightPressPosition[3];
//  int m_mouseLeftDoubleClickPosition[3];
//  QPointF m_grabPosition;
//  QPointF m_lastMouseDataCoord;

  ZStroke2d m_stroke;
  ZStroke2d m_rayMarker;
  ZStackBall m_exploreMarker;
  ZRect2d m_rect;
  bool m_isStrokeOn;

//  ZStackDocReader *m_dataBuffer;

  Qt::CursorShape m_cursorShape;

  bool m_isKeyEventEnabled;
  ZStackDocKeyProcessor *m_keyProcessor = NULL;

  bool m_mouseReleaseSuppressed = false;

  int m_previousKey;
  Qt::KeyboardModifiers m_previousKeyModifiers;
  EKeyMode m_keyMode;

  Z3DTrackballInteractionHandler* m_interactionHandler;
  //ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;
  //bool m_skipMouseReleaseEvent;
};

#endif // ZINTERACTIONENGINE_H
