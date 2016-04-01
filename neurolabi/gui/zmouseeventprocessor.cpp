#include "zmouseeventprocessor.h"
#include "widgets/zimagewidget.h"
#include "zstackdoc.h"
#include "zinteractivecontext.h"
#include "zstackoperator.h"
#include "zstack.hxx"

ZMouseEventProcessor::ZMouseEventProcessor() :
  m_context(NULL), m_imageWidget(NULL)
{
  registerMapper();
}

ZMouseEventProcessor::~ZMouseEventProcessor()
{
  qDebug() << "ZMouseEventProcessor destroyed";
}

void ZMouseEventProcessor::registerMapper()
{
  m_mapperList.append(&m_leftButtonReleaseMapper);
  m_mapperList.append(&m_moveMapper);
  m_mapperList.append(&m_rightButtonReleaseMapper);
  m_mapperList.append(&m_leftButtonDoubleClickMapper);
  m_mapperList.append(&m_leftButtonPressMapper);
  m_mapperList.append(&m_rightButtonPressMapper);
  m_mapperList.append(&m_bothButtonPressMapper);
  foreach (ZMouseEventMapper *mapper, m_mapperList) {
    mapper->setRecorder(&m_recorder);
  }
}

const ZMouseEvent& ZMouseEventProcessor::process(
    QMouseEvent *event, ZMouseEvent::EAction action, int z)
{
  switch (action) {
  case ZMouseEvent::ACTION_PRESS:
    m_context->blockContextMenu(false);
    break;
  case ZMouseEvent::ACTION_RELEASE:
    if (getLatestMouseEvent().getAction() == ZMouseEvent::ACTION_DOUBLE_CLICK) {
      return m_emptyEvent;
    }
    break;
  default:
    break;
  }

  ZMouseEvent zevent;
  zevent.set(event, action, z);
  const ZIntPoint &pt = zevent.getPosition();
  zevent.setRawStackPosition(mapPositionFromWidgetToRawStack(pt));

  if (m_doc->hasStack()) {
    if (m_doc->getStack()->containsRaw(zevent.getRawStackPosition())) {
      zevent.setInStack(true);
    }
  }

  ZPoint stackPosition = zevent.getRawStackPosition();
  if (m_doc.get() != NULL) {
    stackPosition += m_doc->getStackOffset();
  }
  zevent.setStackPosition(stackPosition);

  return m_recorder.record(zevent);;
}

void ZMouseEventProcessor::setInteractiveContext(ZInteractiveContext *context)
{
  m_context = context;
  foreach (ZMouseEventMapper *mapper, m_mapperList) {
    mapper->setContext(context);
  }
}

void ZMouseEventProcessor::setImageWidget(ZImageWidget *widget)
{
  m_imageWidget = widget;
}

void ZMouseEventProcessor::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_doc = doc;
  m_leftButtonReleaseMapper.setDocument(doc);
  foreach (ZMouseEventMapper *mapper, m_mapperList) {
    mapper->setDocument(doc);
  }
}

const ZStackDoc* ZMouseEventProcessor::getDocument() const
{
  return m_doc.get();
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
    } else if (event.getButtons() == Qt::RightButton) {
      return m_rightButtonReleaseMapper;
    }
    break;
  case ZMouseEvent::ACTION_PRESS:
    if (event.getButtons() == Qt::LeftButton) {
      return m_leftButtonPressMapper;
    } else if (event.getButtons() == Qt::RightButton) {
      return m_rightButtonPressMapper;
    } else if (event.getButtons() == (Qt::LeftButton | Qt::RightButton)) {
      return m_bothButtonPressMapper;
    }
    break;
  case ZMouseEvent::ACTION_DOUBLE_CLICK:
    if (event.getButtons() == Qt::LeftButton) {
      return m_leftButtonDoubleClickMapper;
    }
    break;
  default:
    break;
  }

  return m_emptyMapper;
}

ZStackOperator ZMouseEventProcessor::getOperator() const
{
  const ZMouseEvent &event = m_recorder.getLatestMouseEvent();
  if (event.isNull()) {
    return ZStackOperator();
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
  pt.shiftSliceAxis(m_imageWidget->getSliceAxis());

  return pt;
}

void ZMouseEventProcessor::mapPositionFromWidgetToRawStack(double *x, double *y)
const
{
  QSizeF csize = m_imageWidget->projectSize();

  if (csize.width() > 0 && csize.height() > 0) {
    *x = *x * (m_imageWidget->viewPort().width()) / csize.width() +
        m_imageWidget->viewPort().left() -
        m_imageWidget->canvasRegion().left() - 0.5;
    *y = *y * (m_imageWidget->viewPort().height()) / csize.height() +
        m_imageWidget->viewPort().top() -
        m_imageWidget->canvasRegion().top() - 0.5;
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

ZPoint ZMouseEventProcessor::getLatestStackPosition() const
{
  ZPoint pt = getLatestMouseEvent().getRawStackPosition();

  if (m_doc.get() != NULL) {
    pt += m_doc->getStackOffset();
  }

  return pt;
}

ZPoint ZMouseEventProcessor::getStackPosition(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  return getMouseEvent(buttons, action).getStackPosition();
}

ZPoint ZMouseEventProcessor::getRawStackPosition(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  return getMouseEvent(buttons, action).getRawStackPosition();
}

bool ZMouseEventProcessor::isPositionInStack(const ZMouseEvent &event) const
{
  return getDocument()->getStack()->containsRaw(event.getRawStackPosition());
}
