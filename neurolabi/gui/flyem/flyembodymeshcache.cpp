#include "flyembodymeshcache.h"

#include "common/debug.h"
#include "zstring.h"

FlyEmBodyMeshCache::FlyEmBodyMeshCache()
{
}

FlyEmBodyMeshCache::~FlyEmBodyMeshCache()
{
}

int FlyEmBodyMeshCache::getLatestMutationId(uint64_t /*bodyId*/) const
{
  return -1;
}

namespace {

bool is_res_in_range(int level, int minResLevel, int maxResLevel) {
  if (minResLevel >= 0) {
    if (level < minResLevel) {
      return false;
    }
  }

  if (maxResLevel >= 0) {
    if (level > maxResLevel) {
      return false;
    }
  }
  return true;
}

}

FlyEmBodyMeshCache::MeshIndex
FlyEmBodyMeshCache::getActualIndex(const MeshIndex &index) const
{
  MeshIndex actualIndex = index;
  if (index.isValid()) {
    if (index.mutationId == -1) {
      actualIndex.mutationId = getLatestMutationId(index.bodyId);
    }
    if (index.resLevel == -1) {
      std::vector<int> resLevels = getCachedResLevels(
            actualIndex.bodyId, actualIndex.mutationId);
      for (int resLevel : resLevels) {
        if (actualIndex.resLevel < 0 || actualIndex.resLevel > resLevel) {
          if (is_res_in_range(resLevel, index.minResLevel, index.maxResLevel)) {
            actualIndex.resLevel = resLevel;
          }
        }
      }
    }
  }

  HLDEBUG("mesh cache") << "Get actual index: " << index << " -> " << actualIndex << std::endl;

  return actualIndex;
}

std::pair<FlyEmBodyMeshCache::MeshIndex, ZMesh*>
FlyEmBodyMeshCache::get(const MeshIndex &index) const
{
  MeshIndex actualIndex = getActualIndex(index);
  if (actualIndex.isSolidValid()) {
    ZMesh *mesh = getFromSolidIndex(actualIndex);
    return {actualIndex, mesh};
  } else {
    HLDEBUG_FUNC("mesh cache") << "Invalid index: " << actualIndex << std::endl;
  }

  return {actualIndex, nullptr};
}

std::vector<std::string> FlyEmBodyMeshCache::getCachedKeys(
    uint64_t /*bodyId*/, int /*mutationId*/) const
{
  return std::vector<std::string>();
}

std::vector<int> FlyEmBodyMeshCache::getCachedResLevels(
    uint64_t bodyId, int mutationId) const
{
  auto keyList = getCachedKeys(bodyId, mutationId);
  std::vector<int> resList(keyList.size());
  for (size_t i = 0; i < keyList.size(); ++i) {
    resList[i] = ZString::LastInteger(keyList[i]);
  }

#ifdef _DEBUG_
  HLDEBUG("mesh cache") << "Res levels: ";
  std::for_each(keyList.begin(), keyList.end(), [](const std::string &res) {
    std::cout << res << ", ";
  });
  std::cout << " -> ";
  std::for_each(resList.begin(), resList.end(), [](int res) {
    std::cout << res << ", ";
  });
  std::cout << std::endl;
#endif

  return resList;
}

FlyEmBodyMeshCache::MeshIndexBuilder::MeshIndexBuilder(uint64_t bodyId)
{
  m_result.bodyId = bodyId;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::autoRes()
{
  m_result.resLevel = -1;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::latestMutation()
{
  m_result.mutationId = -1;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::withResLevel(int level)
{
  m_result.resLevel = level;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::withMinResLevel(int level)
{
  m_result.minResLevel = level;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::withMaxResLevel(int level)
{
  m_result.maxResLevel = level;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder&
FlyEmBodyMeshCache::MeshIndexBuilder::withMutation(int64_t mid)
{
  m_result.mutationId = mid;
  return *this;
}

FlyEmBodyMeshCache::MeshIndexBuilder::operator MeshIndex() const
{
  return m_result;
}
