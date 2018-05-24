#ifndef ZFLYEMBODY3DTEST_H
#define ZFLYEMBODY3DTEST_H

#include "ztestheader.h"
#include "flyem/zflyembody3ddoc.h"
#include "neutubeconfig.h"
#include "zstackobjectsourcefactory.h"

#ifdef _USE_GTEST_
TEST(FlyEmBody3d, source)
{
  std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(123);
  ASSERT_EQ("#.FlyEmBody#123", source);

  uint64_t id = ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(source);
  ASSERT_EQ(123, (int) id);

  int zoom = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(source);
  ASSERT_EQ(0, zoom);


  source = ZStackObjectSourceFactory::MakeFlyEmBodySource(123, 1);
  ASSERT_EQ("#.FlyEmBody#123_1", source);

  source =
      ZStackObjectSourceFactory::MakeFlyEmBodySource(123, 1, flyem::BODY_FULL);
  ASSERT_EQ("#.FlyEmBody#123_1#.full", source);

  source =
      ZStackObjectSourceFactory::MakeFlyEmBodySource(123, 1, flyem::BODY_COARSE);
  ASSERT_EQ("#.FlyEmBody#123_1#.coarse", source);

  id = ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(source);
  ASSERT_EQ(123, (int) id);

  zoom = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(source);
  ASSERT_EQ(1, zoom);
}

#endif

#endif // ZFLYEMBODY3DTEST_H
