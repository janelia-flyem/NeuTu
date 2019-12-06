#ifndef FLYEMUTILITIESTEST_H
#define FLYEMUTILITIESTEST_H

#include "ztestheader.h"
#include "zswctree.h"
#include "zflyemutilities.h"

#ifdef _USE_GTEST_

TEST(flyemutil, swc)
{
  ZSwcTree tree;

  ASSERT_EQ(int(0), flyem::GetMutationId(tree));
  ASSERT_EQ(int(0), flyem::GetSkeletonVersion(tree));

  tree.addComment("test");
  tree.addComment("${\"mutation id\": 15}");
  tree.addComment("${\"vskl\": 1}");

  ASSERT_EQ(15, flyem::GetMutationId(tree));
  ASSERT_EQ(1, flyem::GetSkeletonVersion(tree));
}

#endif

#endif // FLYEMUTILITIESTEST_H
