#ifndef ZSTACKOBJECTGROUPTEST_H
#define ZSTACKOBJECTGROUPTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackobjectgroup.h"
#include "zobject3d.h"

#ifdef _USE_GTEST_

TEST(ZStackObjectGroup, Basic) {
  ZStackObjectGroup objectGroup;
  ASSERT_TRUE(objectGroup.isEmpty());

  ZObject3d *obj = new ZObject3d;

  objectGroup.add(obj, true);
  obj = new ZObject3d;
  objectGroup.add(obj, true);

  ASSERT_EQ(2, objectGroup.size());
}

TEST(ZStackObjectGroup, Selection) {
  ZStackObjectGroup objectGroup;

  ZObject3d *obj = new ZObject3d;
  obj->setSource("1");
  objectGroup.add(obj, true);
  ZObject3d *obj2 = new ZObject3d;
  objectGroup.add(obj2, true);
  obj2->setSource("2");

  objectGroup.resetSelection();
  objectGroup.setSelected(obj, true);
  ASSERT_TRUE(objectGroup.getSelector()->isInSelectedSet(obj));
  ASSERT_FALSE(objectGroup.getSelector()->isInDeselectedSet(obj));

  objectGroup.setSelected(obj, false);
  ASSERT_FALSE(objectGroup.getSelector()->isInSelectedSet(obj));
  ASSERT_FALSE(objectGroup.getSelector()->isInDeselectedSet(obj));

  obj->setSelected(true);
  objectGroup.setSelected(obj, false);
  ASSERT_FALSE(objectGroup.getSelector()->isInSelectedSet(obj));
  ASSERT_TRUE(objectGroup.getSelector()->isInDeselectedSet(obj));

  objectGroup.setSelected(false);
  ASSERT_FALSE(objectGroup.getSelector()->isInSelectedSet(obj));
  ASSERT_TRUE(objectGroup.getSelector()->isInDeselectedSet(obj));

  objectGroup.setSelected(true);
  ASSERT_FALSE(objectGroup.getSelector()->isInSelectedSet(obj));
  ASSERT_FALSE(objectGroup.getSelector()->isInDeselectedSet(obj));
  ASSERT_TRUE(objectGroup.getSelector()->isInSelectedSet(obj2));

  objectGroup.getSelector()->print();
}

TEST(ZStackObjectGroup, ZOrder)
{
  ZStackObjectGroup objectGroup;
  ZObject3d *obj2 = new ZObject3d;

  objectGroup.add(obj2, false);

  ASSERT_EQ(obj2->getZOrder(), 1);

  ZObject3d *obj3 = new ZObject3d;
  objectGroup.add(obj3, false);
  ASSERT_EQ(obj3->getZOrder(), 2);

  ZObject3d *obj4 = new ZObject3d;
  objectGroup.add(obj4, 100, false);
  ASSERT_EQ(obj4->getZOrder(), 100);

  ZObject3d *obj5 = new ZObject3d;
  objectGroup.add(obj5, false);
  ASSERT_EQ(obj5->getZOrder(), 101);

  objectGroup.compressZOrder();

  ASSERT_EQ(obj2->getZOrder(), 1);
  ASSERT_EQ(obj3->getZOrder(), 2);
  ASSERT_EQ(obj4->getZOrder(), 3);
  ASSERT_EQ(obj5->getZOrder(), 4);

  ZObject3d *obj6 = new ZObject3d;
  objectGroup.add(obj6, false);
  ASSERT_EQ(obj6->getZOrder(), 5);

}

#endif

#endif // ZSTACKOBJECTGROUPTEST_H
