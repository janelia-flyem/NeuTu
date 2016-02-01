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

  //connect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));

void ZStackMvc::connectSignalSlot()
{
  UPDATE_SIGNAL_SLOT(connect);
}

void ZStackMvc::disconnectAll()
{
  UPDATE_SIGNAL_SLOT(disconnect);
}

void ZStackMvc::dropDocument(ztr1::shared_ptr<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    if (m_doc.get() != NULL) {
      UPDATE_DOC_SIGNAL_SLOT(disconnect);
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
  UPDATE_DOC_SIGNAL_SLOT(connect);

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

void ZStackMvc::processViewChange(const ZStackViewParam &viewParam)
{
#if 0
  qDebug() << "ZStackMvc::processViewChange" << viewParam.getZ();
  if (getPresenter()->isObjectVisible()) {
    QList<ZDocPlayer*> playerList =
        m_doc->getPlayerList(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    qDebug() << "#Player:" << playerList.size();
    if (!playerList.isEmpty()) {
      bool updated = false;
      foreach (const ZDocPlayer *player, playerList) {
        if (player->getData()->isVisible()) {
          updated = true;
          player->updateData(viewParam);
        }
      }
      if (updated) {
        qDebug() << "Painting object in ZStackMvc::processViewChange";
        m_view->paintObject();
        m_view->imageWidget()->repaint();
      }
    }
  }
#endif

  processViewChangeCustom(viewParam);
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
