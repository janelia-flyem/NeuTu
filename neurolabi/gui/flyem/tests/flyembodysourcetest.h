#ifndef FLYEMBODYSOURCETEST_H
#define FLYEMBODYSOURCETEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"

#include "../flyembodysource.h"
#include "zobject3dfactory.h"

class FlyEmBodyMockSource : public FlyEmBodySource {
public:
  FlyEmBodyMockSource() {};

  ZObject3dScan* getSparsevol(uint64_t bodyId, int dsLevel) const override {
    if (bodyId == 1) {
      if (dsLevel == 0) {
        return ZObject3dFactory::MakeBoxObject3dScan({0, 0, 0, 1, 1, 1}, nullptr);
      }
    }
    return nullptr;
  }

  int getCoarseSparsevolScale() const override {
    return 5;
  }


};

TEST(FlyEmBodySource, Basic)
{
  FlyEmBodyMockSource source;
  ASSERT_EQ(nullptr, source.getSparsevol(1, 1));

  ZObject3dScan *obj = source.getSparsevol(1, 0);
  ASSERT_EQ(8, obj->getVoxelNumber());

  ASSERT_TRUE(source.getBoundBox(0).isEmpty());
  ZIntCuboid box = source.getBoundBox(1);
  ASSERT_EQ(ZIntCuboid(0, 0, 0, 1, 1, 1), box);

  box = source.getBoundBox(1, 1);
  ASSERT_EQ(ZIntCuboid(0, 0, 0, 0, 0, 0), box);

  ZIntPoint blockSize = source.getBlockSize();
  ASSERT_EQ(ZIntPoint(32, 32, 32), blockSize);
}

#endif

#endif // FLYEMBODYSOURCETEST_H
