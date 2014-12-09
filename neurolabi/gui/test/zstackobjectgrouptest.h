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

#endif

#endif // ZSTACKOBJECTGROUPTEST_H
