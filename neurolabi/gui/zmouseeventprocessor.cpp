#include "zmouseeventprocessor.h"
#include "widgets/zimagewidget.h"
#include "neutubeconfig.h"

#include "zinteractivecontext.h"
#include "zstackoperator.h"
#include "zstack.hxx"

#include "mvc/zpositionmapper.h"
#include "mvc/zstackdocutil.h"
#include "mvc/zstackdoc.h"

ZMouseEventProcessor::ZMouseEventProcessor() :
  m_context(NULL)
{
  registerMapper();
}

ZMouseEventProcessor::~ZMouseEventProcessor()
{
  ZOUT(LTRACE(), 5) << "ZMouseEventProcessor destroyed";
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
    QMouseEvent *event, ZMouseEvent::EAction action, int viewId)
{
  switch (action) {
  case ZMouseEvent::EAction::PRESS:
    m_context->blockContextMenu(false);
    break;
  case ZMouseEvent::EAction::RELEASE:
    if (getLatestMouseEvent().getAction() == ZMouseEvent::EAction::DOUBLE_CLICK) {
      return m_emptyEvent;
    }
    break;
  default:
    break;
  }

  ZMouseEvent zevent;
  zevent.set(event, action, m_context->getSliceViewTransform());
//  zevent.setSliceAxis(getSliceAxis());
  const ZIntPoint &pt = zevent.getWidgetPosition();
//  zevent.setRawStackPosition(ZPositionMapper::WidgetToRawStack(pt, viewProj));
//  zevent.setRawStackPosition(mapPositionFromWidgetToRawStack(pt, viewProj));


  ZPoint stackPosition = m_context->getSliceViewTransform().transform(
        ZPoint(pt.getX(), pt.getY(), 0), neutu::data3d::ESpace::CANVAS,
        neutu::data3d::ESpace::VIEW);
  zevent.setStackPosition(stackPosition);


  ZPoint dataPosition = m_context->getSliceViewTransform().transform(
        stackPosition, neutu::data3d::ESpace::VIEW,
        neutu::data3d::ESpace::MODEL);

  zevent.setDataPosition(dataPosition);

  if (m_doc->getStack() &&
      m_doc->getStack()->contains(dataPosition)) {
      zevent.setInStack(true);
  }

#ifdef _DEBUG_
  if (action == ZMouseEvent::EAction::RELEASE) {
    std::cout << "Stack pos r: " << stackPosition << std::endl;
    std::cout << "Data pos r: " << dataPosition << std::endl;
//    std::cout << "Arb plane r: " << m_arbSlice.getOffset().toString() << std::endl;
  }
#endif

#ifdef _DEBUG_2
  std::cout << "Processing mouse event: Z= " << z << std::endl;
  std::cout << "Data positoin: Z=" << dataPos.getZ() << std::endl;
#endif

  zevent.setViewId(viewId);

  return m_recorder.record(zevent);;
}

void ZMouseEventProcessor::setInteractiveContext(ZInteractiveContext *context)
{
  m_context = context;
  foreach (ZMouseEventMapper *mapper, m_mapperList) {
    mapper->setContext(context);
  }
}

/*
void ZMouseEventProcessor::setSliceAxis(neutu::EAxis axis)
{
  m_sliceAxis = axis;
}

void ZMouseEventProcessor::setArbSlice(const ZAffinePlane &ap)
{
  m_arbSlice = ap;
}

neutu::EAxis ZMouseEventProcessor::getSliceAxis() const
{
  return m_sliceAxis;
}
*/

/*
void ZMouseEventProcessor::setImageWidget(ZImageWidget *widget)
{
  m_imageWidget = widget;
}
*/

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
  case ZMouseEvent::EAction::MOVE:
    return m_moveMapper;
  case ZMouseEvent::EAction::RELEASE:
    if (event.getButtons() == Qt::LeftButton) {
      return m_leftButtonReleaseMapper;
    } else if (event.getButtons() == Qt::RightButton) {
      return m_rightButtonReleaseMapper;
    }
    break;
  case ZMouseEvent::EAction::PRESS:
    if (event.getButtons() == Qt::LeftButton) {
      return m_leftButtonPressMapper;
    } else if (event.getButtons() == Qt::RightButton) {
      return m_rightButtonPressMapper;
    } else if (event.getButtons() == (Qt::LeftButton | Qt::RightButton)) {
      return m_bothButtonPressMapper;
    }
    break;
  case ZMouseEvent::EAction::DOUBLE_CLICK:
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
#if 0
ZPoint ZMouseEventProcessor::mapPositionFromWidgetToRawStack(
    const ZIntPoint &pt, const ZViewProj &viewProj) const
{
  return mapPositionFromWidgetToRawStack(
        pt.getX(), pt.getY(), pt.getZ(), viewProj);
}

ZPoint ZMouseEventProcessor::mapPositionFromWidgetToRawStack(
    int x, int y, int z, const ZViewProj &viewProj) const
{
  ZPoint pt(x, y, z);
  mapPositionFromWidgetToRawStack(pt.xRef(), pt.yRef(), viewProj);
  pt.shiftSliceAxis(getSliceAxis());

  return pt;
}

/*
void ZMouseEventProcessor::mapPositionFromWidgetToRawStack(double *x, double *y)
const
{
  ZViewProj viewProj = m_imageWidget->getViewProj();

  viewProj.mapPointBack(x, y);

  (*x) -= viewProj.getCanvasRect().left();
  (*y) -= viewProj.getCanvasRect().top();
}
*/
void ZMouseEventProcessor::mapPositionFromWidgetToRawStack(
    double *x, double *y, const ZViewProj &viewProj) const
{
//  ZViewProj viewProj = m_imageWidget->getViewProj();

  viewProj.mapPointBack(x, y);

  (*x) -= viewProj.getCanvasRect().left();
  (*y) -= viewProj.getCanvasRect().top();
}
#endif

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
  return getLatestMouseEvent().getStackPosition();
  /*
  ZPoint pt = getLatestMouseEvent().getRawStackPosition();

  if (m_doc.get() != NULL) {
    pt += m_doc->getStackOffset();
  }

  return pt;
  */
}

ZPoint ZMouseEventProcessor::getStackPosition(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  return getMouseEvent(buttons, action).getStackPosition();
}

/*
ZPoint ZMouseEventProcessor::getRawStackPosition(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  return getMouseEvent(buttons, action).getRawStackPosition();
}
*/

/*
bool ZMouseEventProcessor::isPositionInStack(const ZMouseEvent &event) const
{
  return getDocument()->getStack()->containsRaw(event.getRawStackPosition());
}
*/
