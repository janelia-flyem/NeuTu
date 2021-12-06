#ifndef FLYEMBODYMESHFACTORYTEST_H
#define FLYEMBODYMESHFACTORYTEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"
#include "neutubeconfig.h"
#include "zmeshfactory.h"
#include "zmesh.h"
#include "flyem/flyemfunctionbodymeshfactory.h"
#include "flyem/flyembodyconfigbuilder.h"
#include "flyem/flyemchainedbodymeshfactory.h"
#include "flyem/flyembodysource.h"
#include "flyem/flyemsparsevolbodymeshfactory.h"

TEST(FlyEmFunctionBodyMeshFactory, make)
{
  FlyEmFunctionBodyMeshFactory factory([](const FlyEmBodyConfig &config) {
    ZMesh *mesh = nullptr;
    if (config.getBodyId() == 1) {
      mesh = new ZMesh{ZMesh::CreateCube()};
    }
    return FlyEmBodyMesh{mesh, config};
  });

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(1));
    ASSERT_EQ(1, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_TRUE(bodyMesh.hasData());
  }

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(2));
    ASSERT_EQ(2, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_FALSE(bodyMesh.hasData());
  }

//  bodyMesh.releaseData()->save(GET_TEST_DATA_DIR + "/_test.obj");
}

TEST(FlyEmChainedBodyMeshFactory, make)
{
  FlyEmFunctionBodyMeshFactory *factory1 = new FlyEmFunctionBodyMeshFactory(
        [](const FlyEmBodyConfig &config) {
    ZMesh *mesh = nullptr;
    if (config.getBodyId() == 1) {
      mesh = new ZMesh{ZMesh::CreateCube()};
    }
    return FlyEmBodyMesh{mesh, config};
  });

  FlyEmFunctionBodyMeshFactory *factory2 = new FlyEmFunctionBodyMeshFactory(
        [](const FlyEmBodyConfig &config) {
    ZMesh *mesh = nullptr;
    if (config.getBodyId() == 2) {
      mesh = new ZMesh{ZMesh::CreateCube()};
    }
    return FlyEmBodyMesh{mesh, config};
  });

  FlyEmChainedBodyMeshFactory factory;
  factory.append(factory1);
  factory.append(factory2);

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(1));
    ASSERT_EQ(1, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_TRUE(bodyMesh.hasData());
  }

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(2));
    ASSERT_EQ(2, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_TRUE(bodyMesh.hasData());
  }

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(3));
    ASSERT_EQ(3, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_FALSE(bodyMesh.hasData());
  }

  factory.setPostProcess([](FlyEmBodyMesh &bm) {
    bm.setDsLevel(3);
  });

  {
    FlyEmBodyMesh bodyMesh = factory.make(FlyEmBodyConfigBuilder(1));
    ASSERT_EQ(1, bodyMesh.getBodyConfig().getBodyId());
    ASSERT_TRUE(bodyMesh.hasData());
    ASSERT_EQ(3, bodyMesh.getBodyConfig().getDsLevel());
  }
}

class FlyEmMockBodySource : public FlyEmBodySource
{
  ZObject3dScan* getSparsevol(uint64_t bodyId, int dsLevel) const override {
    ZObject3dScan *obj = new ZObject3dScan;
    if (bodyId == 1 && dsLevel == 0) {
      obj->addSegment(0, 0, 0, 0);
    }
    return obj;
  }
};

TEST(FlyEmSparsevolBodyMeshFactory, make)
{
  FlyEmSparsevolBodyMeshFactory factory;
  factory.setBodySource(new FlyEmMockBodySource);
  {
    auto bodyMesh = factory.make(FlyEmBodyConfigBuilder(1).withDsLevel(0));
    ASSERT_TRUE(bodyMesh.hasData());
  }
  {
    auto bodyMesh = factory.make(FlyEmBodyConfigBuilder(1).withDsLevel(1));
    ASSERT_FALSE(bodyMesh.hasData());
  }
  {
    auto bodyMesh = factory.make(FlyEmBodyConfigBuilder(2).withDsLevel(2));
    ASSERT_FALSE(bodyMesh.hasData());
  }

}

#endif


#endif // FLYEMBODYMESHFACTORYTEST_H
