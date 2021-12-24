#ifndef FLYEMBODYMESHCACHETEST_H
#define FLYEMBODYMESHCACHETEST_H

#ifdef _USE_GTEST_

#include <unordered_map>

#include "../flyem/flyembodymeshcache.h"
#include "../flyem/flyemdvidbodymeshcache.h"
#include "flyembodymeshcache_mock.h"
#include "test/ztestheader.h"
#include "zmeshfactory.h"
#include "dvid/zdvidtarget.h"


TEST(FlyEmBodyMeshCache, Basic)
{
  FlyEmBodyMeshCache_Mock cache;

  FlyEmBodyMeshCache::MeshIndex index;
  ASSERT_EQ(nullptr, cache.get(index).second);

  ZMesh mesh = ZMesh::CreateSphereMesh(glm::vec3(0.0, 0.0, 0.0), 5);
  cache.set(index, &mesh);
  ASSERT_EQ(nullptr, cache.get(index).second);

  index.bodyId = 1;
  index.mutationId = 0;
  index.resLevel = 3;

  cache.set(index, &mesh);
  ASSERT_NE(nullptr, cache.get(index).second);

  index.bodyId = 1;
  index.mutationId = -1;
  index.resLevel = -1;

  auto actualIndex = cache.getActualIndex(index);
  ASSERT_EQ(1, actualIndex.bodyId);
  ASSERT_EQ(100, actualIndex.mutationId);
  ASSERT_EQ(-1, actualIndex.resLevel);

  ASSERT_EQ(nullptr, cache.get(index).second);

  index.bodyId = 1;
  index.mutationId = 0;
  index.resLevel = -1;

  actualIndex = cache.getActualIndex(index);
  ASSERT_EQ(1, actualIndex.bodyId);
  ASSERT_EQ(0, actualIndex.mutationId);
  ASSERT_EQ(3, actualIndex.resLevel);

  ASSERT_NE(nullptr, cache.get(index).second);
  ASSERT_NE(nullptr, cache.get(actualIndex).second);

  index.bodyId = 1;
  index.mutationId = 0;
  index.resLevel = 2;
  cache.set(index, &mesh);

  actualIndex = cache.getActualIndex(index);
  ASSERT_EQ(1, actualIndex.bodyId);
  ASSERT_EQ(0, actualIndex.mutationId);
  ASSERT_EQ(2, actualIndex.resLevel);
}

TEST(FlyEmBodyMeshCache, MeshIndex)
{
  FlyEmBodyMeshCache::MeshIndex index = FlyEmBodyMeshCache::MeshIndexBuilder(1);
  ASSERT_EQ(1, index.bodyId);
  ASSERT_EQ(-1, index.mutationId);
  ASSERT_EQ(0, index.resLevel);
  ASSERT_EQ(0, index.minResLevel);
  ASSERT_EQ(30, index.maxResLevel);

  index = FlyEmBodyMeshCache::MeshIndexBuilder(1).withResLevel(2);
  ASSERT_EQ(1, index.bodyId);
  ASSERT_EQ(-1, index.mutationId);
  ASSERT_EQ(2, index.resLevel);
  ASSERT_EQ(0, index.minResLevel);
  ASSERT_EQ(30, index.maxResLevel);

  index = FlyEmBodyMeshCache::MeshIndexBuilder(1).withResLevel(2).withMutation(3);
  ASSERT_EQ(1, index.bodyId);
  ASSERT_EQ(3, index.mutationId);
  ASSERT_EQ(2, index.resLevel);
  ASSERT_EQ(0, index.minResLevel);
  ASSERT_EQ(30, index.maxResLevel);

  index = FlyEmBodyMeshCache::MeshIndexBuilder(1).withResLevel(2).
      withMutation(3).withMinResLevel(1).withMaxResLevel(5);
  ASSERT_EQ(1, index.bodyId);
  ASSERT_EQ(3, index.mutationId);
  ASSERT_EQ(2, index.resLevel);
  ASSERT_EQ(1, index.minResLevel);
  ASSERT_EQ(5, index.maxResLevel);

  index = FlyEmBodyMeshCache::MeshIndexBuilder(1).autoRes().
      withMutation(3).withMinResLevel(4).withMaxResLevel(5);
  ASSERT_EQ(1, index.bodyId);
  ASSERT_EQ(3, index.mutationId);
  ASSERT_EQ(-1, index.resLevel);
  ASSERT_EQ(4, index.minResLevel);
  ASSERT_EQ(5, index.maxResLevel);
}

TEST(FlyEmDvidBodyMeshCache, Basic)
{
  FlyEmDvidBodyMeshCache cache;
  ZDvidTarget target;
  EXPECT_THROW(cache.setDvidTarget(target), std::runtime_error);

  FlyEmBodyMeshCache::MeshIndex index;
  ASSERT_EQ(nullptr, cache.get(index).second);

  index = FlyEmBodyMeshCache::MeshIndexBuilder(1).autoRes();
  ASSERT_EQ(nullptr, cache.get(index).second);

  index.resLevel = 1;
  index.mutationId = 1;
  ASSERT_TRUE(index.isSolidValid());
  ZMesh mesh = ZMesh::CreateSphereMesh(glm::vec3(0.0, 0.0, 0.0), 5);
  cache.set(index, &mesh);
}

#endif

#endif // FLYEMBODYMESHCACHETEST_H
