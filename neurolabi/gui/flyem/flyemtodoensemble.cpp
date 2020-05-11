#include "flyemtodoensemble.h"

#include "geometry/zaffinerect.h"
#include "flyemtodoblockgrid.h"
#include "flyemtodochunk.h"

FlyEmTodoEnsemble::FlyEmTodoEnsemble()
{
  setTarget(ETarget::WIDGET_CANVAS);
  m_blockGrid = std::shared_ptr<FlyEmTodoBlockGrid>(new FlyEmTodoBlockGrid);
}

FlyEmTodoEnsemble::~FlyEmTodoEnsemble()
{

}

#if 0
void FlyEmTodoEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option, neutu::EAxis sliceAxis) const
{

}

void FlyEmTodoEnsemble::display(
    ZPainter &painter, const DisplayConfig &config) const
{
  if (config.sliceAxis == neutu::EAxis::ARB) {
    ZAffineRect rect;
    m_blockGrid->forEachIntersectedBlock(
          config.cutPlane, [&](int i, int j, int k) {
      FlyEmTodoChunk chunk = m_blockGrid->getTodoChunk(i, j, k);
      chunk.forEachItem([&](const ZFlyEmToDoItem &item) {
        item.display(painter, config);
      });
    });
  }
}
#endif
