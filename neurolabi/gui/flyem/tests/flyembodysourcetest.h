#ifndef FLYEMBODYSOURCETEST_H
#define FLYEMBODYSOURCETEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"

#include "../flyembodysource.h"
#include "zobject3dfactory.h"

class FlyEmBodyMockSource : public FlyEmBodySource {
public:
  FlyEmBodyMockSource() {};

  ZObject3dScan* getSparsevol(uint64_t bodyId, int dsLevel) const {
    if (bodyId == 1) {
      if (dsLevel == 0) {
        return ZObject3dFactory::MakeBoxObject3dScan({0, 0, 0, 1, 1, 1}, nullptr);
      }
    }
    return nullptr;
  }
};

TEST(FlyEmBodySource, Basic)
{
  FlyEmBodyMockSource source;
  ASSERT_NE(nullptr, source.getSparsevol(1, 0));
  ASSERT_EQ(nullptr, source.getSparsevol(1, 1));
}

#endif

#endif // FLYEMBODYSOURCETEST_H
