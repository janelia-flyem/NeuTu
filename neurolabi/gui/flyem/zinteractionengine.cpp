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
  m_rect.setColor(255, 0, 0, 128);
  m_namedDecorationList.append(&m_rect);
  m_previousKey = Qt::Key_unknown;
  m_previousKeyModifiers = Qt::NoModifier;
  m_keyMode = KM_NORMAL;
}

ZInteractionEngine::~ZInteractionEngine()
{
  foreach (ZStackObject *drawable, m_unnamedDecorationList) {
    delete drawable;
  }
}

bool ZInteractionEngine::lockingMouseMoveEvent() const
{
  return isStateOn(STATE_DRAW_STROKE) || isStateOn(STATE_DRAW_RECT);
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
  }else if (m_interactiveContext.rectEditMode() ==
            ZInteractiveContext::RECT_DRAW) {
    if (m_mouseLeftButtonPressed == true) {
      m_rect.setLastCorner(event->x(), event->y());
//      m_rect.makeValid();
    }

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
    } else if (isStateOn(STATE_DRAW_RECT)) {
      m_rect.makeValid();
      exitPaintRect();
    }
    m_mouseLeftButtonPressed = false;
  } else if (event->button() == Qt::RightButton) {
    if (isStateOn(STATE_DRAW_STROKE)) {
      exitPaintStroke();
      event->accept();
    }
    if (m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_OFF) {
      exitSwcEdit();
    }
    m_mouseRightButtonPressed = false;
  }
}

void ZInteractionEngine::showContextMenu()
{
  emit showingContextMenu();
}

void ZInteractionEngine::commitData()
{
  saveStroke();
}

bool ZInteractionEngine::hasRectDecoration() const
{
  return m_rect.isValid();
}

void ZInteractionEngine::removeRectDecoration()
{
  m_rect.setSize(0, 0);
}


void ZInteractionEngine::processMousePressEvent(QMouseEvent *event,
                                                int sliceIndex)
{
  UNUSED_PARAMETER(sliceIndex);
  UNUSED_PARAMETER(event);

  if (event->button() == Qt::LeftButton) {
    m_mouseLeftButtonPressed = true;
    if (isStateOn(STATE_DRAW_RECT)) {
      m_rect.setFirstCorner(event->x(), event->y());
      m_rect.setSize(0, 0);
    }
  } else if (event->button() == Qt::RightButton) {
    m_mouseRightButtonPressed = true;
  }
}

bool ZInteractionEngine::processKeyPressEvent(QKeyEvent *event)
{
  if (!m_isKeyEventEnabled) {
    return false;
  }

  bool processed = false;

  switch (event->key()) {
  case Qt::Key_R:
    if (event->modifiers() == Qt::ShiftModifier) {
      enterPaintRect();
      processed = true;
    }
    break;
  case Qt::Key_Escape:
    if (isStateOn(STATE_DRAW_STROKE)) {
      exitPaintStroke();
      processed = true;
    }

    if (isStateOn(STATE_DRAW_RECT)) {
      exitPaintRect();
      processed = true;
    }
    break;
  case Qt::Key_1:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(1);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_2:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(2);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_3:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(3);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_4:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(4);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_5:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(5);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_6:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(6);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_7:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(7);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_QuoteLeft:
    if (isStateOn(STATE_DRAW_STROKE)) {
      m_stroke.setLabel(255);
      emit decorationUpdated();
      processed = true;
    }
    break;
  case Qt::Key_S:
    if (hasRectDecoration()) {
      if (event->modifiers() == Qt::ShiftModifier) {
        emit selectingSwcNodeInRoi(true);
      } else {
        emit selectingSwcNodeInRoi(false);
      }
      processed = true;
    }
    break;
  case Qt::Key_X:
    if (hasRectDecoration()) {
      emit croppingSwc();
      processed = true;
    }
    break;
  default:
    break;
  }

  m_previousKey = event->key();
  m_previousKeyModifiers = event->modifiers();

  return processed;
}

void ZInteractionEngine::enterPaintStroke()
{
  m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  m_stroke.set(m_mouseMovePosition[0], m_mouseMovePosition[1]);
  m_stroke.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::enterPaintRect()
{
  if (isStateOn(STATE_DRAW_STROKE)) {
    exitPaintStroke();
  }

//  m_rect.setVisible(true);
  m_interactiveContext.setRectEditMode(ZInteractiveContext::RECT_DRAW);
  emit decorationUpdated();
}

void ZInteractionEngine::exitPaintRect()
{
  m_interactiveContext.setRectEditMode(ZInteractiveContext::RECT_EDIT_OFF);
//  m_rect.setVisible(false);
  emit decorationUpdated();
}

void ZInteractionEngine::exitSwcEdit()
{
  m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
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
  case STATE_DRAW_RECT:
    return m_interactiveContext.rectEditMode() ==
        ZInteractiveContext::RECT_DRAW;
  case STATE_LEFT_BUTTON_PRESSED:
    return m_mouseLeftButtonPressed;
  case STATE_RIGHT_BUTTON_PRESSED:
    return m_mouseRightButtonPressed;
  case STATE_MOVE_OBJECT:
    if (m_interactionHandler != NULL) {
      return m_interactionHandler->isMovingObjects();
    }
    break;
  case STATE_SWC_EXTEND:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_EXTEND;
  case STATE_SWC_SMART_EXTEND:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_SMART_EXTEND;
  case STATE_SWC_CONNECT:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_CONNECT;
  case STATE_SWC_ADD_NODE:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_ADD_NODE;
  case STATE_SWC_SELECTION:
    return m_keyMode == KM_SWC_SELECTION;
  }

  return false;
}

Qt::CursorShape ZInteractionEngine::getCursorShape() const
{
  if (isStateOn(STATE_DRAW_STROKE)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_MOVE_OBJECT)) {
    return Qt::ClosedHandCursor;
  } else if (isStateOn(STATE_SWC_EXTEND) || isStateOn(STATE_SWC_SMART_EXTEND)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_SWC_CONNECT)) {
    return Qt::SizeBDiagCursor;
  } else if (isStateOn(STATE_SWC_ADD_NODE)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_DRAW_RECT)) {
    return Qt::PointingHandCursor;
  }

  return Qt::ArrowCursor;
}

