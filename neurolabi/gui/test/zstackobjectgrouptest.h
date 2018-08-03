#ifndef ZSTACKOBJECTGROUPTEST_H
#define ZSTACKOBJECTGROUPTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackobjectgroup.h"
#include "zobject3d.h"
#include "zswctree.h"

#ifdef _USE_GTEST_

static bool IsSwc(const ZStackObject *obj)
{
  if (obj == NULL) {
    return false;
  }

  return obj->getType() == ZSwcTree::GetType();
}

TEST(ZStackObjectGroup, Basic) {
  ZStackObjectGroup objectGroup;
  ASSERT_TRUE(objectGroup.isEmpty());

  ZObject3d *obj = new ZObject3d;

  objectGroup.add(obj, true);
  obj = new ZObject3d;
  objectGroup.add(obj, true);

  ASSERT_EQ(2, objectGroup.size());

  ZObject3d *obj2 = new ZObject3d;
  obj2->setSource("test");
  objectGroup.add(obj2, true);
  ASSERT_EQ(3, objectGroup.size());

  ZObject3d *obj3 = new ZObject3d;
  obj3->setSource("test");
  objectGroup.add(obj3, true);
  ASSERT_EQ(3, objectGroup.size());

  ZStackObjectGroup objectGroup2;
  objectGroup.moveTo(objectGroup2);
  ASSERT_EQ(3, objectGroup2.size());
  ASSERT_TRUE(objectGroup.isEmpty());


  ZSwcTree *tree = new ZSwcTree;
  objectGroup2.add(tree, false);
  ASSERT_EQ(4, objectGroup2.size());

  tree = new ZSwcTree;
  tree->setSource("test");
  objectGroup2.add(tree, true);
  ASSERT_EQ(5, objectGroup2.size());

  TStackObjectList objList = objectGroup2.findSameSource("test");
  ASSERT_EQ(2, objList.size());

  objList = objectGroup2.findSameSource(obj3);
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(obj3, dynamic_cast<ZStackObject*>(objList.front()));

  objList = objectGroup2.findSameSource(ZObject3d::GetType(), "test");
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(obj3), objList.front());

  objList = objectGroup2.findSameSource(ZSwcTree::GetType(), "test");
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree), objList.front());

  ZSwcTree *tree2 = new ZSwcTree;
  tree2->setObjectClass("tree");
  objectGroup2.add(tree2, true);

  ZSwcTree *tree3 = new ZSwcTree;
  tree3->setObjectClass("tree");
  objectGroup2.add(tree3, true);

  objList = objectGroup2.findSameClass(
        ZSwcTree::GetType(), "tree");
  ASSERT_EQ(2, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree2), objList[0]);
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree3), objList[1]);

  ZSwcTree *tree4 = new ZSwcTree;
  tree4->setSource("test");
  ZStackObject *replaced = objectGroup2.replaceFirstSameSource(tree4);
  objList = objectGroup2.findSameSource(ZSwcTree::GetType(), "test");
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree), replaced);
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree4), objList.front());

  replaced = objectGroup2.replaceFirstSameSource(tree2);
  ASSERT_EQ(NULL, replaced);
  objList = objectGroup2.findSameSource(ZSwcTree::GetType(), "test");
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree4), objList.front());

  QList<ZStackObject*> queryList;
  queryList.append(obj3);
  queryList.append(tree);

  objList = objectGroup2.findSameSource(queryList.begin(), queryList.end());
  ASSERT_EQ(2, objList.size());
  ASSERT_EQ(dynamic_cast<ZStackObject*>(obj3), objList[0]);
  ASSERT_EQ(dynamic_cast<ZStackObject*>(tree4), objList[1]);
}

TEST(ZStackObjectGroup, add) {
  ZStackObjectGroup group;

  group.add(NULL, 0, false);
  ASSERT_TRUE(group.isEmpty());

  ZObject3d *obj1 = new ZObject3d;
  obj1->setSource("obj1");

  group.add(obj1, false);
  ASSERT_EQ(1, group.size());

  group.add(obj1, false);
  ASSERT_EQ(1, group.size());

  group.add(obj1, true);
  ASSERT_EQ(1, group.size());

  ZObject3d *obj2 = new ZObject3d;
  obj2->setSource("obj2");
  group.add(obj2, true);
  ASSERT_EQ(2, group.size());


  ZObject3d *obj3 = new ZObject3d;
  obj3->setSource("test");
  group.add(obj3, true);
  ASSERT_EQ(3, group.size());

  ZObject3d *obj4 = new ZObject3d;
  obj4->setSource("test");
  group.add(obj4, true);
  ASSERT_EQ(3, group.size());

  ZObject3d *obj5 = new ZObject3d;
  obj5->setSource("test");
  group.add(obj5, 3, true);
  ASSERT_EQ(3, group.size());

  group.add(obj5, 3, true);
  ASSERT_EQ(3, group.size());

  QList<ZSwcTree*> treeList;
  for (int i = 0; i < 3; ++i) {
    treeList.append(new ZSwcTree);
  }

  group.add(treeList.begin(), treeList.end(), true);
  ASSERT_EQ(6, group.size());

  group.add(treeList.begin(), treeList.end(), true);
  ASSERT_EQ(6, group.size());


  for (int i = 0; i < 3; ++i) {
    ZSwcTree *tree = new ZSwcTree;
    tree->setSource("test");
    treeList.append(tree);
  }

  group.add(treeList.begin(), treeList.end(), true);
  ASSERT_EQ(7, group.size());


}

TEST(ZStackObjectGroup, take) {
  ZStackObjectGroup objectGroup;

  ZObject3d *obj = new ZObject3d;

  objectGroup.add(obj, true);
  obj = new ZObject3d;
  objectGroup.add(obj, true);

  ZObject3d *obj2 = new ZObject3d;
  obj2->setSource("test");
  objectGroup.add(obj2, true);

  ASSERT_EQ(obj, objectGroup.take(obj));
  ASSERT_EQ(2, objectGroup.size());

  TStackObjectList objList =
      objectGroup.takeSameSource(ZObject3d::TYPE_OBJ3D, "test");
  ASSERT_EQ(1, objList.size());
  ASSERT_EQ(obj2, objList.front());
  ASSERT_EQ(1, objectGroup.size());

  obj = new ZObject3d;
  objectGroup.add(obj, false);
  objectGroup.removeObject(obj, false);
  ASSERT_EQ(1, objectGroup.size());

  objectGroup.setSelected(ZObject3d::TYPE_OBJ3D, true);
  objectGroup.removeSelected(true);
  ASSERT_TRUE(objectGroup.isEmpty());


  ZSwcTree *tree = new ZSwcTree;
  objectGroup.add(tree, false);

  ZStackObject *taken = objectGroup.take(tree);
  ASSERT_EQ(taken, dynamic_cast<ZStackObject*>(tree));

  objectGroup.add(taken, false);

  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(1, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(1, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(ZSwcTree::GetType(), IsSwc);
  ASSERT_EQ(1, objList.size());
  ASSERT_TRUE(objectGroup.isEmpty());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(ZObject3d::GetType(), IsSwc);
  ASSERT_EQ(0, objList.size());

  objList = objectGroup.takeSelected();
  ASSERT_EQ(0, objList.size());

  objectGroup.setSelected(ZSwcTree::GetType(), true);
  objList = objectGroup.takeSelected();
  ASSERT_EQ(1, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);

  ZSwcTree *tree2 = new ZSwcTree;
  tree2->setSource("test");
  objectGroup.add(tree2, false);

  obj = new ZObject3d;
  objectGroup.add(obj, false);

  obj2 = new ZObject3d;
  obj2->setSource("test");
  objectGroup.add(obj2, false);

  ZObject3d *obj3 = new ZObject3d;
  obj3->setSource("test");
  objectGroup.add(obj3, false);

  ASSERT_EQ(5, objectGroup.size());

  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);

  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(IsSwc);
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(ZSwcTree::GetType(), IsSwc);
  ASSERT_EQ(2, objList.size());
  ASSERT_FALSE(objectGroup.isEmpty());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(ZObject3d::GetType(), IsSwc);
  ASSERT_EQ(0, objList.size());

  objList = objectGroup.takeSelected();
  ASSERT_EQ(1, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objectGroup.setSelected(ZSwcTree::GetType(), true);
  objList = objectGroup.takeSelected();
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.takeSelected(ZSwcTree::GetType());
  ASSERT_EQ(2, objList.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.takeSameSource(ZObject3d::GetType(), "test");
  ASSERT_EQ(2, objList.size());
  ASSERT_EQ(3, objectGroup.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objList = objectGroup.take(objList.begin(), objList.end());
  ASSERT_EQ(2, objList.size());
  ASSERT_EQ(3, objectGroup.size());

  objectGroup.add(objList.begin(), objList.end(), false);
  objectGroup.removeObject(obj, false);
  ASSERT_EQ(4, objectGroup.size());

  ASSERT_FALSE(objectGroup.containsUnsync(obj));

  objectGroup.add(obj, false);
  objectGroup.removeObject(ZObject3d::GetType(), false);
  ASSERT_EQ(2, objectGroup.size());

  ASSERT_FALSE(objectGroup.containsUnsync(obj));

  objectGroup.add(obj, false);
  objectGroup.add(obj2, false);
  objectGroup.add(obj3, false);

  TStackObjectSet objSet;
  objSet.insert(obj);
  objSet.insert(obj2);

  objectGroup.removeObject(objSet, false);
  ASSERT_FALSE(objectGroup.containsUnsync(obj));
  ASSERT_TRUE(objectGroup.containsUnsync(obj3));

  objectGroup.add(obj, false);
  objectGroup.add(obj2, false);
  objectGroup.add(obj3, false);

  objectGroup.removeSelected(ZSwcTree::GetType(), false);
  ASSERT_EQ(3, objectGroup.size());

  objectGroup.add(obj, false);
  objectGroup.add(obj2, false);
  objectGroup.add(obj3, false);

  objectGroup.removeObject(objSet.begin(), objSet.end(), false);
  ASSERT_FALSE(objectGroup.containsUnsync(obj));
  ASSERT_FALSE(objectGroup.containsUnsync(obj2));
  ASSERT_TRUE(objectGroup.containsUnsync(obj3));

  objectGroup.removeAllObject(false);
  ASSERT_TRUE(objectGroup.isEmpty());

  objectGroup.add(obj, false);
  objectGroup.add(obj2, false);
  objectGroup.add(obj3, false);
  objectGroup.add(tree, false);
  objectGroup.add(tree2, false);

  ASSERT_TRUE(objectGroup.hasObject(obj->getType()));
  ASSERT_TRUE(objectGroup.hasObject(obj->getTarget()));
  ASSERT_FALSE(objectGroup.hasObject(ZStackObject::TYPE_CIRCLE));
  ASSERT_TRUE(objectGroup.hasSelected());
  ASSERT_TRUE(objectGroup.hasSelected(ZSwcTree::GetType()));
  ASSERT_FALSE(objectGroup.hasSelected(ZObject3d::GetType()));

  QList<ZStackObject::EType> typeSet = objectGroup.getAllType();
//  foreach (ZStackObject::EType type, typeSet) {
//    std::cout << type << std::endl;
//  }

  ASSERT_EQ(2, typeSet.size());
  ASSERT_TRUE(typeSet.contains(ZSwcTree::GetType()));
  ASSERT_TRUE(typeSet.contains(ZObject3d::GetType()));

  objectGroup.removeObject(ZSwcTree::GetType());
  typeSet = objectGroup.getAllType();
  ASSERT_EQ(1, typeSet.size());
  ASSERT_FALSE(typeSet.contains(ZSwcTree::GetType()));
  ASSERT_TRUE(typeSet.contains(ZObject3d::GetType()));

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

  ZObject3d *obj7 = new ZObject3d;
  obj7->setSource("obj7");
  objectGroup.add(obj7, 3, false);
  ASSERT_EQ(obj7->getZOrder(), 3);

  ZObject3d *obj8 = new ZObject3d;
  obj8->setSource("obj8");
  objectGroup.add(obj8, true);
  ASSERT_EQ(6, obj8->getZOrder());

  ZObject3d *obj9 = new ZObject3d;
  obj9->setSource("obj7");
  objectGroup.add(obj9, true);
  ASSERT_EQ(3, obj9->getZOrder());

  ASSERT_EQ(6, objectGroup.getMaxZOrder());
  ASSERT_EQ(obj9, objectGroup.getLastObject(ZObject3d::GetType()));
}

#endif

#endif // ZSTACKOBJECTGROUPTEST_H
