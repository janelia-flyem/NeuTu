#include "zflyemorthowidget.h"

#include <QGridLayout>
#include <QKeyEvent>

#include "common/zsharedpointer.h"
#include "logging/zlog.h"

#include "neutubeconfig.h"
#include "mvc/zstackdochelper.h"
#include "mvc/zstackview.h"
#include "mvc/zstackpresenter.h"
#include "zcrosshair.h"

#include "dvid/zdvidtarget.h"

#include "zwidgetmessage.h"
#include "widgets/zimagewidget.h"

#include "zflyemorthodoc.h"
#include "zflyemorthomvc.h"
#include "flyemorthocontrolform.h"
#include "zflyemorthoviewhelper.h"
#include "zflyemproofpresenter.h"


ZFlyEmOrthoWidget::ZFlyEmOrthoWidget(const ZDvidTarget &target, QWidget *parent) :
  QWidget(parent)
{
  init(target, 256, 256, 256);
}

ZFlyEmOrthoWidget::ZFlyEmOrthoWidget(
    const ZDvidTarget &target, int width, int height, int depth, QWidget *parent) :
  QWidget(parent)
{
  init(target, width, height, depth);
}

void ZFlyEmOrthoWidget::init(const ZDvidTarget &target,
                             int width, int height, int depth)
{
  QGridLayout *layout = new QGridLayout(this);
  setLayout(layout);

  ZSharedPointer<ZFlyEmOrthoDoc> sharedDoc =
      ZSharedPointer<ZFlyEmOrthoDoc>(new ZFlyEmOrthoDoc(width, height, depth));
  sharedDoc->setDvidTarget(target);

  m_xyMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, neutu::EAxis::Z);
  m_xyMvc->setDvidLabelSliceSize(width, height);
//  xyWidget->setDvidTarget(target);
//  m_xyMvc->getCompleteDocument()->updateStack(ZIntPoint(4085, 5300, 7329));

  m_yzMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, neutu::EAxis::X);
  m_yzMvc->setDvidLabelSliceSize(depth, height);
//  yzWidget->setDvidTarget(target);

  m_xzMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, neutu::EAxis::Y);
  m_xzMvc->setDvidLabelSliceSize(width, depth);
//  xzWidget->setDvidTarget(target);

  m_mvcArray.append(m_xyMvc);
  m_mvcArray.append(m_yzMvc);
  m_mvcArray.append(m_xzMvc);


  layout->addWidget(m_xyMvc, 0, 0);
  layout->addWidget(m_yzMvc, 0, 1);
  layout->addWidget(m_xzMvc, 1, 0);

  m_controlForm = new FlyEmOrthoControlForm(this);
  layout->addWidget(m_controlForm);


  layout->setContentsMargins(0, 0, 0, 0);
  layout->setHorizontalSpacing(0);
  layout->setVerticalSpacing(0);

  connectSignalSlot();

  setDataVisible(m_controlForm->isDataVisible());
  setSegmentationVisible(m_controlForm->isShowingSeg());

  syncMergeWithDvid();
}

void ZFlyEmOrthoWidget::syncView()
{
  QObject *obj = sender();
  syncViewWith(qobject_cast<ZFlyEmOrthoMvc*>(obj));
}

void ZFlyEmOrthoWidget::syncCrossHair()
{
  QObject *obj = sender();
  syncCrossHairWith(qobject_cast<ZFlyEmOrthoMvc*>(obj));
}

ZFlyEmOrthoDoc* ZFlyEmOrthoWidget::getDocument() const
{
  return m_xyMvc->getCompleteDocument();
}

void ZFlyEmOrthoWidget::connectSignalSlot()
{
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    connect(mvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
    connect(mvc, SIGNAL(crossHairChanged()), this, SLOT(syncCrossHair()));
  }

  ZWidgetMessage::ConnectMessagePipe(getDocument(), this);

//  ZWidgetMessage::ConnectMessagePipe(m_xyMvc->getMergeProject(), this);
//  ZWidgetMessage::ConnectMessagePipe(m_yzMvc->getMergeProject(), this);
//  ZWidgetMessage::ConnectMessagePipe(m_xzMvc->getMergeProject(), this);

  connect(m_controlForm, SIGNAL(movingUp()), this, SLOT(moveUp()));
  connect(m_controlForm, SIGNAL(movingDown()), this, SLOT(moveDown()));
  connect(m_controlForm, SIGNAL(movingLeft()), this, SLOT(moveLeft()));
  connect(m_controlForm, SIGNAL(movingRight()), this, SLOT(moveRight()));
  connect(m_controlForm, SIGNAL(locatingMain()),
          this, SLOT(locateMainWindow()));
  connect(m_controlForm, &FlyEmOrthoControlForm::resettingCrosshair,
          this, &ZFlyEmOrthoWidget::resetCrosshair);
  connect(m_controlForm, &FlyEmOrthoControlForm::reloading,
          this, &ZFlyEmOrthoWidget::reloadStack);
  connect(m_controlForm, SIGNAL(showingSeg(bool)),
          this, SLOT(setSegmentationVisible(bool)));
  connect(m_controlForm, SIGNAL(showingData(bool)),
          this, SLOT(setDataVisible(bool)));
  connect(m_controlForm, SIGNAL(settingHighContrast(bool)),
          this, SLOT(setHighContrast(bool)));
  connect(m_controlForm, SIGNAL(settingSmooth(bool)),
          this, SLOT(setSmoothDisplay(bool)));
  connect(m_controlForm, SIGNAL(showingCrosshair(bool)),
          this, SLOT(showCrosshair(bool)));

  connect(getDocument(), SIGNAL(bookmarkEdited(int,int,int)),
          this, SIGNAL(bookmarkEdited(int,int,int)));
  connect(getDocument(), SIGNAL(synapseEdited(int,int,int)),
          this, SIGNAL(synapseEdited(int,int,int)));
  connect(getDocument(), SIGNAL(synapseVerified(int,int,int,bool)),
          this, SIGNAL(synapseVerified(int,int,int,bool)));
  connect(getDocument(), SIGNAL(todoEdited(int,int,int)),
          this, SIGNAL(todoEdited(int,int,int)));
  connect(getDocument(), SIGNAL(bodyMergeEdited()),
          this, SLOT(notifyBodyMergeEdited()));

  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    connect(mvc->getPresenter(),
            SIGNAL(orthoViewTriggered(double,double,double)),
            this, SLOT(moveTo(double, double, double)));
    connect(mvc->getCompletePresenter(), SIGNAL(togglingSegmentation()),
            this, SLOT(toggleSegmentation()));
    connect(mvc->getCompletePresenter(), SIGNAL(togglingData()),
            this, SLOT(toggleData()));
    connect(mvc, SIGNAL(highlightModeChanged()),
            this, SLOT(syncHighlightMode()));
  }
}

void ZFlyEmOrthoWidget::notifyBodyMergeEdited()
{
  emit bodyMergeEdited();
}

void ZFlyEmOrthoWidget::syncMergeWithDvid()
{
  getDocument()->syncMergeWithDvid();
  /*
  foreach (ZFlyEmProofMvc *mvc, m_mvcArray) {
    mvc->syncMergeWithDvid();
  }
  */
#if 0
  m_xyMvc->syncMergeWithDvid();
  m_xzMvc->syncMergeWithDvid();
  m_yzMvc->syncMergeWithDvid();
#endif
}

void ZFlyEmOrthoWidget::moveTo(double x, double y, double z)
{
  moveTo(ZPoint(x, y, z).toIntPoint());
}

void ZFlyEmOrthoWidget::moveTo(const ZIntPoint &center)
{
  ZOUT(LTRACE(), 5) << "Proj region:"
                    << m_xyMvc->getView()->imageWidget()->projectRegion();
  getDocument()->updateStack(center);
  ZOUT(LTRACE(), 5) << "Proj region:"
                    << m_xyMvc->getView()->imageWidget()->projectRegion();
  m_xyMvc->getView()->updateViewBox();
  /*
  m_xyMvc->getPresenter()->optimizeStackBc();
  m_yzMvc->getPresenter()->setStackBc(m_xyMvc->getPresenter()->getGrayScale(),
                                      m_xyMvc->getPresenter()->getGrayOffset());
  m_xzMvc->getPresenter()->setStackBc(m_xyMvc->getPresenter()->getGrayScale(),
                                      m_xyMvc->getPresenter()->getGrayOffset());
                                      */

//  m_yzMvc->getView()->updateViewBox();
//  m_xzMvc->getView()->updateViewBox();
}

void ZFlyEmOrthoWidget::moveUp()
{
//  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntCuboid currentBox = ZStackDocHelper::GetDataSpaceRange(getDocument());
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setY(newCenter.getY() - currentBox.getHeight() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveDown()
{
//  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntCuboid currentBox = ZStackDocHelper::GetDataSpaceRange(getDocument());
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setY(newCenter.getY() + currentBox.getHeight() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveLeft()
{
//  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntCuboid currentBox = ZStackDocHelper::GetDataSpaceRange(getDocument());
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setX(newCenter.getX() - currentBox.getWidth() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveRight()
{
//  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntCuboid currentBox = ZStackDocHelper::GetDataSpaceRange(getDocument());
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setX(newCenter.getX() + currentBox.getWidth() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::locateMainWindow()
{
  ZIntPoint center = m_xyMvc->getViewCenter();
  emit zoomingTo(center.getX(), center.getY(), center.getZ());
}

void ZFlyEmOrthoWidget::reloadStack()
{
  KLOG << ZLog::Info()
       << ZLog::Description("orthogonal view: reload stack from crosshair")
       << ZLog::Window("ZFlyEmOrthoWidget");

  m_xyMvc->updateStackFromCrossHair();
  resetCrosshair();
}

void ZFlyEmOrthoWidget::resetCrosshair()
{
  ZIntPoint center;
  center.setX(m_xyMvc->getViewScreenSize().width() / 2);
  center.setY(m_xyMvc->getViewScreenSize().height() / 2);
  center.setZ(m_xzMvc->getViewScreenSize().height() / 2);

  getDocument()->setCrossHairCenter(center);
  m_xyMvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
  syncCrossHairWith(m_xyMvc);
}

void ZFlyEmOrthoWidget::processMessage(const ZWidgetMessage &message)
{
  if (message.hasTarget(ZWidgetMessage::TARGET_TEXT)) {
    m_controlForm->dump(message);
  }
  /*
  switch (message.getTarget()) {
  case ZWidgetMessage::TARGET_TEXT:
  case ZWidgetMessage::TARGET_TEXT_APPENDING:
    m_controlForm->dump(message);
    break;
  default:
    break;
  }
  */
}

void ZFlyEmOrthoWidget::setSegmentationVisible(bool on)
{
  m_xyMvc->setSegmentationVisible(on);
  m_yzMvc->setSegmentationVisible(on);
  m_xzMvc->setSegmentationVisible(on);
}

void ZFlyEmOrthoWidget::setDataVisible(bool on)
{
  m_xyMvc->showData(on);
  m_yzMvc->showData(on);
  m_xzMvc->showData(on);
}

void ZFlyEmOrthoWidget::setHighContrast(bool on)
{
  m_xyMvc->setHighContrast(on);
  m_yzMvc->setHighContrast(on);
  m_xzMvc->setHighContrast(on);
}

void ZFlyEmOrthoWidget::setSmoothDisplay(bool on)
{
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    mvc->smoothDisplay(on);
  }
}

void ZFlyEmOrthoWidget::showCrosshair(bool on)
{
  getDocument()->getCrossHair()->setVisible(on);
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    mvc->getView()->updateImageScreen(ZStackView::EUpdateOption::DIRECT);
  }
}

void ZFlyEmOrthoWidget::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
  case Qt::Key_D:
    if (event->modifiers() == Qt::ControlModifier) {
      LINFO() << "Ctrl+D pressed: Toggling data";
      toggleData();
    }
    break;
  }
}

void ZFlyEmOrthoWidget::toggleSegmentation()
{
  m_controlForm->toggleShowingSeg();
}

void ZFlyEmOrthoWidget::toggleData()
{
  m_controlForm->toggleData();
}

void ZFlyEmOrthoWidget::updateImageScreen()
{
  m_xyMvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
  m_yzMvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
  m_xzMvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
}

void ZFlyEmOrthoWidget::syncImageScreenWith(ZFlyEmOrthoMvc *mvc)
{
  foreach (ZFlyEmOrthoMvc *tmpMvc, m_mvcArray) {
    if (tmpMvc != mvc) {
      tmpMvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
    }
  }
}

void ZFlyEmOrthoWidget::syncHighlightModeWith(ZFlyEmOrthoMvc *mvc)
{
  foreach (ZFlyEmOrthoMvc *tmpMvc, m_mvcArray) {
    if (tmpMvc != mvc) {
      tmpMvc->highlightSelectedObject(
            mvc->getCompletePresenter()->isHighlight());
    }
  }
}

void ZFlyEmOrthoWidget::syncHighlightMode()
{
  QObject *obj = sender();
  syncHighlightModeWith(qobject_cast<ZFlyEmOrthoMvc*>(obj));
}

void ZFlyEmOrthoWidget::syncImageScreen()
{
  QObject *obj = sender();
  syncImageScreenWith(qobject_cast<ZFlyEmOrthoMvc*>(obj));
}

void ZFlyEmOrthoWidget::beginViewSync()
{
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    disconnect(mvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  }
}

void ZFlyEmOrthoWidget::endViewSync()
{
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    connect(mvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  }
}

void ZFlyEmOrthoWidget::beginCrossHairSync()
{
  beginViewSync();
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    disconnect(mvc, SIGNAL(crossHairChanged()), this, SLOT(syncCrossHair()));
  }
}

void ZFlyEmOrthoWidget::endCrossHairSync()
{
  endViewSync();
  foreach (ZFlyEmOrthoMvc *mvc, m_mvcArray) {
    connect(mvc, SIGNAL(crossHairChanged()), this, SLOT(syncCrossHair()));
  }
}

void ZFlyEmOrthoWidget::syncCrossHairWith(ZFlyEmOrthoMvc *mvc)
{
  if (mvc->getView()->getSliceAxis() == neutu::EAxis::ARB) {
    return;
  }

  beginCrossHairSync();

  ZFlyEmOrthoViewHelper helper;
  helper.attach(mvc);

  switch (mvc->getView()->getSliceAxis()) {
  case neutu::EAxis::Z:
    helper.syncCrossHair(m_yzMvc);
    helper.syncCrossHair(m_xzMvc);
    break;
  case neutu::EAxis::X:
    helper.syncCrossHair(m_xyMvc);
    helper.syncCrossHair(m_xzMvc);
    break;
  case neutu::EAxis::Y:
    helper.syncCrossHair(m_xyMvc);
    helper.syncCrossHair(m_yzMvc);
    break;
  case neutu::EAxis::ARB:
    break;
  }

  endCrossHairSync();
}

void ZFlyEmOrthoWidget::syncViewWith(ZFlyEmOrthoMvc *mvc)
{
  if (mvc->getView()->getSliceAxis() == neutu::EAxis::ARB) {
    return;
  }

  beginViewSync();

  ZFlyEmOrthoViewHelper helper;
  helper.attach(mvc);

  switch (mvc->getView()->getSliceAxis()) {
  case neutu::EAxis::Z:
    helper.syncViewPort(m_yzMvc);
    helper.syncViewPort(m_xzMvc);

//    m_yzMvc->zoomWithHeightAligned(mvc->getView());
//    m_xzMvc->zoomWithWidthAligned(mvc->getView());
    break;
  case neutu::EAxis::X:
    helper.syncViewPort(m_xyMvc);
    helper.syncViewPort(m_xzMvc);
//    m_xyMvc->zoomWithHeightAligned(mvc->getView());
//    m_xzMvc->zoomWithWidthAligned(m_xyMvc->getView());
    break;
  case neutu::EAxis::Y:
    helper.syncViewPort(m_xyMvc);
    helper.syncViewPort(m_yzMvc);
//    m_xyMvc->zoomWithWidthAligned(mvc->getView());
//    m_yzMvc->zoomWithHeightAligned(m_xyMvc->getView());
    break;
  case neutu::EAxis::ARB:
    break;
  }

  endViewSync();
}
