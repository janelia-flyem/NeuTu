#ifndef ZSTACKMVCTEST_H
#define ZSTACKMVCTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "mvc/zstackmvc.h"
#include "mvc/zstackdoc.h"

TEST(ZStackMvc, Make)
{
  std::shared_ptr<ZStackDoc> doc = std::shared_ptr<ZStackDoc>(new ZStackDoc);
  ZStackMvc::Make(nullptr, doc);
}

#endif

#endif // ZSTACKMVCTEST_H
