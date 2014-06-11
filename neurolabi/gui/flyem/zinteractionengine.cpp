#include "zinteractionengine.h"
#include <QMouseEvent>

ZInteractionEngine::ZInteractionEngine(QObject *parent) :
  QObject(parent), m_dataBuffer(NULL)
{
  m_stroke.setWidth(10.0);
  m_namedDecorationList.append(&m_stroke);

  //m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
}

ZInteractionEngine::~ZInteractionEngine()
{
  foreach (ZStackDrawable *drawable, m_unnamedDecorationList) {
    delete drawable;
  }
}

bool ZInteractionEngine::lockingMouseMoveEvent() const
{
  return m_interactiveContext.strokeEditMode() ==
      ZInteractiveContext::STROKE_DRAW;
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
    }
    emit decorationUpdated();
  }
}

void ZInteractionEngine::processMouseReleaseEvent(
    QMouseEvent *event, int sliceIndex)
{
  UNUSED_PARAMETER(sliceIndex);
  if (event->button() == Qt::LeftButton) {
    commitData();
    m_mouseLeftButtonPressed = false;
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
  m_mouseLeftButtonPressed = true;
}

void ZInteractionEngine::processKeyPressEvent(QKeyEvent *event)
{
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
    m_stroke.setLabel(1);
    emit decorationUpdated();
    break;
  case Qt::Key_2:
    m_stroke.setLabel(2);
    emit decorationUpdated();
    break;
  case Qt::Key_3:
    m_stroke.setLabel(3);
    emit decorationUpdated();
    break;
  case Qt::Key_4:
    m_stroke.setLabel(4);
    emit decorationUpdated();
    break;
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

QList<ZStackDrawable*> ZInteractionEngine::getDecorationList() const
{
  QList<ZStackDrawable*> decorationList;
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
