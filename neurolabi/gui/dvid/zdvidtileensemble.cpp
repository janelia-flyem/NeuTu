#include "zdvidtileensemble.h"
#include <QRect>
#include "zstackview.h"
#include "dvid/zdvidreader.h"
#include "zimagewidget.h"
#include "flyem/zdvidtileupdatetaskmanager.h"

ZDvidTileEnsemble::ZDvidTileEnsemble()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = ZStackObject::TYPE_DVID_TILE_ENSEMBLE;
}

ZDvidTileEnsemble::~ZDvidTileEnsemble()
{
  clear();
}

void ZDvidTileEnsemble::clear()
{
  for (std::vector<std::map<ZDvidTileInfo::TIndex, ZDvidTile*> >::iterator
       iter = m_tileGroup.begin(); iter != m_tileGroup.end(); ++iter) {
    for (std::map<ZDvidTileInfo::TIndex, ZDvidTile*>::iterator tileIter =
         iter->begin(); tileIter != iter->end(); ++tileIter) {
      delete tileIter->second;
    }
  }
}

ZDvidTile* ZDvidTileEnsemble::getTile(
    int resLevel, const ZDvidTileInfo::TIndex &index)
{
  if ((int) m_tileGroup.size() <= resLevel) {
    m_tileGroup.resize(resLevel + 1);
  }

  std::map<ZDvidTileInfo::TIndex, ZDvidTile*> &tileMap = m_tileGroup[resLevel];
  if (tileMap.count(index) == 0) {
    ZDvidTile *tile = new ZDvidTile;
    tile->setTarget(m_target);
    tile->setDvidTarget(m_dvidTarget);
    tile->setResolutionLevel(resLevel);
    tile->setTileIndex(index.first, index.second);
    tile->attachView(m_view);
    tileMap[index] = tile;
  }

  return tileMap[index];
}

void ZDvidTileEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (m_view == NULL) {
    return;
  }

  if (m_view->imageWidget() == NULL) {
    return;
  }

  QRect fov = m_view->imageWidget()->viewPort();
  QSize screenSize = m_view->imageWidget()->size();

  if (screenSize.width() == 0 || screenSize.height() == 0) {
    return;;
  }


  int zoomRatio = std::min(fov.width() / screenSize.width(),
                           fov.height() / screenSize.height());
  int level = 0;
  if (zoomRatio > 0) {
    ++level;
  }
  while ((zoomRatio /= 2) > 0) {
    ++level;
  }


  m_view->getViewParameter(NeuTube::COORD_STACK).getViewPort();
  int resLevel = std::min(m_tilingInfo.getMaxLevel(), level);

  std::vector<ZDvidTileInfo::TIndex> tileIndices =
      m_tilingInfo.getCoverIndex(resLevel, fov);

  if (tileIndices.size() > 8) {
    if (resLevel < m_tilingInfo.getMaxLevel()) {
      ++resLevel;
      tileIndices = m_tilingInfo.getCoverIndex(resLevel, fov);
    }
  }

//  tic();
  ZMultiTaskManager taskManager;
  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      ZDvidTileUpdateTask *task = new ZDvidTileUpdateTask(NULL, tile);
      task->setZ(painter.getZ(slice));
      taskManager.addTask(task);
//      tile->display(painter, slice, option);
    }
  }
  taskManager.start();
  taskManager.waitForDone();
  taskManager.clear();

  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      tile->display(painter, slice, option);
    }
  }
//  std::cout << "Draw image time: " << toc() << std::endl;
}

void ZDvidTileEnsemble::setDvidTarget(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = dvidTarget;
  ZDvidReader reader;
  if (reader.open(dvidTarget)) {
    m_tilingInfo = reader.readTileInfo(dvidTarget.getMultiscale2dName());
  }
}

void ZDvidTileEnsemble::attachView(ZStackView *view)
{
  m_view = view;
}

int ZDvidTileEnsemble::getCurrentZ() const
{
  int z = 0;
  if (m_view != NULL) {
    return m_view->getCurrentZ();
  }

  return z;
}

ZStackView* ZDvidTileEnsemble::getView() const
{
  return m_view;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidTileEnsemble)
