#ifndef ZOBJECT3DTEST_H
#define ZOBJECT3DTEST_H

#include "ztestheader.h"
#include "zobject3d.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

TEST(ZObject3d, duplicateAcrossZ) {
  ZObject3d obj;
  obj.append(1, 1, 0);
  obj.duplicateAcrossZ(5);
  EXPECT_EQ(5, (int) obj.size());
  EXPECT_EQ(1, obj.x(0));
  EXPECT_EQ(1, obj.y(0));
  EXPECT_EQ(0, obj.z(0));
  EXPECT_EQ(1, obj.x(1));
  EXPECT_EQ(1, obj.y(1));
  EXPECT_EQ(1, obj.z(1));
}

TEST(ZObject3d, Operation)
{
  ZObject3d obj;
  obj.append(1, 2, 3);
  obj.append(4, 5, 6);
  obj.reverse();
  //obj.print();

  ASSERT_EQ(4, obj.x(0));
  ASSERT_EQ(5, obj.y(0));
  ASSERT_EQ(6, obj.z(0));
  ASSERT_EQ(1, obj.x(1));
  ASSERT_EQ(2, obj.y(1));
  ASSERT_EQ(3, obj.z(1));

  obj.clear();
  obj.append(1, 2, 3);
  obj.append(4, 5, 6);
  obj.append(7, 8, 9);
  obj.reverse();
  //obj.print();
  ASSERT_EQ(7, obj.x(0));
  ASSERT_EQ(8, obj.y(0));
  ASSERT_EQ(9, obj.z(0));

  obj.clear();
  obj.append(1, 2, 3);
  obj.append(4, 5, 6);
  obj.append(7, 8, 9);
  obj.append(10, 11, 12);
  obj.reverse();
  //obj.print();
  ASSERT_EQ(10, obj.x(0));
  ASSERT_EQ(11, obj.y(0));
  ASSERT_EQ(12, obj.z(0));
}

TEST(ZObject3d, upsample)
{
  ZObject3d obj;
  obj.append(1, 2, 3);
  obj.upSample(1, 1, 1);

  ASSERT_EQ(8, (int) obj.size());
  ASSERT_EQ(2, obj.x(0));
  ASSERT_EQ(4, obj.y(0));
  ASSERT_EQ(6, obj.z(0));

  obj.clear();
  obj.append(1, 2, 3);
  obj.upSample(1, 2, 3);
  ASSERT_EQ(24, (int) obj.size());
  ASSERT_EQ(2, obj.x(0));
  ASSERT_EQ(6, obj.y(0));
  ASSERT_EQ(12, obj.z(0));

  obj.clear();
  obj.append(1, 2, 3);
  obj.append(2, 2, 3);
  obj.append(2, 3, 1);
  obj.upSample(1, 1, 1);

  //obj.print();
}

#endif


#endif // ZOBJECT3DTEST_H
