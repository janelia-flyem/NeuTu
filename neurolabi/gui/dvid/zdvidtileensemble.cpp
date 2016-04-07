#include "zdvidtileensemble.h"
#include <QRect>
#include <QElapsedTimer>
#include <QtCore>

#include "zstackview.h"
#include "dvid/zdvidreader.h"
#include "widgets/zimagewidget.h"
#include "flyem/zdvidtileupdatetaskmanager.h"

ZDvidTileEnsemble::ZDvidTileEnsemble()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = GetType();
  m_highContrast = false;
  m_view = NULL;
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

#if defined(_ENABLE_LIBDVIDCPP_)
    for (std::vector<libdvid::DVIDNodeService*>::iterator
         iter = m_serviceArray.begin();
         iter != m_serviceArray.end(); ++iter) {
      delete *iter;
    }
#endif
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
    tile->setDvidTarget(m_dvidTarget, m_tilingInfo);
    tile->setResolutionLevel(resLevel);
    tile->setTileIndex(index.first, index.second);
    tile->attachView(m_view);
    tileMap[index] = tile;
  }

  return tileMap[index];
}
#if defined(_ENABLE_LIBDVIDCPP_)
struct UpdateTileParam {
  ZDvidTileEnsemble *te;
//  libdvid::DVIDNodeService *service;
  libdvid::Slice2D slice;
  int resLevel;
  std::vector<int> loc;
  int z;
  ZDvidTile *tile;
  libdvid::DVIDNodeService *service;
};

void UpdateTileFunc(UpdateTileParam &param)
{
  param.te->updateTile(param.slice, param.resLevel, param.loc, param.z,
        param.tile, param.service);
}

void ZDvidTileEnsemble::updateTile(libdvid::Slice2D slice,
    int resLevel, const std::vector<int> &loc, int z, ZDvidTile *tile,
                                   libdvid::DVIDNodeService *service)
{
//  libdvid::DVIDNodeService service(getDvidTarget().getAddressWithPort(),
//                                   getDvidTarget().getUuid());
//  QElapsedTimer timer;
//  timer.start();
  libdvid::BinaryDataPtr dataPtr = service->get_tile_slice_binary(
        m_dvidTarget.getMultiscale2dName(), slice, resLevel, loc);
//  std::cout << "Single tile reading time: " << timer.elapsed() << std::endl;
  ZDvidTileDecodeTask *task = new ZDvidTileDecodeTask(NULL, tile);
  task->setZ(z);
  task->setData(dataPtr->get_raw(), dataPtr->length());
  task->setHighContrast(m_highContrast);
  task->execute();
  delete task;
}
#endif

bool ZDvidTileEnsemble::update(
    const std::vector<ZDvidTileInfo::TIndex>& tileIndices, int resLevel, int z)
{
  if (!m_reader.good()) {
    return false;
  }

  bool updated = false;
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
#if 0
    if (!tile_locs_array.empty()) {

      QVector<UpdateTileParam> paramList(tile_locs_array.size());
      size_t dataIndex = 0;
      std::vector<ZDvidTile*> updatedTileArray;
      for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
           iter != tileIndices.end(); ++iter) {
        const ZDvidTileInfo::TIndex &index = *iter;
        ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(
              resLevel, index);
        if (tile != NULL) {
          UpdateTileParam &param = paramList[dataIndex];
          param.te = this;
//          param.service = &service;
          param.resLevel = resLevel;
          param.z = z;
          param.tile = tile;
          param.slice = libdvid::XY;
          param.loc = tile_locs_array[dataIndex];
          param.service = m_serviceArray[dataIndex];
          dataIndex++;
          updatedTileArray.push_back(tile);
        }
      }

      std::cout << "Reading tiles ..." << std::endl;
      QElapsedTimer timer;
      timer.start();
      QtConcurrent::blockingMap(paramList, &UpdateTileFunc);
      qint64 tileReadingTime = timer.restart();

      std::cout << tile_locs_array.size() << "x tile reading time: "
                << tileReadingTime << std::endl;

      for (std::vector<ZDvidTile*>::iterator iter = updatedTileArray.begin();
           iter != updatedTileArray.end(); ++iter) {
        ZDvidTile *tile =*iter;
        tile->updatePixmap();
      }

      std::cout << "Pixmap preparing time: " << timer.elapsed() << std::endl;
    }
#else
//    ZMultiTaskManager taskManager;
    if (!tile_locs_array.empty()) {
      std::cout << "Reading tiles ..." << std::endl;
      QElapsedTimer timer;
      timer.start();
//      libdvid::DVIDNodeService service(getDvidTarget().getAddressWithPort(),
//                                       getDvidTarget().getUuid());
      std::cout << "Connecting time: " << timer.elapsed() << std::endl;

#define DVID_TILE_THREAD_FETCH 1

#if DVID_TILE_THREAD_FETCH
      std::vector<libdvid::BinaryDataPtr> data = get_tile_array_binary(
            *(m_reader.getService()), m_dvidTarget.getMultiscale2dName(),
            libdvid::XY, resLevel, tile_locs_array);
#else
      std::vector<libdvid::BinaryDataPtr> data(tile_locs_array.size());
      for (size_t i = 0; i < tile_locs_array.size(); ++i) {
        data[i] = m_reader.getService()->get_tile_slice_binary(
              m_dvidTarget.getMultiscale2dName(),
              libdvid::XY, resLevel, tile_locs_array[i]);
      }
#endif

      qint64 tileReadingTime = timer.elapsed();

      std::cout << data.size() << "x tile reading time: " << tileReadingTime << std::endl;
      if (tileReadingTime > 3000) {
        LWARN() << "Tile reading hickup.";
      }

      size_t dataIndex = 0;

      QList<ZDvidTile*> tileList;
      QList<ZDvidTileDecodeTask*> taskList;
      for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
           iter != tileIndices.end(); ++iter) {
        const ZDvidTileInfo::TIndex &index = *iter;
        ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(
              resLevel, index);
        if (tile != NULL) {
          libdvid::BinaryDataPtr dataPtr= data[dataIndex++];

          ZDvidTileDecodeTask *task = new ZDvidTileDecodeTask(NULL, tile);
          task->setZ(z);
          task->setData(dataPtr->get_raw(), dataPtr->length());
          task->setHighContrast(m_highContrast);
          taskList.append(task);
          tileList.append(tile);
        }
      }

      timer.start();
      QtConcurrent::blockingMap(taskList, &ZDvidTileDecodeTask::ExecuteTask);
      qint64 tileDecodingTime = timer.elapsed();
      std::cout <<"decoding time: " << tileDecodingTime << std::endl;
      if (tileDecodingTime > 3000) {
        LWARN() << "Tile decoding hickup.";
      }


#if 0
      for (QList<ZDvidTile*>::iterator iter = tileList.begin();
           iter != tileList.end(); ++iter) {
        ZDvidTile *tile = *iter;
        tile->updatePixmap();
      }
#endif

      for (QList<ZDvidTileDecodeTask*>::iterator iter = taskList.begin();
           iter != taskList.end(); ++iter) {
        delete *iter;
      }

      updated = true;

#if 0
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
          task->setData(dataPtr->get_raw(), dataPtr->length());
          task->setHighContrast(m_highContrast);
          taskManager.addTask(task);

          QFuture<void> future = QtConcurrent::run(task, &ZDvidTileDecodeTask::execute);
          futureList.append(future);
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

#endif
//      taskManager.start();
//      taskManager.waitForDone();
    }
#endif

#ifdef _DEBUG_2
      std::cout << "Tiles ready." << std::endl;
#endif
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
#else

  //  tic();
  ZMultiTaskManager taskManager;
  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
      ZDvidTileUpdateTask *task = new ZDvidTileUpdateTask(NULL, tile);
      task->setZ(z);
      task->execute();
//      taskManager.addTask(task);
      //      tile->display(painter, slice, option);
    }
  }
  taskManager.start();
  taskManager.waitForDone();
  taskManager.clear();
#endif

  return updated;
}

void ZDvidTileEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    NeuTube::EAxis sliceAxis) const
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
  if (zoomRatio > 1) {
    ++level;
  }
  while ((zoomRatio /= 2) > 1) {
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

  const_cast<ZDvidTileEnsemble&>(*this).update(
        tileIndices, resLevel, painter.getZ(slice));

//  const_cast<ZDvidTileEnsemble&>(*this).updateContrast();

  for (std::vector<ZDvidTileInfo::TIndex>::const_iterator iter = tileIndices.begin();
       iter != tileIndices.end(); ++iter) {
    const ZDvidTileInfo::TIndex &index = *iter;
    ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(resLevel, index);
    if (tile != NULL) {
//      tile->enhanceContrast(m_highContrast, true);
      tile->display(painter, slice, option, sliceAxis);
    }
  }
//  std::cout << "Draw image time: " << toc() << std::endl;
}

void ZDvidTileEnsemble::setDvidTarget(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = dvidTarget;
  if (m_reader.open(dvidTarget)) {
    m_tilingInfo = m_reader.readTileInfo(dvidTarget.getMultiscale2dName());

#if defined(_ENABLE_LIBDVIDCPP_)
    m_serviceArray.resize(36);
    for (std::vector<libdvid::DVIDNodeService*>::iterator
         iter = m_serviceArray.begin();
         iter != m_serviceArray.end(); ++iter) {
      *iter = new libdvid::DVIDNodeService(
            dvidTarget.getAddressWithPort(), dvidTarget.getUuid());
    }
#endif
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
