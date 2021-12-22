#ifndef FLYEMBODYMESHCACHE_MOCK_H
#define FLYEMBODYMESHCACHE_MOCK_H

#include "neulib/core/stringbuilder.h"
#include "../flyem/flyembodymeshcache.h"
#include "zmesh.h"
#include "zstring.h"

class FlyEmBodyMeshCache_Mock : public FlyEmBodyMeshCache
{
public:
  void set(const MeshIndex &index, const ZMesh *mesh) override {
    std::string key = getKey(index);
    if (!key.empty() && mesh) {
      m_meshMap[key] = mesh->clone();
    }
  }

  int getLatestMutationId(uint64_t /*bodyId*/) const override {
    return 100;
  }

protected:
  ZMesh* getFromSolidIndex(const MeshIndex &index) const override {
    std::string key = getKey(index);
    if (m_meshMap.count(key) > 0) {
      return m_meshMap.at(key);
    }
    return nullptr;
  };

  std::vector<std::string> getCachedKeys(uint64_t bodyId, int mutationId) const override
  {
    std::vector<std::string> keyList;
    for (const auto &entry : m_meshMap) {
      if (ZString(entry.first).startsWith(getBodyKey(bodyId, mutationId) + "_")) {
        keyList.push_back(entry.first);
      }
    }
    return keyList;
  }

private:
  std::string getBodyKey(const MeshIndex &index) const {
    return getBodyKey(index.bodyId, index.mutationId);
  }

  std::string getBodyKey(int bodyId, int mutationId) const {
    if (bodyId > 0 && mutationId >= 0) {
      return neulib::StringBuilder("[$]_[$]").arg(bodyId).arg(mutationId);
    }
    return "";
  }

  std::string getKey(const MeshIndex &index) const {
    if (index.isSolidValid()) {
      return neulib::StringBuilder("[$]_[$]").arg(getBodyKey(index))
          .arg(index.resLevel);
    }

    return "";
  }

private:
  std::unordered_map<std::string, ZMesh*> m_meshMap;
};

#endif // FLYEMBODYMESHCACHE_MOCK_H
