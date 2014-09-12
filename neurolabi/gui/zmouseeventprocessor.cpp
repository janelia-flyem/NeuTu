#include "zmouseeventprocessor.h"
#include "zimagewidget.h"

ZMouseEventProcessor::ZMouseEventProcessor() :
  m_context(NULL), m_imageWidget(NULL)
{
}

const ZMouseEvent& ZMouseEventProcessor::process(
    QMouseEvent *event, ZMouseEvent::EAction action, int z)
{
  ZMouseEvent zevent;
  zevent.set(event, action, z);
  const ZIntPoint &pt = zevent.getPosition();
  zevent.setRawStackPosition(mapPositionFromWidgetToRawStack(pt));

  return m_recorder.record(zevent);;
}

void ZMouseEventProcessor::setInteractiveContext(ZInteractiveContext *context)
{
  m_context = context;
  m_leftButtonReleaseMapper.setContext(context);
  m_moveMapper.setContext(context);
}

void ZMouseEventProcessor::setImageWidget(ZImageWidget *widget)
{
  m_imageWidget = widget;
}

const ZMouseEventMapper& ZMouseEventProcessor::getMouseEventMapper(
    const ZMouseEvent &event) const
{
  switch (event.getAction()) {
  case ZMouseEvent::ACTION_MOVE:
    return m_moveMapper;
  case ZMouseEvent::ACTION_RELEASE:
    if (event.getButtons() == Qt::LeftButton) {
      return m_leftButtonReleaseMapper;
    }
    break;
  default:
    break;
  }

  return m_emptyMapper;
}

ZMouseEventMapper::EOperation ZMouseEventProcessor::getOperation() const
{
  const ZMouseEvent &event = m_recorder.getLatestMouseEvent();
  if (event.isNull()) {
    return ZMouseEventMapper::OP_NULL;
  }

  return getMouseEventMapper(event).getOperation(event);
}

ZPoint ZMouseEventProcessor::mapPositionFromWidgetToRawStack(
    const ZIntPoint &pt) const
{
  return mapPositionFromWidgetToRawStack(pt.getX(), pt.getY(), pt.getZ());
}

ZPoint ZMouseEventProcessor::mapPositionFromWidgetToRawStack(
    int x, int y, int z) const
{
  ZPoint pt(x, y, z);
  mapPositionFromWidgetToRawStack(pt.xRef(), pt.yRef());

  return pt;
}

void ZMouseEventProcessor::mapPositionFromWidgetToRawStack(double *x, double *y)
const
{
  QSize csize = m_imageWidget->projectSize();

  if (csize.width() > 0 && csize.height() > 0) {
    *x = *x * (m_imageWidget->viewPort().width()) / csize.width() +
        m_imageWidget->viewPort().left() - 0.5;
    *y = *y * (m_imageWidget->viewPort().height()) / csize.height() +
        m_imageWidget->viewPort().top() - 0.5;
  }
}

const ZMouseEvent& ZMouseEventProcessor::getLatestMouseEvent() const
{
  return m_recorder.getLatestMouseEvent();
}

const ZMouseEvent& ZMouseEventProcessor::getMouseEvent(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  return m_recorder.getMouseEvent(buttons, action);
}
