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
  ZSwcTree obj1;
  ASSERT_TRUE(obj1.getHandle().isValid());
  ASSERT_NE(ZSwcTree().getHandle(), obj1.getHandle());

  ZStackBall obj2;
  ASSERT_TRUE(obj2.getHandle().isValid());
  ASSERT_NE(obj1.getHandle(), obj2.getHandle());

  ZStackBall obj3 = obj2;
  ASSERT_TRUE(obj3.getHandle().isValid());
  ASSERT_NE(obj2.getHandle(), obj3.getHandle());

}

#endif

#endif // ZSTACKOBJECTTEST_H
