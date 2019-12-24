#ifndef ZSTACKDOCOBJECTUPDATETEST_H
#define ZSTACKDOCOBJECTUPDATETEST_H

#include "ztestheader.h"
#include "zswctree.h"
#include "mvc/zstackdocobjectupdate.h"

#ifdef _USE_GTEST_

TEST(ZStackDocObjectUpdate, Init)
{
  ZSwcTree *tree = new ZSwcTree;

  ZStackDocObjectUpdate u(tree, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::ADD_UNIQUE, u.getAction());
  ASSERT_EQ(static_cast<ZStackObject*>(tree), u.getObject());
  ASSERT_TRUE(u.isMergable());
}

/*
class ZStackDocObjectUpdateTest : public ::testing::Test
{
public:
  ZSwcTree *tree = new ZSwcTree;
  ZStackDocObjectUpdate u;

  ZStackDocObjectUpdateTest() : u(tree, ZStackDocObjectUpdate::EAction::CALLBACK) {
  }

  void SetUp() override;
};

void ZStackDocObjectUpdateTest::SetUp()
{
}
*/

TEST(ZStackDocObjectUpdate, Callback)
{
  ZSwcTree *tree = new ZSwcTree;
  ZStackDocObjectUpdate u(tree, ZStackDocObjectUpdate::EAction::CALLBACK);
  u.apply();
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::CALLBACK, u.getAction());
  ASSERT_EQ(static_cast<ZStackObject*>(tree), u.getObject());

  u.setCallback([](ZStackObject* obj) {
    std::cout << obj->getTypeName() << std::endl;
    delete obj;
  });
  u.apply();
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::NONE, u.getAction());
  ASSERT_EQ(nullptr, u.getObject());

  u.apply();
}

TEST(ZStackDocObjectUpdate, Apply)
{
  ZSwcTree *tree = new ZSwcTree;
  ZStackDocObjectUpdate u(tree, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
  u.apply();
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::ADD_UNIQUE, u.getAction());
  ASSERT_EQ(static_cast<ZStackObject*>(tree), u.getObject());

  u.apply([](ZStackObject* obj, ZStackDocObjectUpdate::EAction action) {
    std::cout << neutu::EnumValue(action) << " " << obj->getTypeName() << std::endl;
    delete obj;
  });
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::NONE, u.getAction());
  ASSERT_EQ(nullptr, u.getObject());
}

namespace {

void merge_update(QList<ZStackDocObjectUpdate *>::reverse_iterator firstIter,
                  QList<ZStackDocObjectUpdate *>::reverse_iterator lastIter)
{
  //Rules for processing actions of the same object:
  //  If the last action is delete, then all the other actions will be invalidated
  QMap<ZStackObject*, ZStackDocObjectUpdate::EAction> actionMap;
  for (auto iter = firstIter; iter != lastIter; ++iter) {
    ZStackDocObjectUpdate *u = *iter;
    if (!actionMap.contains(u->getObject())) {
      actionMap[u->getObject()] = u->getAction();
    } else {
      ZStackDocObjectUpdate::EAction laterAction = actionMap[u->getObject()];
      if (laterAction > u->getAction()) {
        if (laterAction == ZStackDocObjectUpdate::EAction::RECYCLE ||
            laterAction == ZStackDocObjectUpdate::EAction::EXPEL ||
            laterAction == ZStackDocObjectUpdate::EAction::KILL) {
          u->reset();
        }
      } else { //upgrade action
        actionMap[u->getObject()] = u->getAction();
      }
    }
  }
}

}

TEST(ZStackDocObjectUpdate, Merge)
{
  QList<ZStackDocObjectUpdate*> updateList;
  ZSwcTree *tree = new ZSwcTree;
  updateList.append(new ZStackDocObjectUpdate(
               tree, ZStackDocObjectUpdate::EAction::ADD_UNIQUE));
  updateList.append(new ZStackDocObjectUpdate(
               tree, ZStackDocObjectUpdate::EAction::KILL));
  updateList.append(new ZStackDocObjectUpdate(
               tree, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE));
  updateList.append(new ZStackDocObjectUpdate(
                      nullptr, ZStackDocObjectUpdate::EAction::CALLBACK));
  updateList.append(new ZStackDocObjectUpdate(
               new ZSwcTree, ZStackDocObjectUpdate::EAction::KILL));
  updateList.append(new ZStackDocObjectUpdate(
               new ZSwcTree, ZStackDocObjectUpdate::EAction::ADD_BUFFER));

  merge_update(updateList.rbegin(), updateList.rend());

  ASSERT_EQ(ZStackDocObjectUpdate::EAction::NONE, updateList[0]->getAction());
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::KILL, updateList[1]->getAction());
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE,
            updateList[2]->getAction());
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::CALLBACK,
            updateList[3]->getAction());
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::KILL,
            updateList[4]->getAction());
  ASSERT_EQ(ZStackDocObjectUpdate::EAction::ADD_BUFFER,
            updateList[5]->getAction());

}


#endif

#endif // ZSTACKDOCOBJECTUPDATETEST_H
