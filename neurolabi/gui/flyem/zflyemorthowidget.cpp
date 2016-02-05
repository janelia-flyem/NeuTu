#include "zflyemorthowidget.h"

#include <QGridLayout>

#include "zsharedpointer.h"
#include "flyem/zflyemorthodoc.h"
#include "flyem/zflyemorthomvc.h"
#include "dvid/zdvidtarget.h"
#include "flyem/flyemorthocontrolform.h"
#include "zstackview.h"

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

ZFlyEmOrthoDoc* ZFlyEmOrthoWidget::getDocument() const
{
  return m_xyMvc->getCompleteDocument();
}

void ZFlyEmOrthoWidget::connectSignalSlot()
{
  connect(m_controlForm, SIGNAL(movingUp()), this, SLOT(moveUp()));
  connect(m_controlForm, SIGNAL(movingDown()), this, SLOT(moveDown()));
  connect(m_controlForm, SIGNAL(movingLeft()), this, SLOT(moveLeft()));
  connect(m_controlForm, SIGNAL(movingRight()), this, SLOT(moveRight()));
}

void ZFlyEmOrthoWidget::moveTo(const ZIntPoint &center)
{
  getDocument()->updateStack(center);
  m_xyMvc->getView()->updateViewBox();
  m_yzMvc->getView()->updateViewBox();
  m_xzMvc->getView()->updateViewBox();
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
