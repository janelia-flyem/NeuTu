#include "flyemtodoensemble.h"

#include "geometry/zaffinerect.h"
#include "data3d/displayconfig.h"

#include "flyemtodoblockgrid.h"
#include "flyemtodochunk.h"

FlyEmTodoEnsemble::FlyEmTodoEnsemble()
{
  setTarget(neutu::data3d::ETarget::HD_OBJECT_CANVAS);
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
#endif

void FlyEmTodoEnsemble::setSource(std::shared_ptr<FlyEmTodoSource> source)
{
  m_blockGrid->setSource(source);
}

bool FlyEmTodoEnsemble::display(
    QPainter *painter, const DisplayConfig &config) const
{
  bool painted = false;
  ZAffineRect rect;
  m_blockGrid->forEachIntersectedBlock(
        config.getCutRect(
          painter->device()->width(), painter->device()->height(),
          neutu::data3d::ESpace::CANVAS), [&](int i, int j, int k) {
    FlyEmTodoChunk chunk = m_blockGrid->getTodoChunk(i, j, k);
    chunk.forEachItem([&](const ZFlyEmToDoItem &item) {
      if (item.display(painter, config)) {
        painted = true;
      }
    });
  });

  return painted;
}
