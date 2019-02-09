#ifndef Z3DSWCFILTERTEST_H
#define Z3DSWCFILTERTEST_H

#include "ztestheader.h"
#include "zswccolorparam.h"
#include "zswctree.h"
#include "z3dswcfilter.h"

#ifdef _USE_GTEST_

TEST(ZSwcColorParam, basic)
{

  ZSwcColorParam param;
  param.setPrefix("SWC");

  ZSwcTree *tree = new ZSwcTree;

  ZVec4Parameter& colorParam = param.getColorParameter(tree);
  ASSERT_EQ("SWC 1 Color", colorParam.name());

  ZVec4Parameter& colorParam2 = param.getColorParameter(tree);
  ASSERT_EQ("SWC 1 Color", colorParam2.name());

  ASSERT_TRUE(param.contains(tree));
  param.remove(tree);
  ASSERT_FALSE(param.contains(tree));

  ZSwcTree *tree2 = new ZSwcTree;
  ZVec4Parameter& colorParam3 = param.getColorParameter(tree2);
  ASSERT_EQ(&colorParam, &colorParam3);

  ZSwcTree *tree3 = new ZSwcTree;
  ZVec4Parameter& colorParam4 = param.getColorParameter(tree3);

  ZSwcTree *tree4 = new ZSwcTree;
  ZVec4Parameter& colorParam6 = param.getColorParameter(tree4);

  param.remove(tree3);
  ZSwcTree *tree5 = new ZSwcTree;
  ZVec4Parameter& colorParam5 = param.getColorParameter(tree5);
  ASSERT_EQ(&colorParam4, &colorParam5);
  ASSERT_NE(&colorParam4, &colorParam6);

  std::vector<ZSwcTree*> objList;
  objList.push_back(tree);
  objList.push_back(tree2);
  objList.push_back(tree3);

  param.update(objList.begin(), objList.end());
  ASSERT_EQ(param.getColorParameterPtr(objList[1]), &colorParam3);
  for (ZSwcTree *tree : objList) {
    ASSERT_TRUE(param.contains(tree));
  }
  ASSERT_FALSE(param.contains(tree4));
  ASSERT_FALSE(param.contains(tree5));

}

#endif

#endif // Z3DSWCFILTERTEST_H
