#include "zdvidtileensemble.h"
#include <QRect>
#include <QElapsedTimer>
#include <QtCore>
#include <QMutexLocker>

#include "neutubeconfig.h"
#include "zstackview.h"
#include "dvid/zdvidreader.h"
#include "widgets/zimagewidget.h"
#include "flyem/zdvidtileupdatetaskmanager.h"
#include "flyem/zflyemmisc.h"
#include "zdvidutil.h"
#include "zdvidpatchdatafetcher.h"

ZDvidTileEnsemble::ZDvidTileEnsemble()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = GetType();
  m_highContrast = false;
  m_view = NULL;
  m_patch = NULL;
  m_dataFetcher = NULL;
//  m_patch = new ZImage(256, 256, QImage::Format_Indexed8);
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
  m_tileGroup.clear();

  delete m_patch;
  m_patch = NULL;
}

void ZDvidTileEnsemble::setDataFetcher(ZDvidPatchDataFetcher *fetcher)
{
  m_dataFetcher = fetcher;
}

void ZDvidTileEnsemble::enhanceContrast(bool high)
{
  m_highContrast = high;
  updateContrast();
}

void ZDvidTileEnsemble::setContrastProtocal(const ZJsonObject &obj)
{
  m_contrastProtocal = obj;
}

void ZDvidTileEnsemble::updatePatch(
    const ZImage *patch, const ZIntCuboid &region)
{
  if (m_patch != NULL) {
    delete m_patch;
    m_patch = NULL;
  }

  m_patch = new ZImage(*patch);
  m_patch->loadHighContrastProtocal(m_contrastProtocal);
  m_patch->enhanceContrast(m_highContrast);
  m_patchRange = region;
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
//#if defined(_ENABLE_LIBDVIDCPP_)
#if defined(_SERVICE_ARRAY_)
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

//#define DVID_TILE_THREAD_FETCH 1

      std::vector<libdvid::BinaryDataPtr> data;
      std::string tileName;
      try {
//#if DVID_TILE_THREAD_FETCH
//        if (tile_locs_array.size() < 5) {
//          tileName = m_dvidTarget.getLosslessTileName();
//        } else {
          tileName = m_dvidTarget.getMultiscale2dName();
//        }

        if (NeutubeConfig::ParallelTileFetching()) {
          data = get_tile_array_binary(
                *(m_reader.getService()), tileName,
                libdvid::XY, resLevel, tile_locs_array);
        } else {
          //#else
          data.resize(tile_locs_array.size());
          //        std::vector<libdvid::BinaryDataPtr> data(tile_locs_array.size());
          for (size_t i = 0; i < tile_locs_array.size(); ++i) {
            data[i] = m_reader.getService()->get_tile_slice_binary(
                  tileName, libdvid::XY, resLevel, tile_locs_array[i]);
          }
        }
//#endif
      } catch (libdvid::DVIDException &e) {
        LWARN() << e.what();
      }

      qint64 tileReadingTime = timer.elapsed();

      if (NeutubeConfig::LoggingProfile()) {
        LINFO() << data.size() << "x tile reading time: " << tileReadingTime;
      } else {
        std::cout << data.size() << "x tile reading time: " << tileReadingTime
                  << std::endl;
      }
      if (tileReadingTime > 3000) {
        LWARN() << "Tile reading hickup.";
      }

      size_t dataIndex = 0;

      QList<ZDvidTile*> tileList;
      QList<ZDvidTileDecodeTask*> taskList;
      for (std::vector<ZDvidTileInfo::TIndex>::const_iterator
           iter = tileIndices.begin();
           iter != tileIndices.end(); ++iter) {
        const ZDvidTileInfo::TIndex &index = *iter;
        ZDvidTile *tile = const_cast<ZDvidTileEnsemble*>(this)->getTile(
              resLevel, index);
        if (tile != NULL) {
          libdvid::BinaryDataPtr dataPtr= data[dataIndex++];

          ZDvidTileDecodeTask *task = new ZDvidTileDecodeTask(NULL, tile);
          task->setZ(z);
          if (dataPtr.get() != NULL) {
            task->setData(dataPtr->get_raw(), dataPtr->length());
            task->setHighContrast(m_highContrast);
            taskList.append(task);
          }
          tile->setContrastProtocal(m_contrastProtocal);
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

      if (m_dataFetcher != NULL && m_dvidTarget.isTileLowQuality()) {
        QRect highresViewPort =
            m_view->getViewParameter(NeuTube::COORD_STACK).getViewPort();
        if (highresViewPort.width() < 1024 || highresViewPort.height() < 1024) {
          int z = m_view->getZ(NeuTube::COORD_STACK);
          QPoint center = highresViewPort.center();
          int width = 512;
          int height = 512;
          int x0 = center.x() - width / 2 - 1;
          int y0 = center.y() - height / 2 - 1;
          int x1 = x0 + width;
          int y1 = y0 + height;

          ZIntCuboid region(x0, y0, z, x1, y1, z);
          m_dataFetcher->submit(region);
        }
      }

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

  QMutexLocker locker(&m_updateMutex);

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





  int resLevel = std::min(m_tilingInfo.getMaxLevel(), level);

  std::vector<ZDvidTileInfo::TIndex> tileIndices =
      m_tilingInfo.getCoverIndex(resLevel, fov);

  if (tileIndices.size() > 16) {
    if (resLevel < m_tilingInfo.getMaxLevel()) {
      ++resLevel;
      tileIndices = m_tilingInfo.getCoverIndex(resLevel, fov);
    }
  }

  QRect highresViewPort =
      m_view->getViewParameter(NeuTube::COORD_STACK).getViewPort();
//  if (highresViewPort.width() > 256 || highresViewPort.height() > 256) {
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
//  } else {
#if 0
  if (highresViewPort.width() < 512 || highresViewPort.height() < 512) {
    QElapsedTimer timer;
    timer.start();

    QPoint center = highresViewPort.center();
    int x0 = center.x() - 128;
    int y0 = center.y() - 128;
//    int x0 = highresViewPort.left();
//    int y0 = highresViewPort.top();
    int width = 256;
    int height = 256;
    int z = m_view->getZ(NeuTube::COORD_STACK);

    ZDvidBufferReader bufferReader;
    ZDvidUrl url(getDvidTarget());
    bufferReader.read(url.getGrayscaleUrl(width, height, x0, y0, z).c_str());

    std::cout << "High-res reading time: " << timer.elapsed() << std::endl;


    if (m_patch != NULL) {
      if (m_patch->width() != width || m_patch->height() != height) {
        delete m_patch;
        m_patch = NULL;
      }
    }

    if (m_patch == NULL) {
      m_patch = new ZImage(width, height, QImage::Format_Indexed8);
    }
    if (m_patch->loadFromData(bufferReader.getBuffer(), "png")) {
      m_patch->loadHighContrastProtocal(m_contrastProtocal);
      m_patch->enhanceContrast(m_highContrast);
      painter.drawImage(x0, y0, *m_patch);
    }
    std::cout << "High-res patching time: " << timer.elapsed() << std::endl;
  }
#endif

  if (m_patch != NULL) {
    if (m_patchRange.getFirstCorner().getZ() == painter.getZOffset() + slice) {
      painter.drawImage(m_patchRange.getFirstCorner().getX(),
                        m_patchRange.getFirstCorner().getY(),
                        *m_patch);
    }
  }
//  std::cout << "Draw image time: " << toc() << std::endl;
}

void ZDvidTileEnsemble::setDvidTarget(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = dvidTarget;
  if (m_reader.open(dvidTarget)) {
    m_tilingInfo = m_reader.readTileInfo(dvidTarget.getMultiscale2dName());

    ZJsonObject obj = m_reader.readContrastProtocal();
    setContrastProtocal(obj);

#if defined(_ENABLE_LIBDVIDCPP_) && defined(_SERVICE_ARRAY_)
    m_serviceArray.resize(36);
    try {
      std::vector<ZSharedPointer<libdvid::DVIDNodeService> >::iterator
          iter = m_serviceArray.begin();
      *iter = ZDvid::MakeDvidNodeService(m_reader.getDvidTarget());
      libdvid::DVIDNodeService *firstService = (*iter).get();
      for (++iter; iter != m_serviceArray.end(); ++iter) {
        *iter = ZDvid::MakeDvidNodeService(firstService);
      }
    } catch (libdvid::DVIDException &e) {
      LWARN() << e.what();
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
