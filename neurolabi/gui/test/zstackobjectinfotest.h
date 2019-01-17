#ifndef ZSTACKOBJECTINFOTEST_H
#define ZSTACKOBJECTINFOTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackobjectinfo.h"
#include "zswctree.h"

#ifdef _USE_GTEST_
TEST(ZStackObjectInfo, Basic)
{
  ZStackObjectInfo info;
  info.setType(ZStackObject::EType::CUBE);
  ASSERT_EQ(ZStackObject::EType::CUBE, info.getType());

  ZSwcTree tree;
  info.set(tree);
  ASSERT_EQ(ZStackObject::EType::SWC, info.getType());
  ASSERT_EQ(ZSwcTree::GetDefaultTarget(), info.getTarget());
  ASSERT_EQ(ZStackObjectRole::ROLE_NONE, info.getRole().getRole());

  tree.setRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
  info.set(tree);
  ASSERT_EQ(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR, info.getRole().getRole());

  ZStackObjectInfo info2;
  ASSERT_FALSE(info == info2);

  info2.set(tree);
  ASSERT_EQ(info, info2);
}

TEST(ZStackObjectInfoSet, Basic)
{
  ZStackObjectInfoSet infoSet;
  ASSERT_TRUE(infoSet.isEmpty());
  ASSERT_TRUE(infoSet.getType().empty());
  ASSERT_TRUE(infoSet.getTarget().isEmpty());
  ASSERT_FALSE(infoSet.contains(ZStackObject::EType::CUBE));
  ASSERT_FALSE(infoSet.contains(ZStackObject::EType::UNIDENTIFIED));
  ASSERT_FALSE(infoSet.contains(ZStackObject::TARGET_NULL));
  ASSERT_FALSE(infoSet.contains(ZStackObjectRole::ROLE_NONE));

  ZStackObjectInfo info;
  infoSet.add(info);
  ASSERT_FALSE(infoSet.isEmpty());
  ASSERT_FALSE(infoSet.getType().empty());
  ASSERT_TRUE(infoSet.getTarget().isEmpty());
  ASSERT_FALSE(infoSet.contains(ZStackObject::EType::SWC));
  ASSERT_TRUE(infoSet.contains(ZStackObject::EType::UNIDENTIFIED));
  ASSERT_TRUE(infoSet.contains(ZStackObject::TARGET_NULL));
  ASSERT_TRUE(infoSet.contains(ZStackObjectRole::ROLE_NONE));

  ZSwcTree tree;
  tree.setRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
  infoSet.add(tree);
  ASSERT_FALSE(infoSet.getTarget().isEmpty());
  ASSERT_TRUE(infoSet.contains(ZStackObject::EType::SWC));
  ASSERT_TRUE(infoSet.contains(ZStackObject::EType::UNIDENTIFIED));
  ASSERT_TRUE(infoSet.contains(ZSwcTree::GetDefaultTarget()));
  ASSERT_TRUE(infoSet.contains(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR));

  ASSERT_TRUE(infoSet.hasObjectModified(
                ZStackObject::EType::SWC, ZStackObjectInfo::STATE_ADDED));
  ASSERT_FALSE(infoSet.hasObjectModified(
                 ZStackObject::EType::CUBE, ZStackObjectInfo::STATE_ADDED));
  ASSERT_FALSE(infoSet.onlyVisibilityChanged(ZStackObject::EType::SWC));
  ASSERT_FALSE(infoSet.onlyVisibilityChanged(ZStackObject::EType::CUBE));

  infoSet.add(ZStackObject::EType::CUBE,
              ZStackObjectInfo::STATE_VISIBITLITY_CHANGED);
  ASSERT_TRUE(infoSet.onlyVisibilityChanged(ZStackObject::EType::CUBE));
}

#endif

#endif // ZSTACKOBJECTINFOTEST_H
