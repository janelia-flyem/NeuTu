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
  m_presenter = NULL;
  m_layout = new QVBoxLayout(this);
  m_topLayout = new QHBoxLayout();
  m_layout->addLayout(m_topLayout);
  m_viewLayout = new QGridLayout();
  m_layout->addLayout(m_viewLayout);
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

ZStackMvc* ZStackMvc::Make(QWidget *parent, std::shared_ptr<ZStackDoc> doc)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, neutu::EAxis::Z);

  return frame;
}

ZStackMvc* ZStackMvc::Make(
    QWidget *parent, std::shared_ptr<ZStackDoc> doc, neutu::EAxis axis)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, axis);
//  frame->getView()->setSliceAxis(axis);
//  frame->getPresenter()->setSliceAxis(axis);

  return frame;
}

ZStackMvc* ZStackMvc::Make(
    QWidget *parent, std::shared_ptr<ZStackDoc> doc,
    const std::vector<neutu::EAxis> &axes)
{
  ZStackMvc *frame = new ZStackMvc(parent);

  BaseConstruct(frame, doc, axes);

  return frame;
}


void ZStackMvc::construct(
    std::shared_ptr<ZStackDoc> doc,  const std::vector<neutu::EAxis> &axes)
{
  dropDocument(std::shared_ptr<ZStackDoc>(doc));

  if (axes.empty()) {
    createView(neutu::EAxis::Z);
    createPresenter(neutu::EAxis::Z, 1);
  } else {
    for (auto axis : axes) {
      createView(axis);
    }
    createPresenter(axes.front(), 1);
  }
//  getPresenter()->createActions();

//  layoutView();

  updateDocument();

  for (ZStackView *view : m_viewList) {
    view->prepareDocument();
  }
//  m_presenter->createActions();
  m_presenter->prepareView();

  forEachView([](ZStackView*view) {
    view->setHoverFocus(true);
  });

  customInit();
}

void ZStackMvc::construct(std::shared_ptr<ZStackDoc> doc, neutu::EAxis axis)
{
  construct(doc, std::vector<neutu::EAxis>(1, axis));
}

void ZStackMvc::customInit()
{
}

void ZStackMvc::BaseConstruct(
    ZStackMvc *frame, std::shared_ptr<ZStackDoc> doc, neutu::EAxis axis)
{
  frame->construct(doc, axis);
}

void ZStackMvc::BaseConstruct(
      ZStackMvc *frame, std::shared_ptr<ZStackDoc> doc,
      const std::vector<neutu::EAxis> &axes)
{
  frame->construct(doc, axes);
}

void ZStackMvc::createView()
{
  createView(neutu::EAxis::Z);
}

void ZStackMvc::createView(neutu::EAxis axis)
{
  if (m_doc.get() != NULL) {
    ZStackView *view = new ZStackView(qobject_cast<QWidget*>(this));
//    m_layout->addWidget(view, 0, 0);

    view->setSliceAxis(axis);

    m_viewList.push_back(view);
  }
}

void ZStackMvc::updateViewLayout()
{
  m_presenter->updateViewLayout();
}

void ZStackMvc::updateViewLayout(std::vector<int> viewLayoutIndices)
{
#ifdef _DEBUG_
  std::cout << "Update view layout: ";
  for (int index : viewLayoutIndices) {
    std::cout << index << ", ";
  }
  std::cout << std::endl;
#endif

  bool updating = true;
  if (viewLayoutIndices.size() == m_viewLayoutIndices.size()) {
    updating = !std::equal(
          viewLayoutIndices.begin(), viewLayoutIndices.end(),
          m_viewLayoutIndices.begin());
  }
  if (updating) {
    m_viewLayoutIndices = viewLayoutIndices;
    layoutView();
  }
}

void ZStackMvc::layoutView(ZStackView *view, int index)
{
  if (view) {
    if (index >= 0) {
      int row = index / 2;
      int column = index % 2;
      m_viewLayout->addWidget(view, row, column);
      if (view != getMainView()) {
//        view->setSliceViewTransform(getMainView()->getSliceViewTransform());
        view->setWidgetReady(true);
      }
      view->setVisible(true);
    } else {
      view->setVisible(false);
    }
  }
}

void ZStackMvc::layoutView()
{
  neutu::ClearLayout(m_viewLayout, nullptr);
//  int row = 0;
//  int column = 0;

  if (m_viewLayoutIndices.empty()) {
    for (size_t i = 0; i < m_viewList.size(); ++i) {
      layoutView(m_viewList[i], i);
    }
  } else {
    for (size_t i = 0; i< m_viewLayoutIndices.size(); ++i) {
      if (i < m_viewList.size()) {
        layoutView(m_viewList[i], m_viewLayoutIndices[i]);
      }
    }
    for (size_t i = m_viewLayoutIndices.size(); i < m_viewList.size(); ++i) {
      m_viewList[i]->hide();
    }
  }
  /*
  for (ZStackView *view : m_viewList) {
    m_viewLayout->addWidget(view, row, column++);
    if (column == 2) {
      column = 0;
      ++row;
    }
  }
  */
}

void ZStackMvc::updateViewData()
{
#ifdef _DEBUG_
  std::cout << "ZStackMvc::updateViewData" << std::endl;
#endif
  for (auto view : m_viewList) {
    if (view->isVisible()) {
      view->updateSceneWithViewData();
    }
  }
}

void ZStackMvc::createPresenter()
{
  if (m_doc.get() != NULL) {
    m_presenter = ZStackPresenter::Make(this);
  }
}

void ZStackMvc::createPresenter(neutu::EAxis axis, int viewCount)
{
  createPresenter();
  if (m_presenter != NULL) {
    m_presenter->setMainSliceAxis(axis);
    m_presenter->setViewCount(viewCount);
    connect(m_presenter, SIGNAL(actionTriggered(ZActionFactory::EAction)),
            this, SLOT(processAction(ZActionFactory::EAction)));
  }
}

void ZStackMvc::attachDocument(ZStackDoc *doc)
{
  attachDocument(std::shared_ptr<ZStackDoc>(doc));
}

void ZStackMvc::attachDocument(std::shared_ptr<ZStackDoc> doc)
{
  m_doc = doc;
}

ZStackView* ZStackMvc::getMainView() const
{
  if (!m_viewList.empty()) {
    return m_viewList.front();
  }

  return nullptr;
}

ZStackView* ZStackMvc::getDefaultView() const
{
  ZStackView *view = getMouseHoveredView();
  if (view == nullptr) {
    view = getMainView();
  }

  return view;
}

ZStackView* ZStackMvc::getView(int viewId) const
{
  if (viewId == 0) {
    return getMainView();
  } else if (viewId == -1) {
    return getDefaultView();
  } else if (viewId > 0) {
    for (auto view : m_viewList) {
      if (view->getViewId() == viewId) {
        return view;
      }
    }
  }

  return nullptr;
}

std::vector<ZStackView*> ZStackMvc::getViewList() const
{
  return m_viewList;
}

ZStackView* ZStackMvc::getMouseHoveredView() const
{
  for (ZStackView *view : m_viewList) {
    if (view->imageWidget()->containsCurrentMousePostion()) {
      return view;
    }
  }

  return nullptr;
}

void ZStackMvc::forEachView(std::function<void(ZStackView*)> f) const
{
  for (ZStackView *view : m_viewList) {
    f(view);
  }
}

void ZStackMvc::forEachVisibleView(std::function<void (ZStackView *)> f) const
{
  for (ZStackView *view : m_viewList) {
    if (view->isVisible()) {
      f(view);
    }
  }
}

void ZStackMvc::setViewReady()
{
  for (ZStackView *view : m_viewList) {
    view->setWidgetReady(true);
  }
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
  for (ZStackView *view : m_viewList) {
    connectAction(m_doc.get(), SIGNAL(stackRangeChanged()),
                  view, SLOT(updateStackRange()), Qt::DirectConnection);
    connectAction(m_doc.get(), SIGNAL(stackModified(bool)),
                  view, SLOT(processStackChange(bool)), Qt::AutoConnection);
    connectAction(m_doc.get(),
                  SIGNAL(objectSelectionChanged(
                           const ZStackObjectInfoSet&,const ZStackObjectInfoSet&)),
                  view,
                  SLOT(paintObject(
                         const ZStackObjectInfoSet&,const ZStackObjectInfoSet&)),
                  Qt::AutoConnection);
    connectAction(m_doc.get(), SIGNAL(stackTargetModified()),
                  view, SLOT(paintStack()), Qt::AutoConnection);
    connectAction(m_doc.get(), SIGNAL(stackBoundBoxChanged()),
                  view, SLOT(updateViewBox()), Qt::QueuedConnection);
  }

  connectAction(m_doc.get(), SIGNAL(zoomingTo(int, int, int)),
                this, SLOT(zoomTo(int,int,int)), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObjectInfoSet)),
                this, SLOT(processObjectModified(ZStackObjectInfoSet)),
                Qt::AutoConnection);
}

void ZStackMvc::updateSignalSlot(FConnectAction connectAction)
{
  updateDocSignalSlot(connectAction);

  for (ZStackView *view : m_viewList) {
    connectAction(view, SIGNAL(viewChanged(int)),
                  this, SLOT(processViewChange(int)), Qt::AutoConnection);
//    connectAction(view, SIGNAL(viewChanged(const ZStackViewParam &viewParam)),
//                  this, SLOT(processViewChange(const ZStackViewParam &viewParam)),
//                  Qt::AutoConnection);
  }
  connectAction(
        m_presenter, SIGNAL(updatingViewLayout(std::vector<int>)),
        this, SLOT(updateViewLayout(std::vector<int>)), Qt::AutoConnection);
  connectAction(
        m_presenter, SIGNAL(updatingViewData()),
        this, SLOT(updateViewData()), Qt::AutoConnection);
//  connectAction(m_view, SIGNAL(viewChanged(ZSliceViewTransform)),
//                m_presenter, SLOT(setSliceViewTransform(ZSliceViewTransform)),
//                Qt::AutoConnection);
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
  }
}

bool ZStackMvc::processKeyEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    return m_presenter->processKeyPressEvent(
          event, getDefaultView()->getViewId());
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
      if (m_presenter->processKeyPressEvent((QKeyEvent*) event, -1)) {
        event->accept();
      }
    }
    return true;
  }

  return QWidget::event(event);
}

/*
void ZStackMvc::processViewChange()
{
  processViewChange(getView()->getViewParameter(neutu::ECoordinateSystem::STACK));
}
*/

/*
void ZStackMvc::updateActiveViewData()
{
  processViewChangeCustom(getView()->getViewParameter(neutube::COORD_STACK));
}
*/

/*
void ZStackMvc::processViewChange(const ZStackViewParam &viewParam)
{
  processViewChangeCustom(viewParam);

  emit viewChanged();
}
*/

void ZStackMvc::blockViewChangeSignal(bool blocking)
{
  m_signalingViewChange = !blocking;
}

namespace {

ZPlane get_orthor_ort(const ZPlane &ort, int fromViewIndex, int toViewIndex)
{
  ZPlane newOrt = ort;
  if (fromViewIndex == 0) {
    if (toViewIndex == 1) {
      newOrt.set(ort.getNormal(), ort.getV2());
    } else if (toViewIndex == 2) {
      newOrt.set(ort.getV1(), ort.getNormal());
    }
  } else if (fromViewIndex == 1) {
    if (toViewIndex == 0) {
      newOrt.set(-ort.getNormal(), ort.getV2());
    } else if (toViewIndex == 2) {
      newOrt.set(-ort.getNormal(), ort.getV1());
    }
  } else if (fromViewIndex == 2) {
    if (toViewIndex == 0) {
      newOrt.set(ort.getV1(), -ort.getNormal());
    } else if (toViewIndex == 1) {
      newOrt.set(ort.getV2(), -ort.getNormal());
    }
  }

  return newOrt;
}

}

int ZStackMvc::getViewIndex(ZStackView *view)
{
  int index = -1;
  for (size_t i = 0; i< m_viewList.size(); ++i) {
    if (m_viewList[i] == view) {
      if (m_viewLayoutIndices.empty()) {
        index = i;
      } else {
        if (i < m_viewLayoutIndices.size()) {
          index = m_viewLayoutIndices[i];
        }
      }
      break;
    }
  }

  return index;
}

int ZStackMvc::getViewIndex(int viewId)
{
  int index = -1;
  for (size_t i = 0; i< m_viewList.size(); ++i) {
    if (m_viewList[i]->getViewId() == viewId) {
      if (m_viewLayoutIndices.empty()) {
        index = i;
      } else {
        if (index < int(m_viewLayoutIndices.size())) {
          index = m_viewLayoutIndices[i];
        }
      }
      break;
    }
  }

  return index;
}


void ZStackMvc::processViewChange(int viewId)
{
  if (m_signalingViewChange) {
    emit viewChanged();
  }

  ZSliceViewTransform transform = getView(viewId)->getSliceViewTransform();
  for (ZStackView *view : m_viewList) {
    if (view->getViewId() != viewId) {
      ZSliceViewTransform newTransform = view->getSliceViewTransform();
      newTransform.copyWithoutOrientation(transform);
//      newTransform.setCutPlane(view->getSliceViewTransform().getCutOrientation());
      if (transform.getSliceAxis() == neutu::EAxis::ARB) {
        newTransform.setCutPlane(
              get_orthor_ort(
                transform.getCutOrientation(),
                getViewIndex(viewId), getViewIndex(view)));
      }/* else {
        newTransform.setCutPlane(view->getSliceViewTransform().getCutOrientation());
      }
      */
//      newTransform.setRightHanded(view->getSliceViewTransform().rightHanded());
      view->enableViewChangeSignal(false);
      view->setSliceViewTransform(newTransform);
      view->enableViewChangeSignal(true);
    }
  }

  processViewChangeCustom(getMainView()->getViewParameter());
}

QRect ZStackMvc::getViewGeometry(int viewId) const
{
  ZStackView *view = getView(viewId);
  if (view) {
    QRect rect = view->geometry();
    rect.moveTo(view->mapToGlobal(rect.topLeft()));

    return rect;
  }

  return QRect();
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
  if (m_infoLabel) {
    m_infoLabel->setText(msg);
  }
//  getView()->dump(msg);
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
    if (ZFileType::IsImageFile(neutu::GetFilePath(url).toStdString())) {
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
    QString filePath = ZDialogFactory::GetSaveFileName(
          "Save Stack", "", "TIFF files (*.tif)", this);
    if (!filePath.isEmpty()) {
      std::string resultPath =
          ZStackDocUtil::SaveStack(getDocument().get(), filePath.toStdString());
      if (!resultPath.empty()) {
        LINFO() << "Stack saved at" << resultPath;
      }
    }
  }
}

void ZStackMvc::processObjectModified(const ZStackObjectInfoSet &objSet)
{
  getPresenter()->processObjectModified(objSet);
//  getView()->paintObject(objSet.getTarget());
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

void ZStackMvc::zoomWithWidthAligned(const ZStackView */*view*/)
{
  /*
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
  */

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

void ZStackMvc::zoomWithHeightAligned(const ZStackView */*view*/)
{
  /*
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
  */

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
  bool depthChanged = (pt.getZ() == getMainView()->getCurrentDepth());

  getPresenter()->blockSignals(true);
  getPresenter()->setZoomRatio(getMainView(), zoomRatio);
  getPresenter()->blockSignals(false);

  getMainView()->blockSignals(true);
  getMainView()->setViewPortCenter(pt, neutu::EAxisSystem::NORMAL);
  getMainView()->blockSignals(false);

  getMainView()->processViewChange(true, depthChanged);

  getMainView()->highlightPosition(pt);

//  getView()->notifyViewChanged();
}

/*
void ZStackMvc::zoomTo(const ZStackViewParam &param)
{
  getView()->setView(param);
  getView()->highlightPosition(param.getCutCenter().toIntPoint());
}
*/

void ZStackMvc::zoomTo(int x, int y, int z, int width)
{
  getMainView()->zoomTo(x, y, z, width);
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

  getMainView()->highlightPosition(x, y, z);
}


void ZStackMvc::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZStackMvc::goToSlice(int z)
{
  getMainView()->setDepth(z);
}

/*
void ZStackMvc::stepSlice(int dz)
{
  getView()->stepSlice(dz);
}
*/

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

neutu::EAxis ZStackMvc::getSliceAxis() const
{
  return getMainView()->getSliceAxis();
}

void ZStackMvc::setDefaultViewPort(const QRect &rect)
{
  getMainView()->setDefaultViewPort(rect);
}

ZAffineRect ZStackMvc::getViewPort() const
{
  return getMainView()->getViewPort();
}

ZIntPoint ZStackMvc::getViewCenter() const
{
  return getMainView()->getViewCenter();
}

QSize ZStackMvc::getViewScreenSize() const
{
  return getMainView()->getScreenSize();
}

void ZStackMvc::setZoomScale(double s)
{
  getMainView()->setZoomScale(s);
}

void ZStackMvc::setInitialScale(double s)
{
  getMainView()->setInitialScale(s);
}

void ZStackMvc::setCutPlane(const ZPoint &v1, const ZPoint &v2)
{
  getMainView()->setCutPlane(v1, v2);
}

void ZStackMvc::setCutPlane(const ZPlane &plane)
{
  getMainView()->setCutPlane(plane.getV1(), plane.getV2());
}

void ZStackMvc::setCutPlane(const ZAffinePlane &plane)
{
  getMainView()->setCutPlane(plane);
}

void ZStackMvc::setCutPlane(
    const ZPoint &center, const ZPoint &v1, const ZPoint &v2)
{
  setCutPlane(ZAffinePlane(center, v1, v2));
}

/*
void ZStackMvc::processViewChange(const ZStackViewParam &viewParam)
{
  for (ZStackView *view : m_viewList) {
    view->blockViewChangeEvent(true);
    if (view->getViewId() != viewParam.getViewId()) {
      view->setSliceViewTransform(viewParam.getSliceViewTransform());
    }
    view->blockViewChangeEvent(false);
  }
}
*/

/*
double ZStackMvc::getWidthZoomRatio() const
{
  return getView()->getCanvasWidthZoomRatio();
}
*/

/*
double ZStackMvc::getHeightZoomRatio() const
{
  return getView()->getCanvasHeightZoomRatio();
}
*/

void ZStackMvc::shortcutTest()
{
  std::cout << "Shortcut triggered: ZStackMvc::shortcutTest()" << std::endl;
}

void ZStackMvc::processAction(ZActionFactory::EAction action)
{
#ifdef _DEBUG_
  LINFO() << "Action triggerd: " << action;
#endif
}
