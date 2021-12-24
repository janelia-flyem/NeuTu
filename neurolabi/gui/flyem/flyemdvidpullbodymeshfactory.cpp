#include "flyemdvidpullbodymeshfactory.h"

#include "common/debug.h"

#include "data3d/zstackobjecthelper.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"

#include "zflyembodymanager.h"

FlyEmDvidPullBodyMeshFactory::FlyEmDvidPullBodyMeshFactory()
{

}

FlyEmDvidPullBodyMeshFactory::~FlyEmDvidPullBodyMeshFactory()
{
  if (m_disposeReader) {
    m_disposeReader(m_reader);
  }
}

void FlyEmDvidPullBodyMeshFactory::setReader(
    ZDvidReader *reader, std::mutex *readerMutex, std::function<void(ZDvidReader*)> dispose)
{
  m_reader = reader;
  m_readerMutex = readerMutex;
  m_disposeReader = dispose;
}

void FlyEmDvidPullBodyMeshFactory::useNgMesh(bool on)
{
  m_usingNgMesh = on;
}

void FlyEmDvidPullBodyMeshFactory::useObjMesh(bool on)
{
  m_usingObjMesh = on;
}

FlyEmBodyMesh FlyEmDvidPullBodyMeshFactory::make_(const FlyEmBodyConfig &config)
{
  if (!m_reader || !config.getRange().isEmpty()) {
    return {nullptr, config};
  }

//  std::lock_guard<std::mutex> lock(*m_readerMutex);
  std::unique_lock<std::mutex> lock =
      m_readerMutex ? std::unique_lock<std::mutex>(*m_readerMutex)
                    : std::unique_lock<std::mutex>();
  int zoom = config.getDsLevel();
  ZMesh *mesh = nullptr;
  FlyEmBodyConfig actualConfig = config;

  if (config.getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
    if (!config.usingCoarseLevel()) {
      HLDEBUG("mesh factory") << "Downloading mesh for coarse supervoxel" << std::endl;
      mesh = m_reader->readSupervoxelMesh(
            ZFlyEmBodyManager::Decode(config.getBodyId()));
//      mesh = readSupervoxelMesh(reader, config.getBodyId());
      if (mesh != NULL) {
        HLDEBUG("mesh factory") << "Mesh downloaded" << std::endl;
        actualConfig.setDsLevel(0);
//        *acturalMeshZoom = 0;
      }
    }
  } else {
    if (!config.usingCoarseLevel() && m_usingNgMesh) { //Skip coarse Level
      //Checking order: merged mesh -> ngmesh -> normal mesh
      std::string mergeKey =
          ZDvidUrl::GetMeshKey(config.getBodyId(), ZDvidUrl::EMeshType::MERGED);
      HLDEBUG("mesh factory") << "Downloading merged mesh" << std::endl;
      mesh = m_reader->readMesh(mergeKey);
      if (mesh) {
        HLDEBUG("mesh factory") << "Mesh downloaded" << std::endl;
        actualConfig.setDsLevel(0);
//        *acturalMeshZoom = 0;
      } else {
        //Skip ngmesh if the merge key exists but is broken
        if (!m_reader->hasKey(
              m_reader->getDvidTarget().getMeshName().c_str(), mergeKey.c_str())) {
          HLDEBUG("mesh factory") << "Downloading ng mesh" << std::endl;
          mesh = m_reader->readMesh(
                ZDvidUrl::GetMeshKey(config.getBodyId(), ZDvidUrl::EMeshType::NG));
          if (mesh) {
            HLDEBUG("mesh factory") << "Mesh downloaded" << std::endl;
            actualConfig.setDsLevel(0);
//            *acturalMeshZoom = 0;
          } else {
            HLDEBUG("mesh factory") << "Downloading ng mesh for zoom: " << zoom << std::endl;
            mesh = m_reader->readMesh(
                  ZDvidUrl::GetMeshKey(
                    config.getBodyId(), zoom, ZDvidUrl::EMeshType::NG));
          }
        }
      }
    }
  }

  if (m_usingObjMesh && (mesh == nullptr)) {
    HLDEBUG("mesh factory") << "Downloading obj mesh" << std::endl;
    mesh = m_reader->readMesh(config.getBodyId(), zoom);
  }

  return {mesh, actualConfig};
}
