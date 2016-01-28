#include "zflyemorthodoc.h"

ZFlyEmOrthoDoc::ZFlyEmOrthoDoc(QObject *parent) :
  ZFlyEmProofDoc(parent)
{
  init();
}

void ZFlyEmOrthoDoc::init()
{
  m_width = 256;
  m_height = 256;
  m_depth = 256;
}

void ZFlyEmOrthoDoc::updateStack(const ZIntPoint &center)
{
  if (m_dvidReader.isReady()) {
    ZIntCuboid box;
    box.setFirstCorner(center - ZIntPoint(m_width / 2, m_height / 2, m_depth / 2));
    box.setSize(m_width, m_height, m_depth);
    m_dvidReader.readGrayScale(box);
    ZStack *stack = m_dvidReader.readGrayScale(box);
    loadStack(stack);
  }
}
