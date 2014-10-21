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
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test1.swc", doc.getSwcList().front()->getSource());

  ZSwcTree *tree = new ZSwcTree;
  tree->load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");
  doc.addSwcTree(tree);
  ASSERT_EQ(2, doc.getSwcList().size());

  doc.saveSwc(GET_TEST_DATA_DIR + "/test2.swc");
  ASSERT_EQ(1, doc.getSwcList().size());
  ASSERT_EQ(GET_TEST_DATA_DIR + "/test2.swc", doc.getSwcList().front()->getSource());
}

TEST(ZStackDoc, Player)
{
  ZStackDoc doc(NULL, NULL);
  ZObject3d *obj = new ZObject3d;
  obj->append(0, 0, 0);
  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  ASSERT_EQ(1, doc.getPlayerList().size());
  ASSERT_EQ(1, doc.getObjectGroup().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_NONE);
  ASSERT_EQ(1, doc.getPlayerList().size());
  ASSERT_EQ(2, doc.getObjectGroup().size());

  doc.removeObject(ZDocPlayer::ROLE_SEED, false);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(1, doc.getObjectGroup().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  ASSERT_EQ(2, doc.getPlayerList().size());
  ASSERT_EQ(3, doc.getObjectGroup().size());

  doc.removeObject(ZDocPlayer::ROLE_SEED, false);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(2, doc.getObjectGroup().size());

  doc.removeAllObject(false);
  ASSERT_EQ(0, doc.getObjectGroup().size());

  doc.addObject(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_SEED);
  doc.removeObject(ZDocPlayer::ROLE_SEED, true);
  ASSERT_EQ(0, doc.getObjectGroup().size());

  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_DISPLAY);
  doc.addObject(new ZObject3d, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_NONE);
  ASSERT_EQ(3, doc.getPlayerList().size());
  ASSERT_EQ(4, doc.getObjectGroup().size());

  doc.removeAllObject(true);
  ASSERT_EQ(0, doc.getPlayerList().size());
  ASSERT_EQ(0, doc.getObjectGroup().size());


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

  std::cout << "ZStackDocTest: v5" << std::endl;

  ZObject3d *obj2 = new ZObject3d;
  obj2->append(1, 2, 3);
  obj2->append(4, 5, 6);
  doc.addObject(obj2, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_3DGRAPH_DECORATOR);

  Z3DGraph graph = doc.get3DGraphDecoration();
  ASSERT_EQ(1, (int) graph.getNodeNumber());
  ASSERT_EQ(0, (int) graph.getEdgeNumber());

  ZObject3d *obj3 = new ZObject3d;
  obj3->append(7, 8, 9);
  for (int i = 0; i < 10; ++i) {
    obj3->append(i, 10, 10);
  }
  doc.addObject(obj3, NeuTube::Documentable_OBJ3D,
                ZDocPlayer::ROLE_3DGRAPH_DECORATOR);

  graph = doc.get3DGraphDecoration();
  ASSERT_EQ(5, (int) graph.getNodeNumber());
  ASSERT_EQ(0, (int) graph.getEdgeNumber());
}

#endif

#endif // ZSTACKDOCTEST_H
