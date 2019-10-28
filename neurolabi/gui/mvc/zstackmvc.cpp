#include "zstackmvc.h"

#include <QMainWindow>
#include <QMimeData>
#include <QEvent>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QShortcut>

#include "neutubeconfig.h"
#include "common/math.h"
#include "logging/zlog.h"

//#include "zstackdoc.h"
#include "zstackview.h"
#include "zstackpresenter.h"
#include "zprogresssignal.h"
#include "zwidgetmessage.h"
#include "zfiletype.h"
#include "widgets/zimagewidget.h"
#include "geometry/zintpoint.h"
#include "zstackviewlocator.h"
#include "zdialogfactory.h"
#include "dialogs/zstresstestoptiondialog.h"
//#include "zstackdochelper.h"
#include "zstackdocutil.h"

ZStackMvc::ZStackMvc(QWidget *parent) :
  QWidget(parent)
{
  m_view = NULL;
  m_presenter = NULL;
  m_layout = new QHBoxLayout(this);
  m_progressSignal = new ZProgressSignal(this);
  setAcceptDrops(true);

//  qRegisterMetaType<ZWidgetMessage>("ZWidgetMessage");

  m_testTimer = new QTimer(this);
  m_role = ERole::ROLE_WIDGET;

#ifdef _DEBUG_2
  QShortcut *shortcut = new QShortcut(this);
//  shortcut->setKey(QKeySequence(Qt::Key_T, Qt::Key_R));
  shortcut->setKey(Qt::Key_G);
  shortcut->setContext(Qt::WindowShortcut);
//  shortcut->setEnabled(false);
  connect(shortcut, SIGNAL(activated()), this, SLOT(shortcutTest()));
#endif
}

ZStackMvc::~ZStackMvc()
{
  ZOUT(LTRACE(), 5) << "ZStackMvc destroyed";
}

ZStackMvc* ZStackMvc::Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, neutu::EAxis::Z);

  return frame;
}

ZStackMvc* ZStackMvc::Make(
    QWidget *parent, ZSharedPointer<ZStackDoc> doc, neutu::EAxis axis)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, axis);
//  frame->getView()->setSliceAxis(axis);
//  frame->getPresenter()->setSliceAxis(axis);

  return frame;
}

void ZStackMvc::construct(ZSharedPointer<ZStackDoc> doc, neutu::EAxis axis)
{
  dropDocument(ZSharedPointer<ZStackDoc>(doc));
  createView(axis);
  createPresenter(axis);
//  getPresenter()->createActions();

  updateDocument();

  m_view->prepareDocument();
//  m_presenter->createActions();
  m_presenter->prepareView();

  customInit();
}

void ZStackMvc::customInit()
{
}

void ZStackMvc::BaseConstruct(
    ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc, neutu::EAxis axis)
{
  frame->construct(doc, axis);
}

void ZStackMvc::createView()
{
  createView(neutu::EAxis::Z);
}

void ZStackMvc::createView(neutu::EAxis axis)
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

void ZStackMvc::createPresenter(neutu::EAxis axis)
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
                            const QObject *obj2, const char *slot,
                            Qt::ConnectionType connetionType)
{
  return connect(obj1, signal, obj2, slot, connetionType);
}

bool ZStackMvc::disconnectFunc(const QObject* obj1, const char *signal,
                               const QObject *obj2, const char *slot,
                               Qt::ConnectionType /*connetionType*/)
{
  return disconnect(obj1, signal, obj2, slot);
}

void ZStackMvc::connectSignalSlot()
{
  updateSignalSlot(connectFunc);
//  UPDATE_SIGNAL_SLOT(connect);
}

void ZStackMvc::updateDocSignalSlot(FConnectAction connectAction)
{
  connectAction(m_doc.get(), SIGNAL(stackRangeChanged()),
                m_view, SLOT(updateStackRange()), Qt::DirectConnection);
  connectAction(m_doc.get(), SIGNAL(stackModified(bool)),
                m_view, SLOT(processStackChange(bool)), Qt::QueuedConnection);
//  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObject::ETarget)),
//          m_view, SLOT(paintObject(ZStackObject::ETarget)), Qt::AutoConnection);
//  connectAction(m_doc.get(), SIGNAL(objectModified(QSet<ZStackObject::ETarget>)),
//                m_view, SLOT(paintObject(QSet<ZStackObject::ETarget>)),
//                Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(holdSegChanged()),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(
        m_doc.get(), SIGNAL(objectSelectionChanged(
                              QList<ZStackObject*>,QList<ZStackObject*>)),
        m_view, SLOT(paintObject(QList<ZStackObject*>,QList<ZStackObject*>)),
        Qt::AutoConnection);
  connectAction(m_doc.get(),
                SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(punctumVisibleStateChanged()),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(stackTargetModified()),
                m_view, SLOT(paintStack()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(zoomingTo(int, int, int)),
                this, SLOT(zoomTo(int,int,int)), Qt::AutoConnection);
  connectAction(m_view, SIGNAL(viewChanged(ZStackViewParam)),
          this, SLOT(processViewChange(ZStackViewParam)), Qt::AutoConnection);

  connectAction(m_doc.get(), SIGNAL(stackBoundBoxChanged()),
                m_view, SLOT(updateViewBox()), Qt::QueuedConnection);
  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObjectInfoSet)),
                m_presenter, SLOT(processObjectModified(ZStackObjectInfoSet)),
                Qt::AutoConnection);
}

void ZStackMvc::updateSignalSlot(FConnectAction connectAction)
{
  updateDocSignalSlot(connectAction);
//  connectAction(m_view, SIGNAL(currentSliceChanged(int)),
//                m_presenter, SLOT(processSliceChangeEvent(int)));
}

void ZStackMvc::disconnectAll()
{
  updateSignalSlot(disconnectFunc);
}

void ZStackMvc::dropDocument(ztr1::shared_ptr<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    if (m_doc.get() != NULL) {
      updateSignalSlot(disconnectFunc);
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

bool ZStackMvc::processKeyEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    return m_presenter->processKeyPressEvent(event);
  }

  return false;
}

void ZStackMvc::keyPressEvent(QKeyEvent *event)
{
  KINFO << QString("Key %1 pressed in ZStackMvc").arg(event->key());

  processKeyEvent(event);
}

bool ZStackMvc::event(QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    if (m_presenter != NULL) {
      if (m_presenter->processKeyPressEvent((QKeyEvent*) event)) {
        event->accept();
      }
    }
    return true;
  }

  return QWidget::event(event);
}

void ZStackMvc::processViewChange()
{
  processViewChange(getView()->getViewParameter(neutu::ECoordinateSystem::STACK));
}

/*
void ZStackMvc::updateActiveViewData()
{
  processViewChangeCustom(getView()->getViewParameter(neutube::COORD_STACK));
}
*/

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

void ZStackMvc::testSlot()
{
  for (int i = 0; i < 5000; ++i) {
    QKeyEvent *event = new QKeyEvent ( QEvent::KeyPress, Qt::Key_E, Qt::NoModifier);
    QCoreApplication::postEvent (this, event);
  }
}

void ZStackMvc::setStressTestEnv(ZStressTestOptionDialog *optionDlg)
{
  assert(optionDlg != NULL);

  m_testTimer->setInterval(1000);
  disconnect(m_testTimer, SIGNAL(timeout()), this, 0);
  prepareStressTestEnv(optionDlg);
}

void ZStackMvc::prepareStressTestEnv(ZStressTestOptionDialog *optionDlg)
{
  switch (optionDlg->getOption()) {
  case ZStressTestOptionDialog::OPTION_CUSTOM:
    connect(m_testTimer, SIGNAL(timeout()), this, SLOT(testSlot()));
    break;
  default:
    break;
  }
}

void ZStackMvc::stressTest(ZStressTestOptionDialog *dlg)
{
  setStressTestEnv(dlg);
  toggleStressTest();
}

ZStackMvc::ERole ZStackMvc::getRole() const
{
  return m_role;
}

void ZStackMvc::setRole(ZStackMvc::ERole role)
{
  m_role = role;
}


void ZStackMvc::toggleStressTest()
{
  if (m_testTimer->isActive()) {
    m_testTimer->stop();
    LINFO() << "Stress test started";
  } else {
    m_testTimer->start();
    LINFO() << "Stress test stopped";
  }
}

/*
void ZStackMvc::emitMessage(const QString &msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  ZWidgetMessage message(msg, neutube::EMessageType::MSG_INFORMATION, target);
  emit messageGenerated(message);
}

void ZStackMvc::emitError(const QString &msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutube::EMessageType::MSG_ERROR, target));
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
    if (ZFileType::isImageFile(neutu::GetFilePath(url).toStdString())) {
      imageUrls.append(url);
    } else {
      nonImageUrls.append(url);
    }
  }
  if (!nonImageUrls.isEmpty()) {
    getDocument()->loadFileList(nonImageUrls);
  }
}

void ZStackMvc::saveStack()
{
  if (getDocument()->hasStackData()) {
    QString filePath =
        ZDialogFactory::GetSaveFileName("Save Stack", ".tif", this);
    if (!filePath.isEmpty()) {
      std::string resultPath =
          ZStackDocUtil::SaveStack(getDocument().get(), filePath.toStdString());
      if (!resultPath.empty()) {
        LINFO() << "Stack saved at" << resultPath;
      }
    }
  }
}

#if 0
void ZStackMvc::focusInEvent(QFocusEvent *event)
{
#ifdef _DEBUG_
  std::cout << "MVC in focus" << std::endl;
  emit messageGenerated(
        ZWidgetMessage("MVC in focus", neutube::EMessageType::MSG_INFORMATION,
                       ZWidgetMessage::TARGET_STATUS_BAR));
#endif
}

void ZStackMvc::focusOutEvent(QFocusEvent *event)
{
#ifdef _DEBUG_
  std::cout << "MVC out focus" << std::endl;
  emit messageGenerated(
        ZWidgetMessage("MVC out focus", neutube::EMessageType::MSG_INFORMATION,
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
            ZWidgetMessage("MVC in focus", neutube::EMessageType::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
#endif
    } else {
#ifdef _DEBUG_
      std::cout << "MVC out focus" << std::endl;
      emit messageGenerated(
            ZWidgetMessage("MVC out focus", neutube::EMessageType::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
#endif
    }
  }
}
#endif

#if 0
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


void ZStackMvc::zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz)
{
  getView()->zoomWithHeightAligned(y0, y1, ph, cx, cz);
}

void ZStackMvc::zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz)
{
  getView()->zoomWithWidthAligned(x0, x1, pw, cy, cz);
}
#endif

void ZStackMvc::zoomWithWidthAligned(const ZStackView *view)
{
  ZStackViewParam param = view->getViewParameter();
  ZViewProj viewProj = param.getViewProj();

  double zoom = viewProj.getZoom();

  if (zoom > 0.0) {
    ZIntPoint center = view->getViewCenter();
    center.shiftSliceAxis(getView()->getSliceAxis());

    int x0 = viewProj.getX0();
    int cy = center.getZ();
    int y0 = cy - neutu::iround(
          double(getView()->getViewProj().getWidgetCenter().y()) / zoom);

    getView()->setViewProj(x0, y0, viewProj.getZoom());
  }

#if 0
  ZIntPoint center = view->getViewCenter();

  center.shiftSliceAxis(getView()->getSliceAxis());

//  bool depthChanged = (center.getZ() == getView()->getCurrentZ());

  QRect refViewPort = view->getViewPort(NeuTube::COORD_STACK);

  zoomWithWidthAligned(refViewPort.left(), refViewPort.right(),
                       view->getProjRegion().width(),
                       center.getY(), center.getZ());
#endif
}

void ZStackMvc::zoomWithHeightAligned(const ZStackView *view)
{
  ZStackViewParam param = view->getViewParameter();
  ZViewProj viewProj = param.getViewProj();

  double zoom = viewProj.getZoom();

  if (zoom > 0.0) {
    ZIntPoint center = view->getViewCenter();
    center.shiftSliceAxis(getView()->getSliceAxis());

    int y0 = viewProj.getY0();
    int cx = center.getZ();
    int x0 = cx - neutu::iround(
          double(getView()->getViewProj().getWidgetCenter().x()) / zoom);

    getView()->setViewProj(x0, y0, viewProj.getZoom());
  }

#if 0
  ZIntPoint center = view->getViewCenter();

//  center.shiftSliceAxis(getView()->getSliceAxis());

//  bool depthChanged = (center.getZ() == getView()->getCurrentZ());

  QRect refViewPort = view->getViewPort(NeuTube::COORD_STACK);

  getView()->setView();

  zoomWithHeightAligned(refViewPort.top(), refViewPort.bottom(),
                        view->getProjRegion().height(),
                        center.getX(), center.getZ());
#endif

}

void ZStackMvc::zoomTo(const ZIntPoint &pt, double zoomRatio)
{
  bool depthChanged = (pt.getZ() == getView()->getCurrentZ());

  getPresenter()->blockSignals(true);
  getPresenter()->setZoomRatio(zoomRatio);
  getPresenter()->blockSignals(false);

  getView()->blockSignals(true);
  getView()->setViewPortCenter(pt, neutu::EAxisSystem::NORMAL);
  getView()->blockSignals(false);

  getView()->processViewChange(true, depthChanged);

  getView()->highlightPosition(pt);

//  getView()->notifyViewChanged();
}

void ZStackMvc::zoomTo(const ZStackViewParam &param)
{
  getView()->setView(param);
  getView()->highlightPosition(param.getViewPort().center().x(),
                               param.getViewPort().center().y(),
                               param.getZ());
}

void ZStackMvc::zoomTo(int x, int y, int z, int width)
{
  getView()->zoomTo(x, y, z, width);
#if 0
  ZGeometry::shiftSliceAxis(x, y, z, getView()->getSliceAxis());

//  z -= getDocument()->getStackOffset().getSliceCoord(getView()->getSliceAxis());

  ZStackViewLocator locator;
  locator.setCanvasSize(getView()->imageWidget()->canvasSize().width(),
                        getView()->imageWidget()->canvasSize().height());

  QRect viewPort = locator.getRectViewPort(x, y, width);
  getPresenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));

  getView()->setViewPortCenter(x, y, z, NeuTube::AXIS_SHIFTED);
#endif

  getView()->highlightPosition(x, y, z);
}


void ZStackMvc::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZStackMvc::goToSlice(int z)
{
  getView()->setZ(z);
}

void ZStackMvc::stepSlice(int dz)
{
  getView()->stepSlice(dz);
}

void ZStackMvc::zoomTo(int x, int y, int z)
{
  zoomTo(x, y, z, 800);
  /*
  QRect viewPort = getView()->getViewPort(NeuTube::COORD_STACK);
  int width = imin3(800, viewPort.width(), viewPort.height());
  if (width < 10) {
    width = 200;
  }
  zoomTo(x, y, z, width);
  */
}

void ZStackMvc::zoomToL1(int x, int y, int z)
{
  zoomTo(x, y, z, 400);
  /*
  QRect viewPort = getView()->getViewPort(NeuTube::COORD_STACK);
  int width = imin3(400, viewPort.width(), viewPort.height());
  if (width < 10) {
    width = 200;
  }
  zoomTo(x, y, z, width);
  */
}

void ZStackMvc::setDefaultViewPort(const QRect &rect)
{
  getView()->setDefaultViewPort(rect);
}

QRect ZStackMvc::getViewPort() const
{
  return getView()->getViewPort(neutu::ECoordinateSystem::STACK);
}

ZIntPoint ZStackMvc::getViewCenter() const
{
  return getView()->getViewCenter();
}

QSize ZStackMvc::getViewScreenSize() const
{
  return getView()->getScreenSize();
}

double ZStackMvc::getWidthZoomRatio() const
{
  return getView()->getCanvasWidthZoomRatio();
}

double ZStackMvc::getHeightZoomRatio() const
{
  return getView()->getCanvasHeightZoomRatio();
}

void ZStackMvc::shortcutTest()
{
  std::cout << "Shortcut triggered: ZStackMvc::shortcutTest()" << std::endl;
}
