#include "zflyemorthowidget.h"

#include <QGridLayout>

#include "zsharedpointer.h"
#include "flyem/zflyemorthodoc.h"
#include "flyem/zflyemorthomvc.h"
#include "dvid/zdvidtarget.h"
#include "flyem/flyemorthocontrolform.h"
#include "zstackview.h"
#include "zstackpresenter.h"
#include "zwidgetmessage.h"

ZFlyEmOrthoWidget::ZFlyEmOrthoWidget(const ZDvidTarget &target, QWidget *parent) :
  QWidget(parent)
{
  init(target);
}

void ZFlyEmOrthoWidget::init(const ZDvidTarget &target)
{
  QGridLayout *layout = new QGridLayout(this);
  setLayout(layout);

  ZSharedPointer<ZFlyEmOrthoDoc> sharedDoc =
      ZSharedPointer<ZFlyEmOrthoDoc>(new ZFlyEmOrthoDoc);
  sharedDoc->setDvidTarget(target);

  m_xyMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, NeuTube::Z_AXIS);
//  xyWidget->setDvidTarget(target);
//  m_xyMvc->getCompleteDocument()->updateStack(ZIntPoint(4085, 5300, 7329));

  m_yzMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, NeuTube::X_AXIS);
//  yzWidget->setDvidTarget(target);

  m_xzMvc = ZFlyEmOrthoMvc::Make(this, sharedDoc, NeuTube::Y_AXIS);
//  xzWidget->setDvidTarget(target);


  layout->addWidget(m_xyMvc, 0, 0);
  layout->addWidget(m_yzMvc, 0, 1);
  layout->addWidget(m_xzMvc, 1, 0);

  m_controlForm = new FlyEmOrthoControlForm(this);
  layout->addWidget(m_controlForm);


  layout->setContentsMargins(0, 0, 0, 0);
  layout->setHorizontalSpacing(0);
  layout->setVerticalSpacing(0);

  connectSignalSlot();
}

void ZFlyEmOrthoWidget::syncView()
{
  QObject *obj = sender();
  syncViewWith(qobject_cast<ZFlyEmOrthoMvc*>(obj));
}

ZFlyEmOrthoDoc* ZFlyEmOrthoWidget::getDocument() const
{
  return m_xyMvc->getCompleteDocument();
}

void ZFlyEmOrthoWidget::connectSignalSlot()
{
  connect(m_xyMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  connect(m_yzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  connect(m_xzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));

  ZWidgetMessage::ConnectMessagePipe(getDocument(), this);
  ZWidgetMessage::ConnectMessagePipe(m_xyMvc->getMergeProject(), this);
  ZWidgetMessage::ConnectMessagePipe(m_yzMvc->getMergeProject(), this);
  ZWidgetMessage::ConnectMessagePipe(m_xzMvc->getMergeProject(), this);

  connect(m_controlForm, SIGNAL(movingUp()), this, SLOT(moveUp()));
  connect(m_controlForm, SIGNAL(movingDown()), this, SLOT(moveDown()));
  connect(m_controlForm, SIGNAL(movingLeft()), this, SLOT(moveLeft()));
  connect(m_controlForm, SIGNAL(movingRight()), this, SLOT(moveRight()));
  connect(m_controlForm, SIGNAL(locatingMain()),
          this, SLOT(locateMainWindow()));

  connect(getDocument(), SIGNAL(bookmarkEdited(int,int,int)),
          this, SIGNAL(bookmarkEdited(int,int,int)));
  connect(getDocument(), SIGNAL(synapseEdited(int,int,int)),
          this, SIGNAL(synapseEdited(int,int,int)));
  connect(getDocument(), SIGNAL(bodyMergeEdited()),
          this, SIGNAL(bodyMergeEdited()));

  connect(m_xyMvc->getPresenter(), SIGNAL(orthoViewTriggered(double,double,double)),
          this, SLOT(moveTo(double, double, double)));
  connect(m_xzMvc->getPresenter(), SIGNAL(orthoViewTriggered(double,double,double)),
          this, SLOT(moveTo(double, double, double)));
  connect(m_yzMvc->getPresenter(), SIGNAL(orthoViewTriggered(double,double,double)),
          this, SLOT(moveTo(double, double, double)));
}

void ZFlyEmOrthoWidget::syncMergeWithDvid()
{
  m_xyMvc->syncMergeWithDvid();
  m_xzMvc->syncMergeWithDvid();
  m_yzMvc->syncMergeWithDvid();
}

void ZFlyEmOrthoWidget::moveTo(double x, double y, double z)
{
  moveTo(ZPoint(x, y, z).toIntPoint());
}

void ZFlyEmOrthoWidget::moveTo(const ZIntPoint &center)
{
  getDocument()->updateStack(center);
  m_xyMvc->getView()->updateViewBox();
//  m_yzMvc->getView()->updateViewBox();
//  m_xzMvc->getView()->updateViewBox();
}

void ZFlyEmOrthoWidget::moveUp()
{
  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setY(newCenter.getY() - currentBox.getHeight() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveDown()
{
  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setY(newCenter.getY() + currentBox.getHeight() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveLeft()
{
  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setX(newCenter.getX() - currentBox.getWidth() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::moveRight()
{
  ZIntCuboid currentBox = getDocument()->getStack()->getBoundBox();
  ZIntPoint newCenter = currentBox.getCenter();
  newCenter.setX(newCenter.getX() + currentBox.getWidth() / 2);

  moveTo(newCenter);
}

void ZFlyEmOrthoWidget::locateMainWindow()
{
  ZIntPoint center = m_xyMvc->getViewCenter();
  emit zoomingTo(center.getX(), center.getY(), center.getZ());
}

void ZFlyEmOrthoWidget::processMessage(const ZWidgetMessage &message)
{
  switch (message.getTarget()) {
  case ZWidgetMessage::TARGET_TEXT:
  case ZWidgetMessage::TARGET_TEXT_APPENDING:
    m_controlForm->dump(message);
    break;
  default:
    break;
  }
}

void ZFlyEmOrthoWidget::syncViewWith(ZFlyEmOrthoMvc *mvc)
{
  disconnect(m_xyMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  disconnect(m_yzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  disconnect(m_xzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));

  switch (mvc->getView()->getSliceAxis()) {
  case NeuTube::Z_AXIS:
    m_yzMvc->zoomWithHeightAligned(mvc->getView());
//    m_yzMvc->zoomTo(mvc->getViewCenter(), mvc->getHeightZoomRatio());
    m_xzMvc->zoomWithWidthAligned(mvc->getView());
//    m_xzMvc->zoomTo(mvc->getViewCenter(), mvc->getWidthZoomRatio());
    break;
  case NeuTube::X_AXIS:
    m_xyMvc->zoomWithHeightAligned(mvc->getView());
    m_xzMvc->zoomWithWidthAligned(m_xyMvc->getView());
//    m_xyMvc->zoomTo(mvc->getViewCenter(), mvc->getHeightZoomRatio());
//    m_xzMvc->zoomTo(m_xyMvc->getViewCenter(), m_xyMvc->getWidthZoomRatio());
    break;
  case NeuTube::Y_AXIS:
    m_xyMvc->zoomWithWidthAligned(mvc->getView());
    m_yzMvc->zoomWithHeightAligned(m_xyMvc->getView());
//    m_xyMvc->zoomTo(mvc->getViewCenter(), mvc->getWidthZoomRatio());
//    m_yzMvc->zoomTo(m_xyMvc->getViewCenter(), m_xyMvc->getHeightZoomRatio());
    break;
  }

  connect(m_xyMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  connect(m_yzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
  connect(m_xzMvc, SIGNAL(viewChanged()), this, SLOT(syncView()));
}
