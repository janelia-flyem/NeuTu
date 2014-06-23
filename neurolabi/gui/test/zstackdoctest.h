#ifndef ZSTACKDOCTEST_H
#define ZSTACKDOCTEST_H

#include "ztestheader.h"
#include "zstackdoc.h"
#include "neutubeconfig.h"
#include "zobject3d.h"

#ifdef _USE_GTEST_
TEST(ZStackDoc, Basic)
{
  ZStackDoc doc(NULL, NULL);
  EXPECT_TRUE(doc.isEmpty());
  EXPECT_FALSE(doc.hasStackData());
  EXPECT_FALSE(doc.hasStackMask());

  EXPECT_TRUE(doc.stackSourcePath().empty());

}

TEST(ZStackDoc, Swc)
{
  ZStackDoc doc(NULL, NULL);
  doc.readSwc((GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/1.swc").c_str());
  doc.saveSwc(GET_TEST_DATA_DIR + "/test1.swc");
  ASSERT_EQ(1, doc.getSwcList().size());
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test1.swc", doc.getSwcList().front()->source());

  ZSwcTree *tree = new ZSwcTree;
  tree->load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");
  doc.addSwcTree(tree);
  ASSERT_EQ(2, doc.getSwcList().size());

  doc.saveSwc(GET_TEST_DATA_DIR + "/test2.swc");
  ASSERT_EQ(1, doc.getSwcList().size());
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test2.swc", doc.getSwcList().front()->source());
}

TEST(ZStackDoc, Player)
{
  ZStackDoc doc(NULL, NULL);
  ZObject3d *obj = new ZObject3d;
  obj->append(0, 0, 0);
  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  ASSERT_EQ(1, doc.getPlayerList().size());
  ASSERT_EQ(1, doc.getObjectList().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_NONE);
  ASSERT_EQ(1, doc.getPlayerList().size());
  ASSERT_EQ(2, doc.getObjectList().size());

  doc.removeObject(ZDocPlayer::ROLE_SEED, false);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(1, doc.getObjectList().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  ASSERT_EQ(2, doc.getPlayerList().size());
  ASSERT_EQ(3, doc.getObjectList().size());

  doc.removeObject(ZDocPlayer::ROLE_SEED, false);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(2, doc.getObjectList().size());

  doc.removeAllObject(false);
  ASSERT_EQ(0, doc.getObjectList().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  doc.removeObject(ZDocPlayer::ROLE_SEED, true);
  ASSERT_EQ(0, doc.getObjectList().size());

  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_NONE);
  ASSERT_EQ(3, doc.getPlayerList().size());
  ASSERT_EQ(4, doc.getObjectList().size());

  doc.removeAllObject(true);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(0, doc.getObjectList().size());


  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_NONE);

  ASSERT_TRUE(doc.hasPlayer(ZDocPlayer::ROLE_DISPLAY));
  ASSERT_TRUE(doc.hasPlayer(ZDocPlayer::ROLE_SEED));
  ASSERT_FALSE(doc.hasPlayer(ZDocPlayer::ROLE_NONE));
  ASSERT_FALSE(doc.hasPlayer(ZDocPlayer::ROLE_3DPAINT));
  ASSERT_EQ(2, doc.getPlayerList(ZDocPlayer::ROLE_SEED).size());

  std::cout << "ZStackDocTest: v4" << std::endl;
}

#endif

#endif // ZSTACKDOCTEST_H
