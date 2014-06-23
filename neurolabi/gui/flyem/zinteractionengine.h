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
    LEFT_RELEASE, RIGHT_RELEASE, LEFT_PRESS, RIGHT_PRESS, LEFT_DOUBLE_CLICK, MOVE
  };

  enum EMouseEventProcessStatus {
    MOUSE_EVENT_PASSED, CONTEXT_MENU_POPPED, MOUSE_HIT_OBJECT,
    MOUSE_COMMAND_EXECUTED, MOUSE_EVENT_CAPTURED
  };

  enum EState {
    STATE_DRAW_STROKE, STATE_DRAW_LINE, STATE_LEFT_BUTTON_PRESSED,
    STATE_RIGHT_BUTTON_PRESSED
  };

  QList<ZStackDrawable*> getDecorationList() const;
  inline ZStackDrawable::Display_Style getObjectStyle() const { return m_objStyle; }
  inline const ZInteractiveContext& getInteractiveContext() const {
    return m_interactiveContext;
  }

  bool hasObjectToShow() const;
  void setObjectVisible(bool v);
  bool isObjectVisible();
  void setObjectStyle(ZStackDrawable::Display_Style style);

  void initInteractiveContext();

  void processMouseReleaseEvent(QMouseEvent *event, int sliceIndex = 0);
  void processKeyPressEvent(QKeyEvent *event);
  void processMouseMoveEvent(QMouseEvent *event);
  void processMousePressEvent(QMouseEvent *event, int sliceIndex = 0);
  void processMouseDoubleClickEvent(QMouseEvent *eventint, int sliceIndex = 0);

  bool lockingMouseMoveEvent() const;

  bool isStateOn(EState status) const;

signals:
  void decorationUpdated();
  void strokePainted(ZStroke2d*);

private:
  void enterPaintStroke();
  void exitPaintStroke();
  void saveStroke();
  void commitData();

private:
  QList<ZStackDrawable*> m_unnamedDecorationList; //need to free up
  QList<ZStackDrawable*> m_namedDecorationList; //no need to free up

  bool m_showObject;
  ZStackDrawable::Display_Style m_objStyle;

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
  bool m_isStrokeOn;

  ZStackDocReader *m_dataBuffer;

  //ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;
  //bool m_skipMouseReleaseEvent;
};

#endif // ZINTERACTIONENGINE_H
