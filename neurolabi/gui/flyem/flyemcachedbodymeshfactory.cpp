#include "flyemcachedbodymeshfactory.h"

#include "common/debug.h"
#include "flyembodymeshcache.h"
#include "flyembodyconfigbuilder.h"

FlyEmCachedBodyMeshFactory::FlyEmCachedBodyMeshFactory()
{

}

FlyEmBodyMesh FlyEmCachedBodyMeshFactory::make_(const FlyEmBodyConfig &config)
{
  FlyEmBodyConfig newConfig = config;
  if (m_fastMeshFactory) {
    // Resolution rule for fast factory when the request res level is:
    //   1. <0 or bigger than m_maxResLevel: start from m_minResLevel to m_maxResLevel
    //   2. in range: start from m_minResLevel to the specified level
    //   3. smaller than m_minResLevel: try m_minResLevel only
    int startLevel = m_minResLevel;
    int endLevel = m_maxResLevel;
    if (config.getDsLevel() >= 0) {
      if (config.getDsLevel() < m_minResLevel) {
        endLevel = m_minResLevel;
      } else if (config.getDsLevel() <= m_maxResLevel) {
        endLevel = config.getDsLevel();
      }
    }
    HLDEBUG("body mesh") << "Fast factory level: "
                            << startLevel << "->" << endLevel << std::endl;
    for (int level = startLevel; level <= endLevel; ++level) {
      FlyEmBodyMesh bodyMesh = m_fastMeshFactory->make(
            FlyEmBodyConfigBuilder(config).withDsLevel(level));
      if (bodyMesh.hasData()) {
        return bodyMesh;
      }
    }
  }

  // Resolution rule for cache when the request res level is:
  //   1. <0 or bigger than m_maxResLevel: use auto res level
  //   2. in range: start from m_minResLevel to the specified level
  //   3. smaller than m_minResLevel: try m_minResLevel only
  FlyEmBodyMeshCache::MeshIndex index;
  if (!config.isHybrid()) {
    index.bodyId = config.getBodyId();
    index.resLevel = -1;
    index.mutationId = -1;
  } else {
    HLDEBUG("body mesh") << "Skip indexing for hybrid body" << std::endl;
  }

  if (m_meshCache && index.isValid()) {
    index.minResLevel = m_minResLevel;
    index.maxResLevel = m_maxResLevel;

    if (config.getDsLevel() >= 0) {
      if (config.getDsLevel() < m_minResLevel) {
        index.maxResLevel = m_minResLevel;
      } else if (config.getDsLevel() <= m_maxResLevel) {
        index.maxResLevel = config.getDsLevel();
      }
    }
    HLDEBUG("body mesh") << "Cache level: "
                            << index.minResLevel << "->" << index.maxResLevel << std::endl;
    auto mesh = m_meshCache->get(index);
    if (mesh.second) {
      index = mesh.first;
      return FlyEmBodyMesh(
            mesh.second,
            FlyEmBodyConfigBuilder(config).withDsLevel(index.resLevel));
    }
  }

  if (m_slowMeshFactory) {
    // Resolution rule for slow factory when the request res level is:
    //   1. < 0 or bigger than m_maxResLevel: try m_maxResLevel only
    //   2. in range: try the specified level only
    //   3. smaller than m_minResLevel: try m_minResLevel only
    int level = config.getDsLevel();
    if (level < 0 || level > m_maxResLevel) {
      level = m_maxResLevel;
    } else if (level < m_minResLevel) {
      level = m_minResLevel;
    }
    FlyEmBodyMesh bodyMesh = m_slowMeshFactory->make(
          FlyEmBodyConfigBuilder(config).withDsLevel(level));
    if (m_meshCache && bodyMesh.hasData() && !bodyMesh.getBodyConfig().isHybrid()) {
      if (index.mutationId == -1) {
        if (m_meshCache) {
          index.mutationId = m_meshCache->getLatestMutationId(index.bodyId);
        }
      }
      index.resLevel = bodyMesh.getBodyConfig().getDsLevel();
      HLDEBUG("body mesh") << "Caching " << index << std::endl;
      m_meshCache->set(index, bodyMesh.getData());
    }
    return bodyMesh;
  }

  return FlyEmBodyMesh(nullptr, config);
}

void FlyEmCachedBodyMeshFactory::setFastFactory(
    std::shared_ptr<FlyEmBodyMeshFactory> f)
{
  m_fastMeshFactory = f;
}

void FlyEmCachedBodyMeshFactory::setSlowFactory(
    std::shared_ptr<FlyEmBodyMeshFactory> f)
{
  m_slowMeshFactory = f;
}

void FlyEmCachedBodyMeshFactory::setCache(
    std::shared_ptr<FlyEmBodyMeshCache> cache)
{
  m_meshCache = cache;
}

