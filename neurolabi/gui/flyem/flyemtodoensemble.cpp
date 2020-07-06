#include "flyemtodoensemble.h"

#include "geometry/zaffinerect.h"
#include "data3d/displayconfig.h"

#include "flyemtodoblockgrid.h"
#include "flyemtodochunk.h"
#include "flyemtododvidsource.h"

FlyEmTodoEnsemble::FlyEmTodoEnsemble()
{
  m_type = GetType();
  setTarget(neutu::data3d::ETarget::NONBLOCKING_OBJECT_CANVAS);
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

/*
void FlyEmTodoEnsemble::setSource(std::shared_ptr<FlyEmTodoSource> source)
{
  m_blockGrid->setSource(source);
}
*/

bool FlyEmTodoEnsemble::display(
    QPainter *painter, const DisplayConfig &config) const
{
  bool painted = false;
  ZAffineRect rect;
  m_blockGrid->forEachIntersectedBlockApprox(
        config.getCutRect(
          painter->device()->width(), painter->device()->height(),
          neutu::data3d::ESpace::CANVAS), [&](int i, int j, int k) {
    m_blockGrid->forEachItemInChunk(i, j, k, [&](const ZFlyEmToDoItem &item) {
      if (item.display(painter, config)) {
        painted = true;
      }
    });
  }, 1.5);

  return painted;
}

void FlyEmTodoEnsemble::addItem(const ZFlyEmToDoItem &item)
{
  ZFlyEmToDoItem oldItem = m_blockGrid->getExistingItem(item.getPosition());
  m_blockGrid->addItem(item);

  if (oldItem.isValid()) {
    if (oldItem.isSelected() != item.isSelected()) {
      m_selector.removeObject(item.getPosition());
    }
  }

  if (item.isSelected()) {
    m_selector.setSelection(item.getPosition(), item.isSelected());
  }
}

bool FlyEmTodoEnsemble::hit(double x, double y, double z)
{
  if (isVisible() && isHittable()) {
    m_hitItem = m_blockGrid->pickClosestExistingItem(
          x, y, z, m_blockGrid->getBlockSize().toPoint().length() * 0.5);

    return m_hitItem.isValid();
  }

  return false;
}

void FlyEmTodoEnsemble::processDeselected()
{
  m_selector.forEachDeselected([&](const ZIntPoint &pt) {
    m_blockGrid->setExistingSelection(pt, false);
  });
  m_selector.clearDeselected();
  setSelected(m_selector.hasSelected());
}

void FlyEmTodoEnsemble::setSelectionAt(const ZIntPoint &pos, bool selecting)
{
  m_selector.setSelection(pos, selecting);
  processDeselected();
  m_blockGrid->setExistingSelection(pos, selecting);
}

void FlyEmTodoEnsemble::selectAt(const ZIntPoint &pos)
{
  setSelectionAt(pos, true);
}

void FlyEmTodoEnsemble::deselectAt(const ZIntPoint &pos)
{
  setSelectionAt(pos, false);
}

bool FlyEmTodoEnsemble::hasSelected() const
{
  return m_selector.hasSelected();
}

std::set<ZIntPoint> FlyEmTodoEnsemble::getSelectedPos() const
{
  return m_selector.getSelectedSet();
}

void FlyEmTodoEnsemble::processHit(ESelection s)
{
  ZIntPoint pos = m_hitItem.getPosition();
  switch (s) {
  case ESelection::SELECT_SINGLE:
    if (m_hitItem.isValid()) {
      m_selector.deselectAll();
      selectAt(pos);
    }
    break;
  case ESelection::SELECT_MULTIPLE:
    if (m_hitItem.isValid()) {
      selectAt(pos);
    }
    break;
  case ESelection::SELECT_TOGGLE:
    if (m_hitItem.isValid()) {
      setSelectionAt(pos, !m_selector.isInSelectedSet(pos));
    }
    break;
  case ESelection::DESELECT:
    m_selector.deselectAll();
    processDeselected();
    break;
  }

  m_hitItem.invalidate();
}

void FlyEmTodoEnsemble::deselect(bool)
{
  m_selector.deselectAll();
  processDeselected();
}

void FlyEmTodoEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  FlyEmTodoDvidSource *source = new FlyEmTodoDvidSource;
  source->setDvidTarget(target);
  m_blockGrid->setSource(std::shared_ptr<FlyEmTodoSource>(source));
}

void FlyEmTodoEnsemble::removeItem(const ZIntPoint &pos)
{
  m_blockGrid->removeItem(pos);
  m_selector.removeObject(pos);
}

int FlyEmTodoEnsemble::removeSelected(
    std::function<void(const std::string&)> errorProcessor)
{
  std::vector<ZIntPoint> removed;
  m_selector.forEachSelected([&](const ZIntPoint &pos) {
    try {
      m_blockGrid->removeItem(pos);
      removed.push_back(pos);
    } catch (std::exception &e) {
      errorProcessor(e.what());
    }
  });
  for (const ZIntPoint &pos : removed) {
    m_selector.removeObject(pos);
  }

  return removed.size();
}

void FlyEmTodoEnsemble::_setSource(std::shared_ptr<FlyEmTodoSource> source)
{
  m_blockGrid->setSource(source);
}

ZFlyEmToDoItem FlyEmTodoEnsemble::_getHitItem() const
{
  return m_hitItem;
}

std::shared_ptr<FlyEmTodoBlockGrid> FlyEmTodoEnsemble::_getBlockGrid() const
{
  return m_blockGrid;
}
