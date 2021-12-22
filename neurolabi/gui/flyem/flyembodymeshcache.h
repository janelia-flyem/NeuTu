#ifndef FLYEMBODYMESHCACHE_H
#define FLYEMBODYMESHCACHE_H

#include <utility>
#include <vector>
#include <string>
#include <iostream>

#include "tz_stdint.h"

class ZMesh;

class FlyEmBodyMeshCache
{
public:
  FlyEmBodyMeshCache();
  virtual ~FlyEmBodyMeshCache();

  struct MeshIndex {
    uint64_t bodyId = 0;
    int64_t mutationId = -1; // Try to get the latest mutation if it is -1
    int resLevel = 0; // Try to get mesh with the best resolution if it is -1.
    int minResLevel = 0;
    int maxResLevel = 30; // reaching max int

    bool isValid() const {
      return bodyId > 0;
    }

    bool isSolidValid() const {
      return bodyId > 0 && mutationId >= 0 && resLevel >= 0;
    }

    friend std::ostream& operator<< (std::ostream& stream, const MeshIndex &index) {
      stream << index.bodyId << "[" << index.mutationId << "]" << "@L" << index.resLevel;
      return stream;
    }
  };

  class MeshIndexBuilder {
  public:
    MeshIndexBuilder(uint64_t bodyId);
    MeshIndexBuilder& autoRes();
    MeshIndexBuilder& latestMutation();
    MeshIndexBuilder& withMutation(int64_t mid);
    MeshIndexBuilder& withResLevel(int level);
    MeshIndexBuilder& withMinResLevel(int level);
    MeshIndexBuilder& withMaxResLevel(int level);

    operator MeshIndex() const;
  private:
    MeshIndex m_result;
  };

  /*!
   * \brief Get a mesh from cache.
   *
   * It returns a cached mesh and its actual index. The caller is responsible
   * for managing the returned mesh memory.
   */
  std::pair<MeshIndex, ZMesh*> get(const MeshIndex &index) const;

  /*!
   * \brief Cache a mesh.
   *
   * Nothing will be done if \a mesh is null. Even if it is not null, it does
   * not garuantee that the mesh will be cached. The function will not take
   * the ownership of \a mesh. So it is still the caller's resposibility to
   * manage its memory. \a index must be solidified
   */
  virtual void set(const MeshIndex &index, const ZMesh *mesh) = 0;

  /*!
   * \brief Get the actual index for retrieving a mesh.
   *
   * Since MeshIndex allows automatic determination for some attributes such
   * as mutation id and resolution level, \a index may result in a different
   * actual index in a different context.
   */
  virtual MeshIndex getActualIndex(const MeshIndex &index) const;

  /*!
   * \brief Get the latest mutation ID of a body.
   *
   * This API is for making an index for the most recent data. If it returns -1,
   * it means that the cache does not know how to get the mutation ID.
   */
  virtual int getLatestMutationId(uint64_t bodyId) const;

protected:
  virtual ZMesh* getFromSolidIndex(const MeshIndex &index) const = 0;
  virtual std::vector<int> getCachedResLevels(uint64_t bodyId, int mutationId) const;
  virtual std::vector<std::string> getCachedKeys(uint64_t bodyId, int mutationId) const;
};

#endif // FLYEMBODYMESHCACHE_H
