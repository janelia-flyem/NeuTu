#include "zinteractionengine.h"
#include <QMouseEvent>

ZInteractionEngine::ZInteractionEngine(QObject *parent) :
  QObject(parent), m_showObject(true), m_objStyle(ZStackObject::NORMAL),
  m_mouseLeftButtonPressed(false), m_mouseRightButtonPressed(false),
  m_cursorRadius(5), m_isStrokeOn(false), m_dataBuffer(NULL),
  m_isKeyEventEnabled(true), m_interactionHandler(NULL)
{
  m_stroke.setWidth(10.0);
  m_namedDecorationList.append(&m_stroke);

  //m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
}

ZInteractionEngine::~ZInteractionEngine()
{
  foreach (ZStackObject *drawable, m_unnamedDecorationList) {
    delete drawable;
  }
}

bool ZInteractionEngine::lockingMouseMoveEvent() const
{
  return isStateOn(STATE_DRAW_STROKE);
}

void ZInteractionEngine::processMouseMoveEvent(QMouseEvent *event)
{
  m_mouseMovePosition[0] = event->x();
  m_mouseMovePosition[1] = event->y();

  if (m_interactiveContext.strokeEditMode() ==
      ZInteractiveContext::STROKE_DRAW) {
    if (m_mouseLeftButtonPressed == true){
      m_stroke.append(event->x(), event->y());
      event->accept();
    } else {
      m_stroke.set(event->x(), event->y());
      m_stroke.toggleLabel(event->modifiers() == Qt::ShiftModifier);
    }
#ifdef _DEBUG_2
    std::cout << "decorationUpdated emitted" << std::endl;
#endif

    emit decorationUpdated();
  }
}

void ZInteractionEngine::processMouseReleaseEvent(
    QMouseEvent *event, int sliceIndex)
{
  UNUSED_PARAMETER(sliceIndex);
  if (event->button() == Qt::LeftButton) {
    if (isStateOn(STATE_DRAW_STROKE)) {
      commitData();
    }
    m_mouseLeftButtonPressed = false;
  } else if (event->button() == Qt::RightButton) {
    if (isStateOn(STATE_DRAW_STROKE)) {
      exitPaintStroke();
      event->accept();
    }
    m_mouseRightButtonPressed = false;
  }
}

void ZInteractionEngine::commitData()
{
  saveStroke();
}

void ZInteractionEngine::processMousePressEvent(QMouseEvent *event,
                                                int sliceIndex)
{
  UNUSED_PARAMETER(sliceIndex);
  UNUSED_PARAMETER(event);

  if (event->button() == Qt::LeftButton) {
    m_mouseLeftButtonPressed = true;
  } else if (event->button() == Qt::RightButton) {
    m_mouseRightButtonPressed = true;
  }
}

void ZInteractionEngine::processKeyPressEvent(QKeyEvent *event)
{
  if (!m_isKeyEventEnabled) {
    return;
  }

  switch (event->key()) {
  case Qt::Key_R:
    if (event->modifiers() == Qt::ControlModifier) {
      enterPaintStroke();
    }
    break;
  case Qt::Key_Escape:
    exitPaintStroke();
    break;
  case Qt::Key_1:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(1);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_2:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(2);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_3:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(3);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_4:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(4);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_5:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(5);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_6:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(6);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_7:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(7);
      emit decorationUpdated();
    }
    break;
  case Qt::Key_QuoteLeft:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(255);
      emit decorationUpdated();
    }
  default:
    break;
  }
}

void ZInteractionEngine::enterPaintStroke()
{
  m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  m_stroke.set(m_mouseMovePosition[0], m_mouseMovePosition[1]);
  m_stroke.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::exitPaintStroke()
{
  m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_EDIT_OFF);
  m_stroke.setVisible(false);
  emit decorationUpdated();
}

QList<ZStackObject*> ZInteractionEngine::getDecorationList() const
{
  QList<ZStackObject*> decorationList;
  decorationList.append(m_namedDecorationList);
  decorationList.append(m_unnamedDecorationList);

  return decorationList;
}

void ZInteractionEngine::saveStroke()
{
  //if (m_dataBuffer != NULL) {
  if (m_stroke.isVisible() && !m_stroke.isEmpty()) {
    ZStroke2d *stroke = new ZStroke2d;
    *stroke = m_stroke;
    //m_dataBuffer->addStroke(stroke);
    emit strokePainted(stroke);
  }
  //}
}

bool ZInteractionEngine::isStateOn(EState status) const
{
  switch (status) {
  case STATE_DRAW_LINE:
    return false;
  case STATE_DRAW_STROKE:
    return m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW;
  case STATE_LEFT_BUTTON_PRESSED:
    return m_mouseLeftButtonPressed;
  case STATE_RIGHT_BUTTON_PRESSED:
    return m_mouseRightButtonPressed;
  case STATE_MOVE_OBJECT:
    if (m_interactionHandler != NULL) {
      return m_interactionHandler->isMovingObjects();
    }
  }

  return false;
}

Qt::CursorShape ZInteractionEngine::getCursorShape() const
{
  if (isStateOn(STATE_DRAW_STROKE)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_MOVE_OBJECT)) {
    return Qt::ClosedHandCursor;
  }

  return Qt::ArrowCursor;
}

