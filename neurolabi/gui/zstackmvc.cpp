#include "zstackmvc.h"

#include <QMainWindow>
#include <QMimeData>
#include <QEvent>
#include <QCoreApplication>
#include <QKeyEvent>

#include "zstackdoc.h"
#include "zstackview.h"
#include "zstackpresenter.h"
#include "zprogresssignal.h"
#include "zwidgetmessage.h"
#include "zfiletype.h"
#include "widgets/zimagewidget.h"
#include "zintpoint.h"
#include "zstackviewlocator.h"

ZStackMvc::ZStackMvc(QWidget *parent) :
  QWidget(parent)
{
  m_view = NULL;
  m_presenter = NULL;
  m_layout = new QHBoxLayout(this);
  m_progressSignal = new ZProgressSignal(this);
  setAcceptDrops(true);

  qRegisterMetaType<uint64_t>("uint64_t");
  qRegisterMetaType<ZWidgetMessage>("ZWidgetMessage");
}

ZStackMvc::~ZStackMvc()
{
  qDebug() << "ZStackMvc destroyed";
}

ZStackMvc* ZStackMvc::Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, NeuTube::Z_AXIS);

  return frame;
}

ZStackMvc* ZStackMvc::Make(
    QWidget *parent, ztr1::shared_ptr<ZStackDoc> doc, NeuTube::EAxis axis)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, axis);
//  frame->getView()->setSliceAxis(axis);
//  frame->getPresenter()->setSliceAxis(axis);

  return frame;
}

void ZStackMvc::construct(ztr1::shared_ptr<ZStackDoc> doc, NeuTube::EAxis axis)
{
  dropDocument(ZSharedPointer<ZStackDoc>(doc));
  createView(axis);
  createPresenter(axis);
  updateDocument();

  m_view->prepareDocument();
  m_presenter->prepareView();

  customInit();
}

void ZStackMvc::customInit()
{

}

void ZStackMvc::BaseConstruct(
    ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc, NeuTube::EAxis axis)
{
  frame->construct(doc, axis);
}

void ZStackMvc::createView()
{
  createView(NeuTube::Z_AXIS);
}

void ZStackMvc::createView(NeuTube::EAxis axis)
{
  if (m_doc.get() != NULL) {
    m_view = new ZStackView(qobject_cast<QWidget*>(this));
    m_layout->addWidget(m_view);
    m_view->setSliceAxis(axis);
  }
}

void ZStackMvc::createPresenter()
{
  if (m_doc.get() != NULL) {
    m_presenter = ZStackPresenter::Make(this);
  }
}

void ZStackMvc::createPresenter(NeuTube::EAxis axis)
{
  createPresenter();
  if (m_presenter != NULL) {
    m_presenter->setSliceAxis(axis);
  }
}

void ZStackMvc::attachDocument(ZStackDoc *doc)
{
  attachDocument(ZSharedPointer<ZStackDoc>(doc));
}

void ZStackMvc::attachDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_doc = doc;
}

bool ZStackMvc::connectFunc(const QObject* obj1, const char *signal,
                            const QObject *obj2, const char *slot)
{
  return connect(obj1, signal, obj2, slot);
}

void ZStackMvc::connectSignalSlot()
{
  updateSignalSlot(connectFunc);
//  UPDATE_SIGNAL_SLOT(connect);
}

void ZStackMvc::updateDocSignalSlot(FConnectAction connectAction)
{
//  connectAction(m_doc.get(), SIGNAL(stackLoaded()), this, SIGNAL(stackLoaded()));
  connectAction(m_doc.get(), SIGNAL(messageGenerated(ZWidgetMessage)),
          this, SIGNAL(messageGenerated(ZWidgetMessage)));
  connectAction(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateChannelControl()));
  connectAction(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateThresholdSlider()));
  connectAction(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(updateSlider()));
  connectAction(m_doc.get(), SIGNAL(stackModified()),
          m_presenter, SLOT(updateStackBc()));
  connectAction(m_doc.get(), SIGNAL(stackModified()),
          m_view, SLOT(redraw()));
  connectAction(m_doc.get(), SIGNAL(objectModified()),
                m_view, SLOT(paintObject()));
  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObject::ETarget)),
          m_view, SLOT(paintObject(ZStackObject::ETarget)));
  connectAction(m_doc.get(), SIGNAL(objectModified()),
                m_view, SLOT(paintObject()));
  connectAction(m_doc.get(), SIGNAL(objectModified(QSet<ZStackObject::ETarget>)),
          m_view, SLOT(paintObject(QSet<ZStackObject::ETarget>)));
//  connectAction(m_doc.get(), SIGNAL(cleanChanged(bool)),
//                this, SLOT(changeWindowTitle(bool)));
  connectAction(m_doc.get(), SIGNAL(holdSegChanged()),
                m_view, SLOT(paintObject()));
//  connectAction(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged()),
//          this, SLOT(updateSwcExtensionHint()));
  connectAction(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
                m_view, SLOT(paintObject()));
  connectAction(
        m_doc.get(), SIGNAL(objectSelectionChanged(
                              QList<ZStackObject*>,QList<ZStackObject*>)),
        m_view, SLOT(paintObject(QList<ZStackObject*>,QList<ZStackObject*>)));
  connectAction(m_doc.get(),
                SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
                m_view, SLOT(paintObject()));
  connectAction(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),
          m_view, SLOT(paintObject()));
  connectAction(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),
          m_view, SLOT(paintObject()));
  connectAction(m_doc.get(), SIGNAL(punctumVisibleStateChanged()),
          m_view, SLOT(paintObject()));
//  connectAction(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),
//          this, SLOT(notifyUser(QString)));
  connectAction(m_doc.get(), SIGNAL(stackTargetModified()),
                m_view, SLOT(paintStack()));
  connectAction(m_view, SIGNAL(viewChanged(ZStackViewParam)),
          this, SLOT(processViewChange(ZStackViewParam)));
}

void ZStackMvc::updateSignalSlot(FConnectAction connectAction)
{
  updateDocSignalSlot(connectAction);
//  connectAction(m_view, SIGNAL(currentSliceChanged(int)),
//                m_presenter, SLOT(processSliceChangeEvent(int)));
}

#if 0
#define UPDATE_DOC_SIGNAL_SLOT(connect) \
  connect(m_doc.get(), SIGNAL(stackLoaded()), this, SIGNAL(stackLoaded()));\
  connect(m_doc.get(), SIGNAL(messageGenerated(ZWidgetMessage)), \
          this, SIGNAL(messageGenerated(ZWidgetMessage)));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateChannelControl()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateThresholdSlider()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateSlider()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_presenter, SLOT(updateStackBc()));\
  connect(m_doc.get(), SIGNAL(stackModified()),\
          m_view, SLOT(updateView()));\
  connect(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(objectModified(ZStackObject::ETarget)), \
          m_view, SLOT(paintObject(ZStackObject::ETarget)));\
  connect(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(objectModified(QSet<ZStackObject::ETarget>)), \
          m_view, SLOT(paintObject(QSet<ZStackObject::ETarget>)));\
  connect(m_doc.get(), SIGNAL(cleanChanged(bool)),\
          this, SLOT(changeWindowTitle(bool)));\
  connect(m_doc.get(), SIGNAL(holdSegChanged()), m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(chainSelectionChanged(QList<ZLocsegChain*>,\
          QList<ZLocsegChain*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged()),\
          this, SLOT(updateSwcExtensionHint()));\
  connect(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(\
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(objectSelectionChanged(\
                                QList<ZStackObject*>,QList<ZStackObject*>)),\
          m_view, SLOT(paintObject(QList<ZStackObject*>,QList<ZStackObject*>)));\
  connect(m_doc.get(), SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(punctumVisibleStateChanged()),\
          m_view, SLOT(paintObject()));\
  connect(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),\
          this, SLOT(notifyUser(QString)));\
  connect(m_doc.get(), SIGNAL(stackTargetModified()), m_view, SLOT(paintStack()));\
  connect(m_doc.get(), SIGNAL(thresholdChanged(int)), m_view, SLOT(setThreshold(int)));\
  connect(m_view, SIGNAL(viewChanged(ZStackViewParam)), \
          this, SLOT(processViewChange(ZStackViewParam)));

#define UPDATE_SIGNAL_SLOT(connect) \
  UPDATE_DOC_SIGNAL_SLOT(connect) \
  connect(m_view, SIGNAL(currentSliceChanged(int)),\
          m_presenter, SLOT(processSliceChangeEvent(int)));
#endif
  //connect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));


void ZStackMvc::disconnectAll()
{
  updateSignalSlot(disconnect);
}

void ZStackMvc::dropDocument(ztr1::shared_ptr<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    if (m_doc.get() != NULL) {
      updateSignalSlot(disconnect);
      m_doc->removeUser(this);
    }

    m_doc = doc;
    m_doc->registerUser(this);
    ZProgressSignal::ConnectProgress(m_doc->getProgressSignal(),
                                     getProgressSignal());
    //m_doc->setParentFrame(this);
  }
}

void ZStackMvc::updateDocument()
{
  updateDocSignalSlot(connectFunc);

  if (m_doc->hasStackData()) {
    if (m_presenter != NULL) {
      m_presenter->optimizeStackBc();
    }

    if (m_view != NULL) {
      m_view->reset();
    }
  }
}

void ZStackMvc::keyPressEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    m_presenter->processKeyPressEvent(event);
  }
}

void ZStackMvc::processViewChange()
{
  processViewChange(getView()->getViewParameter(NeuTube::COORD_STACK));
}

void ZStackMvc::updateActiveViewData()
{
  processViewChangeCustom(getView()->getViewParameter(NeuTube::COORD_STACK));
}

void ZStackMvc::processViewChange(const ZStackViewParam &viewParam)
{
  processViewChangeCustom(viewParam);

  emit viewChanged();
}

QRect ZStackMvc::getViewGeometry() const
{
  QRect rect = getView()->geometry();
  rect.moveTo(getView()->mapToGlobal(rect.topLeft()));

  return rect;
}

QMainWindow* ZStackMvc::getMainWindow() const
{
  QMainWindow *mainwin = NULL;
  QObject *parentObject = parent();
  while (parentObject != NULL) {
    parentObject = parentObject->parent();
    mainwin = qobject_cast<QMainWindow*>(parentObject);
    if (mainwin != NULL) {
      break;
    }
    parentObject = parentObject->parent();
  }

  return mainwin;
}

void ZStackMvc::dump(const QString &msg)
{
  getView()->dump(msg);
}

void ZStackMvc::test()
{
  for (int i = 0; i < 5000; ++i) {
    QKeyEvent *event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_E, Qt::NoModifier);
    QCoreApplication::postEvent (this, event);
  }
}

/*
void ZStackMvc::emitMessage(const QString &msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  ZWidgetMessage message(msg, NeuTube::MSG_INFORMATION, target);
  emit messageGenerated(message);
}

void ZStackMvc::emitError(const QString &msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, NeuTube::MSG_ERROR, target));
}
*/

void ZStackMvc::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void ZStackMvc::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();

  //Filter out tiff files
  QList<QUrl> imageUrls;
  QList<QUrl> nonImageUrls;

  foreach (QUrl url, urls) {
    if (ZFileType::isImageFile(url.path().toStdString())) {
      imageUrls.append(url);
    } else {
      nonImageUrls.append(url);
    }
  }
  if (!nonImageUrls.isEmpty()) {
    getDocument()->loadFileList(nonImageUrls);
  }
}

#if 0
void ZStackMvc::focusInEvent(QFocusEvent *event)
{
#ifdef _DEBUG_
  std::cout << "MVC in focus" << std::endl;
  emit messageGenerated(
        ZWidgetMessage("MVC in focus", NeuTube::MSG_INFORMATION,
                       ZWidgetMessage::TARGET_STATUS_BAR));
#endif
}

void ZStackMvc::focusOutEvent(QFocusEvent *event)
{
#ifdef _DEBUG_
  std::cout << "MVC out focus" << std::endl;
  emit messageGenerated(
        ZWidgetMessage("MVC out focus", NeuTube::MSG_INFORMATION,
                       ZWidgetMessage::TARGET_STATUS_BAR));
#endif
}
#endif

#if 0
void ZStackMvc::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::ActivationChange) {
    if (isActiveWindow()) {
#ifdef _DEBUG_
      std::cout << "MVC in focus" << std::endl;
      emit messageGenerated(
            ZWidgetMessage("MVC in focus", NeuTube::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
#endif
    } else {
#ifdef _DEBUG_
      std::cout << "MVC out focus" << std::endl;
      emit messageGenerated(
            ZWidgetMessage("MVC out focus", NeuTube::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
#endif
    }
  }
}
#endif

void ZStackMvc::zoomWithWidthAligned(int x0, int x1, int cy)
{
//  getView()->blockSignals(true);
  getView()->zoomWithWidthAligned(x0, x1, cy);
//  getView()->blockSignals(false);

//  getView()->processViewChange(true, depthChanged);
}

void ZStackMvc::zoomWithWidthAligned(const QRect &viewPort, int z, double pw)
{
  getView()->zoomWithWidthAligned(
        viewPort.left(), viewPort.right(), pw, viewPort.center().y(), z);
}

void ZStackMvc::zoomWithHeightAligned(const ZStackView *view)
{
  ZIntPoint center = view->getViewCenter();

  center.shiftSliceAxis(getView()->getSliceAxis());

//  bool depthChanged = (center.getZ() == getView()->getCurrentZ());

  QRect refViewPort = view->getViewPort(NeuTube::COORD_STACK);

  zoomWithHeightAligned(refViewPort.top(), refViewPort.bottom(),
                        view->getProjRegion().height(),
                        center.getX(), center.getZ());
}

void ZStackMvc::zoomWithWidthAligned(const ZStackView *view)
{
  ZIntPoint center = view->getViewCenter();

  center.shiftSliceAxis(getView()->getSliceAxis());

//  bool depthChanged = (center.getZ() == getView()->getCurrentZ());

  QRect refViewPort = view->getViewPort(NeuTube::COORD_STACK);

  zoomWithWidthAligned(refViewPort.left(), refViewPort.right(),
                       view->getProjRegion().width(),
                       center.getY(), center.getZ());
}

void ZStackMvc::zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz)
{
  getView()->zoomWithHeightAligned(y0, y1, ph, cx, cz);
}

void ZStackMvc::zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz)
{
  getView()->zoomWithWidthAligned(x0, x1, pw, cy, cz);
}

void ZStackMvc::zoomTo(const ZIntPoint &pt, double zoomRatio)
{
  bool depthChanged = (pt.getZ() == getView()->getCurrentZ());

  getPresenter()->blockSignals(true);
  getPresenter()->setZoomRatio(zoomRatio);
  getPresenter()->blockSignals(false);

  getView()->blockSignals(true);
  getView()->setViewPortCenter(pt, NeuTube::AXIS_NORMAL);
  getView()->blockSignals(false);

  getView()->processViewChange(true, depthChanged);

//  getView()->notifyViewChanged();
}

void ZStackMvc::zoomTo(int x, int y, int z, int width)
{
  ZGeometry::shiftSliceAxis(x, y, z, getView()->getSliceAxis());

//  z -= getDocument()->getStackOffset().getSliceCoord(getView()->getSliceAxis());

  ZStackViewLocator locator;
  locator.setCanvasSize(getView()->imageWidget()->canvasSize().width(),
                        getView()->imageWidget()->canvasSize().height());

  QRect viewPort = locator.getRectViewPort(x, y, width);
  getPresenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));

  getView()->setViewPortCenter(x, y, z, NeuTube::AXIS_SHIFTED);

/*
  getView()->imageWidget()->setViewPortOffset(
        x - getView()->imageWidget()->viewPort().width() / 2,
        y - getView()->imageWidget()->viewPort().height() / 2);
  getView()->setSliceIndex(z);
  */
//  buddyView()->updateImageScreen(ZStackView::UPDATE_QUEUED);

//  getPresenter()->setViewPortCenter(x, y, z);

  getView()->highlightPosition(x, y, z);
}


void ZStackMvc::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZStackMvc::zoomTo(int x, int y, int z)
{
  QRect viewPort = getView()->getViewPort(NeuTube::COORD_STACK);
  int width = imin3(800, viewPort.width(), viewPort.height());
  if (width < 10) {
    width = 200;
  }
  zoomTo(x, y, z, width);
}

ZIntPoint ZStackMvc::getViewCenter() const
{
  return getView()->getViewCenter();
}

double ZStackMvc::getWidthZoomRatio() const
{
  return getView()->getCanvasWidthZoomRatio();
}

double ZStackMvc::getHeightZoomRatio() const
{
  return getView()->getCanvasHeightZoomRatio();
}

