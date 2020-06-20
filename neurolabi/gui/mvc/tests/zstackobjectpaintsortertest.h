#ifndef ZSTACKOBJECTPAINTSORTERTEST_H
#define ZSTACKOBJECTPAINTSORTERTEST_H

#include "mvc/zstackobjectpaintsorter.h"

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "zstackball.h"

TEST(ZStackObjectPaintSorter, Basic)
{
  ZStackObjectPaintSorter sorter;
  ASSERT_TRUE(sorter.isEmpty());

  {
    ZStackBall *obj = new ZStackBall;
    obj->setTarget(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
    sorter.add(obj);
    ASSERT_FALSE(sorter.isEmpty());
  }

  sorter.clear();
  ASSERT_TRUE(sorter.isEmpty());

  {
    ZStackBall *obj = new ZStackBall;
    obj->setTarget(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
    sorter.add(obj);
    ASSERT_FALSE(sorter.isEmpty());
  }

  QList<ZStackObject*> objList = sorter.getVisibleObjectList(
        neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS,
        ZStackObjectPaintSorter::SortingZ(false));
  ASSERT_EQ(1, objList.size());

  {
    ZStackBall *obj = new ZStackBall;
    obj->setTarget(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
    sorter.add(obj);
    ASSERT_FALSE(sorter.isEmpty());
  }
  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(false));
  ASSERT_EQ(2, objList.size());

  {
    ZStackBall *obj = new ZStackBall;
    obj->setTarget(neutu::data3d::ETarget::HD_OBJECT_CANVAS);
    sorter.add(obj);
    ASSERT_FALSE(sorter.isEmpty());
  }
  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::HD_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(false));
  ASSERT_EQ(1, objList.size());

  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(false));
  ASSERT_EQ(2, objList.size());

  sorter.clear();
  ASSERT_TRUE(sorter.isEmpty());

  objList.clear();
  objList.append(new ZStackBall());
  objList.append(new ZStackBall());
  objList.append(new ZStackBall());
  for (int i = 0; i < objList.size(); ++i) {
    auto obj = objList[i];
    obj->setTarget(neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS);
    obj->setZOrder(3 - i);
  }
  sorter.add(objList);
  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(false));
  ASSERT_EQ(3, objList.size());
  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(true));
  ASSERT_EQ(1, objList[0]->getZOrder());
  ASSERT_EQ(2, objList[1]->getZOrder());
  ASSERT_EQ(3, objList[2]->getZOrder());

  objList[0]->setVisible(false);
  objList = sorter.getVisibleObjectList(
          neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS,
          ZStackObjectPaintSorter::SortingZ(true));
  ASSERT_EQ(2, objList.size());
  ASSERT_EQ(2, objList[0]->getZOrder());
  ASSERT_EQ(3, objList[1]->getZOrder());

  {
    ZStackBall *obj = new ZStackBall;
    obj->setTarget(neutu::data3d::ETarget::HD_OBJECT_CANVAS);
    sorter.add(obj);
    ASSERT_FALSE(sorter.isEmpty());
  }

  sorter.forEachVisibleTarget(
        [](neutu::data3d::ETarget target, const QList<ZStackObject*> &objList) {
    if (!objList.isEmpty()) {
      std::cout << "Target: " << neutu::ToString(target) << std::endl;
      for (auto obj : objList) {
        std::cout << obj->isVisible() << " " << obj->getZOrder() << std::endl;
      }
    }
  }, true);
}

#endif

#endif // ZSTACKOBJECTPAINTSORTERTEST_H
