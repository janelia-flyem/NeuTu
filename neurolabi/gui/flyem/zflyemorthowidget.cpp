#include "zflyemorthowidget.h"

#include <QGridLayout>

#include "zsharedpointer.h"
#include "flyem/zflyemorthodoc.h"
#include "flyem/zflyemorthomvc.h"
#include "dvid/zdvidtarget.h"

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

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setHorizontalSpacing(0);
  layout->setVerticalSpacing(0);
}

ZFlyEmOrthoDoc* ZFlyEmOrthoWidget::getDocument() const
{
  return m_xyMvc->getCompleteDocument();
}
