#ifndef FLYEMBODYSELECTIONMANAGERTEST_H
#define FLYEMBODYSELECTIONMANAGERTEST_H

#include <tuple>

#include "ztestheader.h"
#include "flyem/flyembodyselectionmanager.h"
#include "flyem/zflyembodymanager.h"

#ifdef _USE_GTEST_

TEST(FlyEmBodySelectionManager, Basic)
{
  FlyEmBodySelectionManager bsm;
  bsm.selectBody(1);
  ASSERT_TRUE(bsm.isSelected(1));

  bsm.selectBody(ZFlyEmBodyManager::EncodeSupervoxel(1));
  ASSERT_TRUE(bsm.isSelected(ZFlyEmBodyManager::EncodeSupervoxel(1)));

  bsm.deselectBody(1);
  ASSERT_FALSE(bsm.isSelected(1));
  ASSERT_TRUE(bsm.isSelected(ZFlyEmBodyManager::EncodeSupervoxel(1)));

  bsm.deselectAll();
  ASSERT_EQ(0, int(bsm.getLastSelected()));
  ASSERT_FALSE(bsm.isSelected(ZFlyEmBodyManager::EncodeSupervoxel(1)));

  bsm.selectBody(1);
  bsm.selectBody(2);
  ASSERT_EQ(2, int(bsm.getLastSelected()));
  bsm.selectBody(3);
  ASSERT_EQ(3, int(bsm.getLastSelected()));

  std::set<uint64_t> selected;
  std::set<uint64_t> deselected;

  std::tie(selected, deselected) = bsm.flushSeletionChange();
  ASSERT_TRUE(deselected.empty());
  ASSERT_EQ(3, int(selected.size()));

  bsm.selectBody(4);
  ASSERT_EQ(4, int(bsm.getLastSelected()));
  bsm.selectBody(3);
  ASSERT_EQ(3, int(bsm.getLastSelected()));

  bsm.deselectBody(1);
  ASSERT_EQ(0, int(bsm.getLastSelected()));
  bsm.deselectBody(2);
  std::tie(selected, deselected) = bsm.flushSeletionChange();

  ASSERT_EQ(2, int(deselected.size()));
  ASSERT_EQ(1, int(deselected.count(1)));
  ASSERT_EQ(1, (int) deselected.count(2));
  ASSERT_EQ(1, int(selected.size()));
  ASSERT_EQ(1, int(selected.count(4)));

  std::tie(selected, deselected) = bsm.flushSeletionChange();
  ASSERT_TRUE(selected.empty());
  ASSERT_TRUE(deselected.empty());

  bsm.deselectAll();
  std::tie(selected, deselected) = bsm.flushSeletionChange();
  ASSERT_TRUE(selected.empty());
  ASSERT_EQ(2, (int) deselected.size());
  ASSERT_EQ(1, (int) deselected.count(3));
  ASSERT_EQ(1, (int) deselected.count(4));
}


#endif

#endif // FLYEMBODYSELECTIONMANAGERTEST_H
