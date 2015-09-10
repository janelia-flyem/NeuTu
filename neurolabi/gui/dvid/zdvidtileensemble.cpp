#include "zdvidtileensemble.h"
#include <QRect>
#include "zstackview.h"
#include "dvid/zdvidreader.h"
#include "widgets/zimagewidget.h"
#include "flyem/zdvidtileupdatetaskmanager.h"

#if _ENABLE_LIBDVIDCPP_
#include "libdvid/DVIDThreadedFetch.h"
#endif

ZDvidTileEnsemble::ZDvidTileEnsemble()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = ZStackObject::TYPE_DVID_TILE_ENSEMBLE;
  m_highContrast = false;
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

void ZDvidTileEnsemble::enhanceContrast(bool high)
{
  m_highContrast = high;
  updateContrast();
}

void ZDvidTileEnsemble::updateContrast()
{
  for (std::vector<std::map<ZDvidTileInfo::TIndex, ZDvidTile*> >::iterator
       iter = m_tileGroup.begin(); iter != m_tileGroup.end(); ++iter) {
    std::map<ZDvidTileInfo::TIndex, ZDvidTile*>& tileMap = *iter;
    for (std::map<ZDvidTileInfo::TIndex, ZDvidTile*>::iterator tileIter =
         tileMap.begin(); tileIter != tileMap.end(); ++tileIter) {
      ZDvidTile *tile = tileIter->second;
      if (tile != NULL) {
        if (m_highContrast) {
          tile->addVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
        } else {
          tile->removeVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
        }
      }
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

void ZDvidTileEnsemble::update(
    const std::vector<ZDvidTileInfo::TIndex>& tileIndices, int resLevel, int z)
{
#if defined(_ENABLE_LIBDVIDCPP_)
  std::vector<std::vector<int> > tile_locs_array;
  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      std::vector<int> loc(3);
      loc[0] = tile->getIx();
      loc[1] = tile->getIy();
      loc[2] = z;
      tile_locs_array.push_back(loc);
    }
  }

  try  {
    ZMultiTaskManager taskManager;
    if (!tile_locs_array.empty()) {
      libdvid::DVIDNodeService service(getDvidTarget().getAddressWithPort(),
                                       getDvidTarget().getUuid());
#ifdef _DEBUG_2
      std::cout << "Reading tiles ..." << std::endl;
#endif
      const std::vector<libdvid::BinaryDataPtr> &data = get_tile_array_binary(
            service, m_dvidTarget.getMultiscale2dName(), libdvid::XY, resLevel,
            tile_locs_array);

      std::cout << "tile count" << tile_locs_array.size() << std::endl;
      std::cout << "tile read:" << data.size() << std::endl;

      size_t dataIndex = 0;

//      QThreadFutureMap futureMap;
      QList<QFuture<void> > futureList;

      for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
           iter != tileIndices.end(); ++iter) {
        const ZDvidTileInfo::TIndex &index = *iter;
        ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(
              resLevel, index);
        if (tile != NULL) {
          libdvid::BinaryDataPtr dataPtr= data[dataIndex++];

          ZDvidTileDecodeTask *task = new ZDvidTileDecodeTask(NULL, tile);
          task->setZ(z);
          std::cout << "data length" << dataPtr->length() << std::endl;
          task->setData(dataPtr->get_raw(), dataPtr->length());
          task->setHighContrast(m_highContrast);
          task->execute();
//          taskManager.addTask(task);

//          QFuture<void> future = QtConcurrent::run(task, &ZDvidTileDecodeTask::execute);
//          futureList.append(future);
            //      tile->display(painter, slice, option);

//          tile->loadDvidSlice(dataPtr->get_raw(), dataPtr->length(), z);
          /*
          tile->setImageData(
                data[dataIndex++]->get_raw(), m_tilingInfo.getWidth(),
              m_tilingInfo.getHeight());
          tile->setZ(z);
          */
        }
      }

      for (QList<QFuture<void> >::iterator iter = futureList.begin();
           iter != futureList.end(); ++iter) {
        iter->waitForFinished();
      }


//      taskManager.start();
//      taskManager.waitForDone();
    }
#ifdef _DEBUG_2
      std::cout << "Tiles ready." << std::endl;
#endif
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
#else

  //  tic();
    std::cout << "reading tiles" << std::endl;
  ZMultiTaskManager taskManager;
  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      ZDvidTileUpdateTask *task = new ZDvidTileUpdateTask(NULL, tile);
      task->setZ(z);
      taskManager.addTask(task);
      //      tile->display(painter, slice, option);
    }
  }
  taskManager.start();
  taskManager.waitForDone();
  taskManager.clear();
  std::cout << "reading done" << std::endl;
#endif
}

void ZDvidTileEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
    std::cout << "displaying tiles" << std::endl;

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

  if (tileIndices.size() > 16) {
    if (resLevel < m_tilingInfo.getMaxLevel()) {
      ++resLevel;
      tileIndices = m_tilingInfo.getCoverIndex(resLevel, fov);
    }
  }

   std::cout << "updating tiles" << std::endl;

  const_cast<ZDvidTileEnsemble&>(*this).update(tileIndices, resLevel, painter.getZ(slice));

//  const_cast<ZDvidTileEnsemble&>(*this).updateContrast();

  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      tile->enhanceContrast(m_highContrast);
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

bool ZDvidTileEnsemble::isEmpty() const
{
  return m_tileGroup.empty();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidTileEnsemble)
