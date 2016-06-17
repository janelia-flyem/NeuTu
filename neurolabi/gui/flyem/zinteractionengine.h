#ifndef ZINTERACTIONENGINE_H
#define ZINTERACTIONENGINE_H

#include <QObject>
#include <QPointF>
#include <QWidget>
#include "zstackdrawable.h"
#include "zinteractivecontext.h"
#include "zstroke2d.h"
#include "zuncopyable.h"
#include "zstackdoc.h"
#include "z3dinteractionhandler.h"

/*!
 * \brief An experimental class of handling GUI interaction
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
    STATE_DRAW_STROKE, STATE_DRAW_LINE, STATE_LEFT_BUTTON_PRESSED,
    STATE_RIGHT_BUTTON_PRESSED, STATE_MOVE_OBJECT, STATE_SWC_SMART_EXTEND,
    STATE_SWC_EXTEND, STATE_SWC_CONNECT, STATE_SWC_ADD_NODE,
    STATE_DRAW_RECT, STATE_SWC_SELECTION
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

  inline void set3DInteractionHandler(Z3DTrackballInteractionHandler *handler) {
    m_interactionHandler = handler;
  }

  bool hasObjectToShow() const;
  void setObjectVisible(bool v);
  bool isObjectVisible();
  void setObjectStyle(ZStackObject::EDisplayStyle style);

  void initInteractiveContext();

  void processMouseReleaseEvent(QMouseEvent *event, int sliceIndex = 0);
  bool processKeyPressEvent(QKeyEvent *event);
  void processMouseMoveEvent(QMouseEvent *event);
  void processMousePressEvent(QMouseEvent *event, int sliceIndex = 0);
  void processMouseDoubleClickEvent(QMouseEvent *eventint, int sliceIndex = 0);

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

  void showContextMenu();

signals:
  void decorationUpdated();
  void strokePainted(ZStroke2d*);
  void showingContextMenu();
  void selectingSwcNodeInRoi(bool appending);
  void selectingDownstreamSwcNode();
  void selectingUpstreamSwcNode();
  void selectingConnectedSwcNode();
  void croppingSwc();

private:
  void enterPaintStroke();
  void exitPaintStroke();
  void enterPaintRect();
  void exitPaintRect();
  void exitSwcEdit();
  void saveStroke();
  void commitData();

private:
  QList<ZStackObject*> m_unnamedDecorationList; //need to free up
  QList<ZStackObject*> m_namedDecorationList; //no need to free up

  bool m_showObject;
  ZStackObject::EDisplayStyle m_objStyle;

  bool m_mouseLeftButtonPressed;
  bool m_mouseRightButtonPressed;
  ZInteractiveContext m_interactiveContext;
  int m_cursorRadius;

  int m_mouseMovePosition[3];
  int m_mouseLeftReleasePosition[3];
  int m_mouseRightReleasePosition[3];
  int m_mouseLeftPressPosition[3];
  int m_mouseRightPressPosition[3];
  int m_mouseLeftDoubleClickPosition[3];
  QPointF m_grabPosition;
  QPointF m_lastMouseDataCoord;

  ZStroke2d m_stroke;
  ZRect2d m_rect;
  bool m_isStrokeOn;

  ZStackDocReader *m_dataBuffer;

  Qt::CursorShape m_cursorShape;

  bool m_isKeyEventEnabled;


  int m_previousKey;
  Qt::KeyboardModifiers m_previousKeyModifiers;
  EKeyMode m_keyMode;

  Z3DTrackballInteractionHandler* m_interactionHandler;
  //ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;
  //bool m_skipMouseReleaseEvent;
};

#endif // ZINTERACTIONENGINE_H
