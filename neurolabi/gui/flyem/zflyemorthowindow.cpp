#include "zflyemorthowindow.h"

#include "flyem/zflyemorthowidget.h"
#include "flyem/zflyemorthodoc.h"

ZFlyEmOrthoWindow::ZFlyEmOrthoWindow(const ZDvidTarget &target, QWidget *parent) :
  QMainWindow(parent)
{
  m_orthoWidget = new ZFlyEmOrthoWidget(target, this);
  setCentralWidget(m_orthoWidget);
}


void ZFlyEmOrthoWindow::updateData(const ZIntPoint &center)
{
  m_orthoWidget->moveTo(center);
}
