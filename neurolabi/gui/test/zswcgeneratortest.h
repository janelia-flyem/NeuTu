#ifndef ZSWCGENERATORTEST_H
#define ZSWCGENERATORTEST_H

#ifdef _USE_GTEST_
#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zswcrangeanalyzer.h"
#include "zswcnodezrangeselector.h"
#include "zswcnodeellipsoidrangeselector.h"
#include "zswcnodecompositeselector.h"
#include "swctreenode.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zswcsubtreefeatureanalyzer.h"
#include "zswcgenerator.h"

TEST(ZSwcGenerator, VoxelArray)
{
  ZVoxelArray voxelArray;
  voxelArray.append(0, 0, 0, 1.0);
  voxelArray.append(2, 0, 0, 1.0);
  voxelArray.append(4, 0, 0, 1.0);

  ZSwcTree * tree =
      ZSwcGenerator::createSwc(voxelArray, ZSwcGenerator::REGION_SAMPLING);
  ASSERT_EQ(3, tree->size());

  delete tree;

  voxelArray.append(5, 0, 0, 2.0);
  tree = ZSwcGenerator::createSwc(voxelArray, ZSwcGenerator::REGION_SAMPLING);
  ASSERT_EQ(3, tree->size());
  delete tree;

  voxelArray.clear();
  voxelArray.append(0, 0, 0, 1.0);
  voxelArray.append(2, 0, 0, 1.0);
  voxelArray.append(4, 0, 0, 3.0);
  voxelArray.append(6, 0, 0, 3.1);
  voxelArray.append(8, 0, 0, 1.0);
  voxelArray.append(10, 0, 0, 3.0);

  tree = ZSwcGenerator::createSwc(voxelArray, ZSwcGenerator::REGION_SAMPLING);
  tree->print();
  delete tree;
}


#endif

#endif // ZSWCGENERATORTEST_H
