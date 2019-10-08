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

TEST(ZStackDocObjectUpdateTest, Callback)
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

TEST(ZStackDocObjectUpdateTest, Apply)
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


#endif

#endif // ZSTACKDOCOBJECTUPDATETEST_H
