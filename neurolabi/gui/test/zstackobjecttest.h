#ifndef ZSTACKOBJECTTEST_H
#define ZSTACKOBJECTTEST_H

#include "ztestheader.h"
#include "zstackobject.h"
#include "zswctree.h"
#include "zstackball.h"

#ifdef _USE_GTEST_

TEST(ZStackObject, basic)
{
  ASSERT_EQ("SWC", ZStackObject::GetTypeName(ZStackObject::EType::SWC));

  ZStackObject::SetDefaultPenWidth(5.0);
  ASSERT_EQ(5.0, ZStackObject::GetDefaultPenWidth());
  ZStackObject::SetDefaultPenWidth(6.0);
  ASSERT_EQ(6.0, ZStackObject::GetDefaultPenWidth());

  ASSERT_TRUE(ZStackObject::IsSameSource("test", "test"));
  ASSERT_FALSE(ZStackObject::IsSameSource("test", "test2"));
  ASSERT_FALSE(ZStackObject::IsSameSource("", "test2"));
  ASSERT_FALSE(ZStackObject::IsSameSource("test", ""));
  ASSERT_FALSE(ZStackObject::IsSameSource("", ""));

  ASSERT_FALSE(ZStackObject::IsSelected(nullptr));
}

TEST(ZStackObject, Handle)
{
  uint64_t handle = ZSwcTree().getHandle();

  ZSwcTree obj1;
  ASSERT_EQ(handle + 1, obj1.getHandle());

  ZStackBall obj2;
  ASSERT_EQ(handle + 2, obj2.getHandle());

}

#endif

#endif // ZSTACKOBJECTTEST_H
