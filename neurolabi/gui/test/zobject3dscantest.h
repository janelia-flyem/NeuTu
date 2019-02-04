#ifndef ZOBJECT3DSCANTEST_H
#define ZOBJECT3DSCANTEST_H

#include "ztestheader.h"
#include "zobject3dscan.h"
#include "neutubeconfig.h"
#include "zgraph.h"
#include "tz_iarray.h"
#include "zdebug.h"
#include "zdoublevector.h"
#include "zstack.hxx"
#include "zstackfactory.h"
#include "geometry/zintcuboid.h"
#include "misc/miscutility.h"

#ifdef _USE_GTEST_

static void createStripe(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 5);
}

static void createStripe2(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5, false);
  stripe->addSegment(0, 1, false);
  stripe->addSegment(3, 1, false);
}

static void createStripe3(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1);
}

static void createStripe4(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1, false);
}

TEST(ZObject3dStripe, TestGetProperty) {
  {
    ZObject3dStripe stripe;
    createStripe(&stripe);
    ASSERT_EQ(stripe.getMinX(), 0);
    ASSERT_EQ(stripe.getMaxX(), 5);
    ASSERT_EQ(stripe.getSegmentNumber(), 2);
    ASSERT_EQ((int) stripe.getSize(), 2);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ((int) stripe.getVoxelNumber(), 5);
    ASSERT_EQ(24, (int) stripe.getByteCount());

    createStripe2(&stripe);
    ASSERT_EQ(stripe.getSegmentNumber(), 2);
    ASSERT_EQ((int) stripe.getSize(), 2);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ((int) stripe.getVoxelNumber(), 7);
    ASSERT_EQ(24, (int) stripe.getByteCount());

    stripe.canonize();
    ASSERT_EQ(stripe.getMinX(), 0);
    ASSERT_EQ(stripe.getMaxX(), 5);
    ASSERT_EQ(stripe.getSegmentNumber(), 1);
    ASSERT_EQ((int) stripe.getSize(), 1);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ((int) stripe.getVoxelNumber(), 6);
    ASSERT_EQ(16, (int) stripe.getByteCount());

    createStripe3(&stripe);
    ASSERT_EQ(stripe.getMinX(), 0);
    ASSERT_EQ(stripe.getMaxX(), 5);
    ASSERT_EQ(stripe.getSegmentNumber(), 1);
    ASSERT_EQ((int) stripe.getSize(), 1);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ((int) stripe.getVoxelNumber(), 6);
    ASSERT_EQ(16, (int) stripe.getByteCount());

    createStripe4(&stripe);
    ASSERT_EQ(stripe.getSegmentNumber(), 2);
    ASSERT_EQ((int) stripe.getSize(), 2);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ(24, (int) stripe.getByteCount());

    stripe.canonize();
    ASSERT_EQ(stripe.getMinX(), 0);
    ASSERT_EQ(stripe.getMaxX(), 5);
    ASSERT_EQ(stripe.getSegmentNumber(), 1);
    ASSERT_EQ((int) stripe.getSize(), 1);
    ASSERT_EQ(stripe.getY(), 3);
    ASSERT_EQ(stripe.getZ(), 5);
    ASSERT_EQ((int) stripe.getVoxelNumber(), 6);
    ASSERT_EQ(16, (int) stripe.getByteCount());
  }

  {
    ZObject3dStripe stripe;
    ASSERT_FALSE(stripe.hasVoxel());
    stripe.addSegment(0, 1);
    ASSERT_TRUE(stripe.hasVoxel());
  }
}

TEST(ZObject3dStripe, TestUnify) {
  ZObject3dStripe stripe;
  stripe.setY(3);
  stripe.setZ(5);
  stripe.addSegment(3, 5);

  ZObject3dStripe stripe2;
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(6, 7);

  EXPECT_FALSE(stripe.equalsLiterally(stripe2));

  ASSERT_TRUE(stripe.unify(stripe2));
  ASSERT_EQ(1, stripe.getSegmentNumber());
  ASSERT_EQ(3, stripe.getMinX());
  ASSERT_EQ(7, stripe.getMaxX());
  ASSERT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.setY(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  ASSERT_EQ(1, stripe.getSegmentNumber());
  ASSERT_EQ(3, stripe.getMinX());
  ASSERT_EQ(7, stripe.getMaxX());
  ASSERT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.setY(3);
  stripe2.setZ(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  ASSERT_EQ(1, stripe.getSegmentNumber());
  ASSERT_EQ(3, stripe.getMinX());
  ASSERT_EQ(7, stripe.getMaxX());
  ASSERT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(1, 7);
  ASSERT_TRUE(stripe.unify(stripe2));
  ASSERT_EQ(1, stripe.getSegmentNumber());
  ASSERT_EQ(1, stripe.getMinX());
  ASSERT_EQ(7, stripe.getMaxX());
  ASSERT_EQ(7, (int) stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(9, 10);
  ASSERT_TRUE(stripe.unify(stripe2));
  ASSERT_EQ(2, stripe.getSegmentNumber());
  ASSERT_EQ(1, stripe.getMinX());
  ASSERT_EQ(10, stripe.getMaxX());
  ASSERT_EQ(9, (int) stripe.getVoxelNumber());
}

TEST(ZObject3dStripe, TestIO) {
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "w");
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  stripe.write(fp);

  fclose(fp);

  fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "r");
  ZObject3dStripe stripe2;
  stripe2.read(fp);
  fclose(fp);

  ASSERT_TRUE(stripe.equalsLiterally(stripe));
  ASSERT_TRUE(stripe.equalsLiterally(stripe2));
}

bool isSorted(const ZObject3dStripe &stripe)
{
  if (!stripe.isEmpty()) {
    for (int i = 0; i < stripe.getSegmentNumber() - 1; ++i) {
      if (stripe.getSegmentStart(i) > stripe.getSegmentStart(i + 1)) {
        return false;
      } else if (stripe.getSegmentStart(i) == stripe.getSegmentStart(i + 1)) {
        if (stripe.getSegmentEnd(i) > stripe.getSegmentEnd(i + 1)) {
          return false;
        }
      }
    }
  }

  return true;
}

TEST(ZObject3dStripe, TestSort) {
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  ASSERT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 2, false);
  stripe.addSegment(3, 5, false);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  ASSERT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.setY(0);
  stripe.setZ(1);
  stripe.addSegment(0, 1, false);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 0, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);

  stripe.sort();

  ASSERT_TRUE(isSorted(stripe));
}

TEST(ZObject3dStripe, TestCanonize) {
  ZObject3dStripe stripe;

  stripe.setY(0);
  stripe.setZ(1);

  ZObject3dStripe stripe2;
  stripe2.setY(2);
  stripe2.setZ(3);
  ASSERT_TRUE(stripe.unify(stripe2));
  ASSERT_EQ(2, stripe.getY());
  ASSERT_EQ(3, stripe.getZ());

  stripe.addSegment(0, 1);
  stripe2.setY(0);
  ASSERT_FALSE(stripe.unify(stripe2));

  stripe.clearSegment();
  createStripe2(&stripe);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.canonize();
  ASSERT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(3, 4, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);
  ASSERT_TRUE(stripe.isCanonized());

  stripe.canonize();
  ASSERT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(7, 8, false);
  ASSERT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(7, 8, false);
  stripe.addSegment(1, 2, false);
  ASSERT_FALSE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(1, 7, false);
  stripe.addSegment(7, 8, false);
  ASSERT_TRUE(stripe.isCanonized());

  stripe.canonize();
  ASSERT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(4, 7, false);
  stripe.addSegment(7, 8, false);
  stripe.addSegment(4, 8, false);
  stripe.addSegment(5, 6, false);
  stripe.addSegment(10, 15, false);
  stripe.addSegment(19, 15, false);
  ASSERT_TRUE(stripe.isCanonized());
  ASSERT_EQ(2, stripe.getSegmentNumber());
  ASSERT_EQ(15, (int) stripe.getVoxelNumber());
}

TEST(ZObject3dStripe, Relation)
{
  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s2.setY(0);
    s2.setZ(1);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(0);
    s2.setZ(2);
    s2.addSegment(0, 1);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(0);
    s2.setZ(2);
    s2.addSegment(2, 3);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(1);
    s2.setZ(2);
    s2.addSegment(2, 3);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(1);
    s2.setZ(2);
    s2.addSegment(3, 4);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(2);
    s2.setZ(1);
    s2.addSegment(3, 4);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_FALSE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_FALSE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(2, 3);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 1);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 2);

    ASSERT_TRUE(s1.hasOverlap(s2));
    ASSERT_TRUE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 2);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 1);

    ASSERT_TRUE(s1.hasOverlap(s2));
    ASSERT_TRUE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 2);

    s2.setY(0);
    s2.setZ(2);
    s2.addSegment(1, 1);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 2);

    s2.setY(1);
    s2.setZ(1);
    s2.addSegment(1, 1);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 5);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 10);

    ASSERT_TRUE(s1.hasOverlap(s2));
    ASSERT_TRUE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(1);
    s1.setZ(1);
    s1.addSegment(0, 5);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 10);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(0, 5);

    s2.setY(0);
    s2.setZ(2);
    s2.addSegment(1, 10);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(5, 10);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 1);
    s2.addSegment(2, 3);
    s2.addSegment(4, 5);

    ASSERT_TRUE(s1.hasOverlap(s2));
    ASSERT_TRUE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(5, 10);

    s2.setY(1);
    s2.setZ(1);
    s2.addSegment(1, 1);
    s2.addSegment(2, 3);
    s2.addSegment(4, 5);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(5, 10);

    s2.setY(0);
    s2.setZ(2);
    s2.addSegment(1, 1);
    s2.addSegment(2, 3);
    s2.addSegment(4, 5);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(5, 10);
    s1.addSegment(0, 0);
    s1.addSegment(-5, -10);

    s2.setY(0);
    s2.setZ(1);
    s2.addSegment(1, 1);
    s2.addSegment(2, 3);
    s2.addSegment(4, 5);

    ASSERT_TRUE(s1.hasOverlap(s2));
    ASSERT_TRUE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }

  {
    ZObject3dStripe s1;
    ZObject3dStripe s2;

    s1.setY(0);
    s1.setZ(1);
    s1.addSegment(5, 10);
    s1.addSegment(0, 0);
    s1.addSegment(-5, -10);

    s2.setY(1);
    s2.setZ(1);
    s2.addSegment(1, 1);
    s2.addSegment(2, 3);
    s2.addSegment(4, 5);

    ASSERT_FALSE(s1.hasOverlap(s2));
    ASSERT_FALSE(s2.hasOverlap(s1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D1));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D2));
    ASSERT_TRUE(s1.isAdjacentTo(s2, neutube::EStackNeighborhood::D3));
    ASSERT_TRUE(s2.isAdjacentTo(s1, neutube::EStackNeighborhood::D3));
  }
}

static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}

static void createObject2(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0);
  obj->addSegment(0, 1, false);
  obj->addSegment(0, 5, false);
  obj->addSegment(0, 8, false);
  obj->addStripe(0, 1);
  obj->addSegment(3, 3, false);
  obj->addSegment(0, 1, false);
  obj->addStripe(0, 1);
  obj->addSegment(5, 7, false);
}

static void createObject3(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0);
  obj->addSegment(0, 1);
  obj->addSegment(0, 5);
  obj->addSegment(0, 8);
  obj->addStripe(0, 1);
  obj->addSegment(3, 3);
  obj->addSegment(0, 1);
  obj->addStripe(0, 1);
  obj->addSegment(5, 7);
}

TEST(ZObject3dScan, TestGetProperty) {
  ZObject3dScan obj;
  createObject(&obj);
//  obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  ZIntCuboid box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);
  ASSERT_EQ(64, (int) obj.getByteCount());

  obj.canonize();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);
  ASSERT_EQ(64, (int) obj.getByteCount());

  createObject2(&obj);
//  obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);
  ASSERT_EQ(48, (int) obj.getByteCount());

//  obj.isEmpty();
  obj.canonize();
  obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);
  ASSERT_EQ(48, (int) obj.getByteCount());

  createObject3(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);
  ASSERT_EQ(48, (int) obj.getByteCount());

//  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
//  size_t area = obj.getSurfaceArea();
//  std::cout << area << std::endl;
//  std::cout << obj.getVoxelNumber() << std::endl;

}

TEST(ZObject3dScan, Basic)
{
  ZObject3dScan obj;
  ASSERT_FALSE(obj.hasVoxel());
  obj.addStripe(0, 1);
  obj.addStripe(3, 4);
  ASSERT_FALSE(obj.hasVoxel());

  obj.addSegment(0, 0, 1, 2);
  ASSERT_TRUE(obj.hasVoxel());
}

TEST(ZObject3dScan, TestAddSegment) {
  ZObject3dScan obj;
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(3, 4, false);

  ASSERT_TRUE(obj.isCanonized());

  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  ASSERT_TRUE(obj.isCanonized());

  obj.addSegment(5, 6, false);
  ASSERT_TRUE(obj.isCanonized());

  obj.addSegment(3, 6, false);
  obj.print();
  ASSERT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  obj.addSegment(3, 6, false);
  obj.print();
  ASSERT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(7, 8, false);
  obj.addSegment(3, 6, false);
  obj.print();
  ASSERT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(1, 0, 3, 4, false);

  ASSERT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addSegment(1, 0, 5, 6, false);
  obj.addSegment(1, 0, 7, 8, false);
  obj.addSegment(1, 0, 3, 6, false);
  ASSERT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addSegment(1, 0, 5, 6, false);
  obj.addSegment(1, 0, 7, 8, false);
  obj.addSegment(1, 0, 3, 4, false);
  ASSERT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addSegment(1, 0, 5, 6, false);
  obj.addSegment(1, 0, 7, 8, false);
  obj.addSegment(1, 0, 3, 3, false);
  ASSERT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addSegment(0, 0, 0, 1, false);
  obj.addSegment(0, 0, 0, 5, false);
  obj.addSegment(0, 0, 0, 8, false);
  obj.addSegment(0, 1, 3, 3, false);
  obj.addSegment(0, 1, 0, 1, false);
  obj.addSegment(0, 1, 5, 7, false);
  ASSERT_EQ(2, (int) obj.getStripeNumber());
  ASSERT_EQ(15, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 0, 5);
  obj.addSegment(0, 0, 0, 8);
  obj.addSegment(0, 1, 3, 3);
  obj.addSegment(0, 1, 0, 1);
  obj.addSegment(0, 1, 5, 7);
  ASSERT_EQ(2, (int) obj.getStripeNumber());
  ASSERT_EQ(15, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, Appender) {
  {
    ZObject3dScan obj;
    ZObject3dScan::Appender appender(&obj);
    appender.addSegment(1, 0, 1, 2);
    appender.addSegment(3, 4);

    ASSERT_TRUE(obj.isCanonized());
  }

  {
    ZObject3dScan obj;
    ZObject3dScan::Appender appender(&obj);

    appender.addSegment(0, 0, 0, 1);
    appender.addSegment(0, 0, 0, 5);
    appender.addSegment(0, 0, 0, 8);
    appender.addSegment(0, 1, 3, 3);
    appender.addSegment(0, 1, 0, 1);
    appender.addSegment(0, 1, 5, 7);

    ASSERT_EQ(2, (int) obj.getStripeNumber());
    ASSERT_EQ(15, (int) obj.getVoxelNumber());
  }

  {
    ZObject3dScan obj;
    ZObject3dScan::Appender appender(&obj);

    appender.addSegment(0, 0, 0, 1);
    appender.addSegment(0, 1, 3, 3);
    appender.addSegment(0, 0, 0, 5);
    appender.addSegment(0, 1, 5, 7);
    appender.addSegment(0, 0, 0, 8);
    appender.addSegment(0, 1, 0, 1);

    ASSERT_EQ(2, (int) obj.getStripeNumber());
    ASSERT_EQ(15, (int) obj.getVoxelNumber());
  }

}

TEST(ZObject3dScan, downsample) {
  ZObject3dScan obj;
  createObject(&obj);

  obj.downsample(1, 1, 1);
  ASSERT_EQ(1, (int) obj.getStripeNumber());
  ASSERT_EQ(3, (int) obj.getVoxelNumber());

  createObject(&obj);
  obj.print();
  obj.downsampleMax(1, 1, 1);
  obj.print();
  ASSERT_EQ(1, (int) obj.getStripeNumber());
  ASSERT_EQ(5, (int) obj.getVoxelNumber());

  createObject(&obj);
  obj.downsampleMax(1, 0, 0);
  ASSERT_EQ(2, (int) obj.getStripeNumber());
  ASSERT_EQ(8, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, TestObjectSize){
  ZObject3dScan obj;

  std::vector<size_t> sizeArray = obj.getConnectedObjectSize();
  ASSERT_TRUE(sizeArray.empty());

  createObject(&obj);
  obj.print();
  sizeArray = obj.getConnectedObjectSize();
  ASSERT_EQ(2, (int) sizeArray.size());
  ASSERT_EQ(8, (int) sizeArray[0]);
  ASSERT_EQ(4, (int) sizeArray[1]);

  obj.clear();
  Stack *stack = C_Stack::readSc(GET_BENCHMARK_DIR +
        "/binary/2d/disk_n2.tif");
  obj.loadStack(stack);
  ASSERT_TRUE(obj.isCanonized());
  sizeArray = obj.getConnectedObjectSize();
  ASSERT_EQ(2, (int) sizeArray.size());
  ASSERT_EQ(489, (int) sizeArray[0]);
  ASSERT_EQ(384, (int) sizeArray[1]);

  C_Stack::kill(stack);

  obj.clear();
  stack = C_Stack::readSc(
        GET_BENCHMARK_DIR +
        "/binary/2d/ring_n10.tif");
  obj.loadStack(stack);

  ASSERT_TRUE(obj.isCanonized());

  sizeArray = obj.getConnectedObjectSize();

  ASSERT_EQ(10, (int) sizeArray.size());
  ASSERT_EQ(616, (int) sizeArray[0]);
  ASSERT_EQ(572, (int) sizeArray[1]);
  ASSERT_EQ(352, (int) sizeArray[2]);
  ASSERT_EQ(296, (int) sizeArray[3]);
  ASSERT_EQ(293, (int) sizeArray[4]);
  ASSERT_EQ(279, (int) sizeArray[5]);
  ASSERT_EQ(208, (int) sizeArray[6]);
  ASSERT_EQ(125, (int) sizeArray[7]);
  ASSERT_EQ(112, (int) sizeArray[8]);
  ASSERT_EQ(112, (int) sizeArray[9]);

  C_Stack::kill(stack);

  /*
  obj.clear();
  obj.load(GET_TEST_DATA_DIR +
           "/benchmark/432.sobj");
  sizeArray = obj.getConnectedObjectSize();
  ASSERT_EQ(77, (int) sizeArray.size());
  std::cout << sizeArray[0] << std::endl;

  int offset[3];
  stack = obj.toStack(offset);
  offset[0] = -offset[0];
  offset[1] = -offset[1];
  offset[2] = -offset[2];
  obj.labelStack(stack, 2, offset);
  C_Stack::write(GET_TEST_DATA_DIR +
                 "/test.tif", stack);
                 */
  //ASSERT_EQ(616, (int) sizeArray[0]);
}

TEST(ZObject3dScan, TestBuildGraph) {
  ZObject3dScan obj;
  //createObject(&obj);
  obj.addStripe(0, 0);
  obj.addSegment(0, 1, false);

  ZGraph *graph = obj.buildConnectionGraph();

  ASSERT_EQ(0, graph->getEdgeNumber());

  delete graph;

  obj.addSegment(3, 4);
  graph = obj.buildConnectionGraph();
  ASSERT_EQ(0, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(0, 1);
  obj.addSegment(0, 1, false);
  graph = obj.buildConnectionGraph();
  ASSERT_EQ(1, graph->getEdgeNumber());
  delete graph;

  obj.addSegment(2, 2, false);
  graph = obj.buildConnectionGraph();
  ASSERT_EQ(2, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(1, 0);
  obj.addSegment(2, 2);
  graph = obj.buildConnectionGraph();
  ASSERT_EQ(5, graph->getEdgeNumber());

  const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();
  ASSERT_EQ(1, (int) subGraph.size());
  delete graph;

  obj.clear();
  Stack *stack = C_Stack::readSc(
        GET_BENCHMARK_DIR +
        "/binary/2d/ring_n10.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph2 = graph->getConnectedSubgraph();
  ASSERT_EQ(10, (int) subGraph2.size());
  delete graph;
  C_Stack::kill(stack);

#if 1
  obj.clear();
  stack = C_Stack::readSc(
        GET_BENCHMARK_DIR +
        "/binary/3d/diadem_e1.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph3 = graph->getConnectedSubgraph();
  ASSERT_EQ(4, (int) subGraph3.size());
  delete graph;
#endif

  obj.clear();
  obj.addStripe(1, 2);
  obj.addSegment(2, 2);
  obj.addStripe(2, 1);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(2, 3);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(3, 0);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  obj.addStripe(3, 2);
  obj.addSegment(0, 0);
  obj.addSegment(4, 4);
  obj.addStripe(3, 4);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph5 = graph->getConnectedSubgraph();
  ASSERT_EQ(16, graph->getEdgeNumber());
  ASSERT_EQ(1, (int) subGraph5.size());
  delete graph;


  obj.clear();
  stack = C_Stack::readSc(
        GET_BENCHMARK_DIR +
        "/binary/3d/series.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph4 = graph->getConnectedSubgraph();
  ASSERT_EQ(15, (int) subGraph4.size());
  delete graph;

  obj.clear();
  stack = C_Stack::readSc(
        GET_BENCHMARK_DIR +
        "/binary/3d/block/test.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  ASSERT_EQ(1, (int) graph->getConnectedSubgraph().size());
  delete graph;
}

/*
static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5, false);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}
*/

TEST(ZObject3dScan, TestGetSegment) {
  ZObject3dScan obj;
  createObject(&obj);
  int z, y, x1, x2;
  obj.getSegment(0, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(0, y);
  ASSERT_EQ(0, x1);
  ASSERT_EQ(1, x2);

  obj.getSegment(1, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(0, y);
  ASSERT_EQ(4, x1);
  ASSERT_EQ(5, x2);

  obj.getSegment(2, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(0, y);
  ASSERT_EQ(7, x1);
  ASSERT_EQ(8, x2);

  obj.getSegment(3, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(1, y);
  ASSERT_EQ(0, x1);
  ASSERT_EQ(1, x2);

  obj.getSegment(4, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(1, y);
  ASSERT_EQ(3, x1);
  ASSERT_EQ(3, x2);

  obj.getSegment(5, &z, &y, &x1, &x2);
  ASSERT_EQ(0, z);
  ASSERT_EQ(1, y);
  ASSERT_EQ(5, x1);
  ASSERT_EQ(7, x2);
}

TEST(ZObject3dScan, TestGetConnectedComponent)
{
  {
    ZObject3dScan obj;
    obj.addSegment(0, 0, 1, 2);
    obj.addSegment(0, 1, 3, 4);
    std::vector<ZObject3dScan> objArray =
        obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    ASSERT_EQ(1, (int) objArray.size());
  }

  {
    ZObject3dScan obj;
    obj.addSegment(0, 0, 1, 2);
    obj.addSegment(1, 1, 3, 4);
    std::vector<ZObject3dScan> objArray =
        obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    ASSERT_EQ(1, (int) objArray.size());
  }

  {
    ZObject3dScan obj;
    createObject(&obj);

    std::vector<ZObject3dScan> objArray =
        obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    ASSERT_EQ(2, (int) objArray.size());
    ASSERT_EQ(4, (int) objArray[0].getVoxelNumber());
    ASSERT_EQ(8, (int) objArray[1].getVoxelNumber());

    obj.clear();
    Stack *stack = C_Stack::readSc(
          GET_BENCHMARK_DIR +
          "/binary/2d/ring_n10.tif");
    obj.loadStack(stack);

    objArray = obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    ASSERT_EQ(10, (int) objArray.size());
    ASSERT_EQ(352, (int) objArray[0].getVoxelNumber());
    ASSERT_EQ(279, (int) objArray[1].getVoxelNumber());
    ASSERT_EQ(125, (int) objArray[2].getVoxelNumber());
    ASSERT_EQ(112, (int) objArray[3].getVoxelNumber());
    ASSERT_EQ(616, (int) objArray[4].getVoxelNumber());
    ASSERT_EQ(112, (int) objArray[5].getVoxelNumber());
    ASSERT_EQ(296, (int) objArray[6].getVoxelNumber());
    ASSERT_EQ(293, (int) objArray[7].getVoxelNumber());
    ASSERT_EQ(572, (int) objArray[8].getVoxelNumber());
    ASSERT_EQ(208, (int) objArray[9].getVoxelNumber());

    for (size_t i = 0; i < objArray.size() - 1; ++i) {
      for (size_t j = i + 1; j < objArray.size(); ++j) {
        ASSERT_FALSE(objArray[i].isAdjacentTo(
                       objArray[j], neutube::EStackNeighborhood::D3));
      }
    }

    C_Stack::kill(stack);

    obj.clear();
    stack = C_Stack::readSc(
          GET_BENCHMARK_DIR +
          "/binary/3d/diadem_e1.tif");
    obj.loadStack(stack);
    objArray = obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    ASSERT_EQ(43, (int) objArray.size());
    ASSERT_EQ(2, (int) objArray[0].getVoxelNumber());
    ASSERT_EQ(68236, (int) objArray[1].getVoxelNumber());
    ASSERT_EQ(2, (int) objArray[2].getVoxelNumber());
    ASSERT_EQ(3, (int) objArray[3].getVoxelNumber());

    for (size_t i = 0; i < objArray.size() - 1; ++i) {
      for (size_t j = i + 1; j < objArray.size(); ++j) {
        /*
        if (!objArray[i].isAdjacentTo(
              objArray[j], neutube::EStackNeighborhood::D3)) {
          objArray[i].print();
          objArray[j].print();
        }
        */
        ASSERT_FALSE(objArray[i].isAdjacentTo(
                       objArray[j], neutube::EStackNeighborhood::D1));
        ASSERT_FALSE(objArray[i].isAdjacentTo(
                       objArray[j], neutube::EStackNeighborhood::D2));
        ASSERT_FALSE(objArray[i].isAdjacentTo(
                       objArray[j], neutube::EStackNeighborhood::D3));
      }
    }
  }

  /*
  for (size_t i = 0; i < objArray.size(); ++i) {
    std::cout<< objArray[i].getVoxelNumber() << std::endl;
  }
*/
}

TEST(ZObject3dScan, duplicateAcrossZ)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 2);
  obj.duplicateSlice(3);

  ASSERT_EQ(3, (int) obj.getStripeNumber());
  ASSERT_EQ(6, (int) obj.getVoxelNumber());

  obj.duplicateSlice(2);
  obj.print();
  ASSERT_EQ(2, (int) obj.getStripeNumber());
  ASSERT_EQ(4, (int) obj.getVoxelNumber());

  obj.addSegment(0, 1, 3, 4);
  obj.duplicateSlice(3);

  ASSERT_EQ(6, (int) obj.getStripeNumber());
  ASSERT_EQ(12, (int) obj.getVoxelNumber());

  //obj.print();
}

TEST(ZObject3dScan, TestScanArray) {
  Stack *stack = C_Stack::readSc(GET_BENCHMARK_DIR +
                                 "/binary/3d/diadem_e1.tif");

  std::map<uint64_t, ZObject3dScan*> *objSet = ZObject3dScan::extractAllObject(
        stack->array, C_Stack::width(stack), C_Stack::height(stack),
        C_Stack::depth(stack), 0, 1, NULL);

  ASSERT_EQ(2, (int) objSet->size());
  ASSERT_TRUE((*objSet)[0]->isCanonizedActually());
  ASSERT_TRUE((*objSet)[1]->isCanonizedActually());

  (*objSet)[1]->save(GET_TEST_DATA_DIR + "/test.sobj");
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  ASSERT_TRUE(obj.isCanonizedActually());


  ZStack stack2;
  stack2.load(GET_BENCHMARK_DIR + "/block.tif");
  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(stack2);
  ASSERT_EQ(99, (int) objArray.size());
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZObject3dScan *obj = objArray[i];
    ASSERT_EQ(1, (int) obj->getVoxelNumber());
  }

  stack2.setOffset(ZIntPoint(30, 40, 50));
  objArray = ZObject3dScan::extractAllObject(stack2);
  ASSERT_EQ(99, (int) objArray.size());
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZObject3dScan *obj = objArray[i];
//    obj->print();
    ASSERT_EQ(1, (int) obj->getVoxelNumber());
  }

  {
    uint64_t array[10] = {0, 0, 1, 1, 0, 1, 1, 0, 0, 1};
    ZObject3dScan obj;
    obj.scanArrayV(array, 10, 20, 30, 10, uint64_t(1));
    ASSERT_EQ(5, (int) obj.getVoxelNumber());
    ZObject3dScan obj2;
    obj2.addSegment(30, 20, 12, 13);
    obj2.addSegment(30, 20, 15, 16);
    obj2.addSegment(30, 20, 19, 19);

    obj.equalsLiterally(obj2);
  }

  {
    uint64_t array[10] = {1, 0, 1, 1, 0, 1, 1, 0, 0, 1};
    ZObject3dScan obj;
    obj.scanArrayV(array, 10, 20, 30, 10, uint64_t(1));

    ZObject3dScan obj2;
    obj2.addSegment(30, 20, 10, 10);
    obj2.addSegment(30, 20, 12, 13);
    obj2.addSegment(30, 20, 15, 16);
    obj2.addSegment(30, 20, 19, 19);

//    obj.print();

    obj.equalsLiterally(obj2);
  }

  //obj.scanArray(array, )
}

TEST(ZObject3dScan, TestIO) {
  ZObject3dScan obj;
  obj.load(GET_BENCHMARK_DIR + "/29.sobj");

  ASSERT_TRUE(obj.isCanonizedActually());
}

TEST(ZObject3dScan, dilate) {
  ZObject3dScan obj;

  obj.addSegment(0, 0, 0, 1);

  obj.dilate();
  ASSERT_EQ(5, (int) obj.getStripeNumber());
  ASSERT_EQ(12, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 3, 4);

  obj.dilate();

  ASSERT_EQ(5, (int) obj.getStripeNumber());
  ASSERT_EQ(23, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 1);
  obj.dilate();
  //obj.print();
  ASSERT_EQ(8, (int) obj.getStripeNumber());
  ASSERT_EQ(20, (int) obj.getVoxelNumber());

  obj.clear();;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.dilate();
  ASSERT_EQ(5, (int) obj.getStripeNumber());
  ASSERT_EQ(24, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, -1, 2, 3);
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.addSegment(0, 1, 2, 3);
  //obj.print();
  obj.dilate();
  //obj.print();
  ASSERT_EQ(11, (int) obj.getStripeNumber());
  ASSERT_EQ(13, (int) obj.getSegmentNumber());
  ASSERT_EQ(40, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(1, 0, 0, 1);
  obj.dilate();
  ASSERT_EQ(8, (int) obj.getStripeNumber());
  ASSERT_EQ(20, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, overlap)
{
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  Zero_Stack(stack);
  stack->array[0] = 1;

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);

  ASSERT_EQ(1, (int) obj.countForegroundOverlap(stack));
}

class ZObject3dScanTestF1 : public ::testing::Test {
protected:
  virtual void SetUp() {
    m_obj.clear();
    m_obj.addSegment(0, 1, 0, 2);
    m_obj.addSegment(1, 0, 1, 1);
    m_obj.addSegment(1, 1, 0, 2);
    m_obj.addSegment(1, 2, 1, 1);
    m_obj.addSegment(2, 0, 1, 1);
    m_obj.addSegment(2, 1, 1, 1);
    m_obj.addSegment(2, 2, 1, 1);
  }

  virtual void TearDown() {}

  ZObject3dScan m_obj;
};

TEST_F(ZObject3dScanTestF1, Slice)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZObject3dScan slice = obj.getSlice(0);
  ASSERT_EQ(3, (int) slice.getVoxelNumber());

  slice = obj.getSlice(1);
  ASSERT_EQ(5, (int) slice.getVoxelNumber());

  slice = obj.getSlice(2);
  ASSERT_EQ(3, (int) slice.getVoxelNumber());

  slice = obj.getSlice(0, 1);
//  slice.print();
  ASSERT_EQ(8, (int) slice.getVoxelNumber());

  slice = obj.getSlice(0, 2);
  ASSERT_EQ(11, (int) slice.getVoxelNumber());
}

TEST_F(ZObject3dScanTestF1, Statistics)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZPoint center = obj.getCentroid();
  EXPECT_DOUBLE_EQ(1.0, center.x());
  EXPECT_DOUBLE_EQ(1.0, center.y());
  EXPECT_DOUBLE_EQ(1.0, center.z());
}

TEST_F(ZObject3dScanTestF1, equal)
{
  ZObject3dScan obj = m_obj;
  ASSERT_TRUE(m_obj.equalsLiterally(obj));
  obj.canonize();
  ASSERT_TRUE(m_obj.equalsLiterally(obj));

  obj.addSegment(2, 1, 1, 1, false);
  EXPECT_FALSE(m_obj.equalsLiterally(obj));

  obj.canonize();
  //obj.print();
  //m_obj.print();
  ASSERT_TRUE(m_obj.equalsLiterally(obj));
}

TEST_F(ZObject3dScanTestF1, complement)
{
  ZObject3dScan obj = m_obj;
  ZObject3dScan compObj = obj.getComplementObject();

  int offset[3];
  Stack *stack = obj.toStack(offset);
  iarray_neg(offset, 3);
  ASSERT_EQ(0, (int) compObj.countForegroundOverlap(stack, offset));

  ASSERT_EQ(obj.getVoxelNumber() + compObj.getVoxelNumber(),
            C_Stack::voxelNumber(stack));

  C_Stack::kill(stack);
}

TEST(ZObject3dScanTest, findHole)
{
  ZObject3dScan obj;
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  One_Stack(stack);
  C_Stack::setPixel(stack, 1, 1, 1, 0, 0);

  obj.loadStack(stack);
  //obj.print();

  ZObject3dScan hole = obj.findHoleObject();

  ASSERT_EQ(1, (int) hole.getVoxelNumber());

  C_Stack::kill(stack);

  stack = C_Stack::make(GREY, 5, 5, 5);
  One_Stack(stack);
  C_Stack::setPixel(stack, 0, 0, 0, 0, 0);
  C_Stack::setPixel(stack, 2, 2, 2, 0, 0);
  obj.loadStack(stack);

  //obj.print();

  hole = obj.findHoleObject();

  ASSERT_EQ(1, (int) hole.getVoxelNumber());

  C_Stack::setPixel(stack, 2, 2, 3, 0, 0);
  obj.loadStack(stack);
  hole = obj.findHoleObject();
  ASSERT_EQ(2, (int) hole.getVoxelNumber());

  C_Stack::kill(stack);
}

TEST(ZObject3dScan, Cov)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 1);
  std::vector<double> cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.0, cov[0]);
  ASSERT_DOUBLE_EQ(0.0, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);

  obj.addSegment(0, 0, 1, 2);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.5, cov[0]);
  ASSERT_DOUBLE_EQ(0.0, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);

  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(0, 1, 1, 3);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.8, cov[0]);
  ASSERT_DOUBLE_EQ(0.3, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);


  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(0, 1, 1, 3);
  obj.addSegment(0, 2, 3, 6);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(2.666666666666667, cov[0]);
  ASSERT_DOUBLE_EQ(0.76666666666666639, cov[1]);
  ASSERT_DOUBLE_EQ(1.0, cov[2]);

  obj.clear();
  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(1, 1, 1, 3);
  obj.addSegment(2, 2, 3, 6);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(2.666666666666667, cov[0]);
  ASSERT_DOUBLE_EQ(0.76666666666666639, cov[1]);
  ASSERT_DOUBLE_EQ(1.0, cov[2]);

  obj.clear();
  obj.addSegment(0, 0, 1, 5);
  obj.addSegment(0, 1, 1, 5);
  obj.addSegment(0, 2, 1, 5);
  std::cout << obj.getSpread(0) << std::endl;
  //ZDebugPrintDoubleArray(cov, 0, 2);
}

TEST(ZObject3dScan, contains)
{
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(0, 1);

  ASSERT_TRUE(stripe.containsX(0));
  ASSERT_FALSE(stripe.containsX(2));

  stripe.addSegment(3, 4);
  stripe.addSegment(6, 9);
  ASSERT_TRUE(stripe.containsX(0));
  ASSERT_FALSE(stripe.containsX(2));
  ASSERT_TRUE(stripe.containsX(7));
  ASSERT_FALSE(stripe.containsX(10));

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  ASSERT_TRUE(obj.contains(0, 0, 0));
  ASSERT_FALSE(obj.contains(2, 0, 0));

  obj.addSegment(0, 0, 3, 4);
  obj.addSegment(0, 0, 6, 9);
  ASSERT_TRUE(obj.contains(7, 0, 0));
  ASSERT_FALSE(obj.contains(10, 0, 0));

  obj.addSegment(0, 1, 0, 1);
  ASSERT_TRUE(obj.contains(0, 1, 0));
  ASSERT_FALSE(obj.contains(0, 0, 1));

  obj.addSegment(3, 4, 0, 10);
  obj.addSegment(6, 8, 1, 10);
  ASSERT_TRUE(obj.contains(0, 4, 3));
  ASSERT_FALSE(obj.contains(0, 8, 6));

  obj.clear();
  obj.load(GET_BENCHMARK_DIR + "/tower3.sobj");
  ASSERT_TRUE(obj.contains(1, 1, 0));
  ASSERT_FALSE(obj.contains(0, 0, 0));
  ASSERT_FALSE(obj.contains(0, 0, 2));
  ASSERT_TRUE(obj.contains(1, 0, 2));
  ASSERT_TRUE(obj.contains(1, 1, 1));
  ASSERT_FALSE(obj.contains(2, 0, 1));
  ASSERT_TRUE(obj.contains(2, 1, 2));
  ASSERT_TRUE(obj.contains(1, 2, 2));
  ASSERT_FALSE(obj.contains(2, 2, 2));

//  obj.addSegment(0, 0, 3, 4);
//  obj.addSegment(0, 0, 6, 9);
}

TEST(ZObject3dScan, component)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 1);

  auto &vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(1, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(0, 0, 1, 2);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(1, 1, 2, 4);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(5, 1, 0, 4);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(5, (int) vs[5]);
  ASSERT_EQ(0, (int) vs.count(2));

  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(5, (int) vs[5]);
  ASSERT_EQ(0, (int) vs.count(2));
}

TEST(ZObject3dScan, load)
{
  std::vector<int> array;
  array.push_back(1);
  array.push_back(0);
  array.push_back(0);
  array.push_back(1);
  array.push_back(0);
  array.push_back(1);

  ZObject3dScan obj;
  ASSERT_TRUE(obj.load(&(array[0]), array.size()));
  //obj.print();
  ASSERT_EQ(2, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, Relation)
{
  ZObject3dScan obj1;
  obj1.addStripe(0, 0);
  obj1.addSegment(0, 2);
  obj1.print();

  ZObject3dScan obj2;
  obj2.addStripe(1, 0);
  obj2.addSegment(0, 1);

  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj2.addSegment(0, 0, 0, 1);
  ASSERT_TRUE(obj1.hasOverlap(obj2));

  obj2.clear();
  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj1.clear();
  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj1.addStripe(10, 20);
  obj1.addSegment(30, 40);

  obj2.addStripe(10, 20);
  obj2.addSegment(30, 30);
  ASSERT_TRUE(obj1.hasOverlap(obj2));

  obj2.clear();
  obj2.addStripe(10, 20);
  obj2.addSegment(29, 29);
  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj1.clear();
  obj2.clear();
  obj1.addSegment(0, 0, 0, 1);
  obj2.addSegment(0, 0, 1, 2);
  ASSERT_EQ(1, (int) misc::CountOverlap(obj1, obj2));

  obj2.clear();
  obj2.addSegment(0, 0, 2, 3);
  ASSERT_EQ(0, (int) misc::CountOverlap(obj1, obj2));


  ASSERT_EQ(1, (int) misc::CountNeighbor(obj1, obj2));
  ASSERT_EQ(1, (int) misc::CountNeighborOnPlane(obj1, obj2));

  obj2.addSegment(1, 0, 1, 2);
  ASSERT_EQ(2, (int) misc::CountNeighbor(obj1, obj2));
  ASSERT_EQ(1, (int) misc::CountNeighborOnPlane(obj1, obj2));

  obj1.addSegment(1, 0, 2, 3);
  ASSERT_EQ(3, (int) misc::CountNeighborOnPlane(obj1, obj2));

  obj1.clear();
  obj2.clear();
  obj1.addSegment(0, 0, 0, 1);
  obj2.addSegment(0, 0, 2, 3);
  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));

  obj2.clear();
  obj2.addSegment(0, 1, 1, 2);
  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));

  obj2.clear();
  obj2.addSegment(0, 0, 3, 4);
  obj2.addSegment(0, 1, 1, 2);
  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));


  obj2.clear();
  obj2.addSegment(0, -1, 1, 2);
  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));

  obj1.clear();
  obj2.clear();
  obj1.addSegment(0, 1, 0, 1);
  obj1.addSegment(0, 3, 0, 1);
  obj1.addSegment(0, 5, 0, 1);
  obj1.addSegment(0, 6, 0, 1);
  obj1.addSegment(0, 9, 0, 1);
  obj1.addSegment(0, 10, 0, 1);
  obj1.addSegment(0, 11, 0, 1);

  obj2.addSegment(0, 2, 3, 4);
  obj2.addSegment(0, 3, 3, 4);
  obj2.addSegment(0, 7, 3, 4);
  obj2.addSegment(0, 8, 3, 4);
  obj2.addSegment(0, 9, 3, 4);
  obj2.addSegment(0, 10, 3, 4);
  obj2.addSegment(0, 12, 1, 4);
  obj2.addSegment(0, 13, 3, 4);
  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));

  obj1.clear();
  obj2.clear();
  obj1.addSegment(1, 0, 0, 1);
  obj1.addSegment(3, 0, 0, 1);
  obj1.addSegment(5, 0, 0, 1);
  obj1.addSegment(6, 0, 0, 1);
  obj1.addSegment(9, 0, 0, 1);
  obj1.addSegment(10, 0, 0, 1);
  obj1.addSegment(11, 0, 0, 1);

  obj2.addSegment(2, 0, 3, 4);
  obj2.addSegment(3, 0, 3, 4);
  obj2.addSegment(7, 0, 2, 4);
  obj2.addSegment(8, 0, 3, 4);
  obj2.addSegment(9, 0, 3, 4);
  obj2.addSegment(10, 0, 3, 4);
  obj2.addSegment(12, 0, 3, 4);
  obj2.addSegment(13, 0, 1, 4);

  ASSERT_FALSE(obj1.isAdjacentTo(obj2));
  ASSERT_FALSE(obj2.isAdjacentTo(obj1));
  ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1, neutube::EStackNeighborhood::D2));
  ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D3));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1, neutube::EStackNeighborhood::D3));

  obj1.clear();
  obj2.clear();
  obj1.addSegment(1, 0, 0, 1);
  obj1.addSegment(3, 0, 0, 1);
  obj1.addSegment(5, 0, 0, 1);
  obj1.addSegment(6, 0, 0, 1);
  obj1.addSegment(9, 0, 0, 1);
  obj1.addSegment(10, 0, 0, 1);
  obj1.addSegment(11, 0, 0, 1);

  obj2.addSegment(2, 0, 3, 4);
  obj2.addSegment(3, 0, 3, 4);
  obj2.addSegment(7, 0, 3, 4);
  obj2.addSegment(8, 0, 3, 4);
  obj2.addSegment(9, 0, 3, 4);
  obj2.addSegment(10, 0, 3, 4);
  obj2.addSegment(12, 0, 1, 4);
  obj2.addSegment(13, 0, 3, 4);

  ASSERT_TRUE(obj1.isAdjacentTo(obj2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1));
  ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D2));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1, neutube::EStackNeighborhood::D2));
  ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D3));
  ASSERT_TRUE(obj2.isAdjacentTo(obj1, neutube::EStackNeighborhood::D3));
}

TEST(ZObject3dScan, upSample)
{
  ZObject3dScan obj;
  createObject(&obj);
  //obj.print();

  std::cout << "Upsampling" << std::endl;
  obj.upSample(1, 0, 0);
  ASSERT_TRUE(obj.contains(0, 0, 0));
  ASSERT_TRUE(obj.contains(15, 0, 0));
  ASSERT_FALSE(obj.contains(15, 0, 1));
  ASSERT_TRUE(obj.contains(15, 1, 0));

  //obj.print();

//  std::cout << "Upsampling" << std::endl;
//  obj.upSample(1, 1, 1);
//  obj.print();

//  obj.downsampleMax(1, 0, 0);
//  obj.print();


//  ZObject3dScan obj1;
//  obj1.addStripe(1, 1);
//  obj1.addSegment(0, 2);

//  std::cout << "Upsampling" << std::endl;
//  obj1.upSample(1, 1, 1);
//  obj1.print();

}

TEST(ZObject3dScan, Stack)
{
  ZObject3dScan obj;
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);
  ZStack *stack = obj.toStackObject(1);
//  stack->printInfo();

  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(*stack);
  ASSERT_EQ(1, (int) objArray.size());
  //objArray[0]->print();

  obj.addStripe(1, 0);
  obj.addSegment(0, 0);
  obj.addStripe(1, 2);
  obj.addSegment(2, 2);

  obj.canonize();

//  obj.print();

  delete stack;
  stack = obj.toStackObject(2);
  Print_Stack_Value(stack->c_stack());
  objArray = ZObject3dScan::extractAllObject(*stack);
  ASSERT_EQ(1, (int) objArray.size());
//  objArray[0]->print();

  ZObject3dScan obj2;
  obj2.addStripe(1, 1);
  obj2.addSegment(1, 1);

  ZObject3dScan obj3 = obj.subtract(obj2);
//  obj.print();
//  obj2.print();
//  obj3.print();
  ASSERT_EQ(1, (int) obj3.getVoxelNumber());

  obj.clear();
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);
  obj.addSegment(1, 3);
  obj.addStripe(0, 0);
  obj.addSegment(1, 3);

//  obj.print();

  delete stack;
  stack = ZStackFactory::makeIndexStack(3, 3, 3);

  obj.maskStack(stack);

//  Print_Stack_Value(stack->c_stack());
  ASSERT_EQ(0, stack->getIntValue(0));
  ASSERT_EQ(1, stack->getIntValue(1));
  ASSERT_EQ(2, stack->getIntValue(2));
  ASSERT_EQ(0, stack->getIntValue(3));
  ASSERT_EQ(13, stack->getIntValue(13));
  ASSERT_EQ(14, stack->getIntValue(14));
  ASSERT_EQ(0, stack->getIntValue(18));

  delete stack;

  ZStack *stack2 = obj.toStackObjectWithMargin(1, 1);

  ZObject3dScan obj4;
  obj4.loadStack(*stack2);

  delete stack2;

  ASSERT_TRUE(obj.equalsLiterally(obj4));

  obj.clear();
  obj.load(GET_BENCHMARK_DIR + "/tower5.sobj");
  stack2 = obj.toStackObjectWithMargin(1, 1);
  obj4.loadStack(*stack2);

  delete stack2;

  ASSERT_TRUE(obj.equalsLiterally(obj4));
}

TEST(ZObject3dScan, Intersect)
{
  ZObject3dScan obj;
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);

  ZObject3dScan obj2;
  obj2.addStripe(1, 1);
  obj2.addSegment(1, 1);

  ZObject3dScan obj3 = obj.intersect(obj2);

  ASSERT_TRUE(obj3.equalsLiterally(obj));

  obj2.addSegment(1, 2, 0, 5);
  obj2.addSegment(1, 1, 0, 1);
  obj3 = obj.intersect(obj2);

  ASSERT_TRUE(obj3.equalsLiterally(obj));

  obj.addSegment(2, 1, 0, 5);
  obj3 = obj.intersect(obj2);

  ASSERT_EQ(1, (int) obj3.getVoxelNumber());
  ASSERT_TRUE(obj3.contains(1, 1, 1));
}

TEST(ZObject3dScan, Iterator)
{
  ZObject3dScan::ConstSegmentIterator segIter(NULL);
  ASSERT_FALSE(segIter.hasNext());

  ZObject3dScan::ConstVoxelIterator voxelIter(NULL);
  ASSERT_FALSE(voxelIter.hasNext());

  {
    ZObject3dScan obj1;
    obj1.addSegment(0, 0, 0, 2);
    obj1.addSegment(1, 1, 1, 3);
    ZObject3dScan::ConstSegmentIterator segIter1(&obj1);
    ASSERT_TRUE(segIter1.hasNext());
    const ZObject3dScan::Segment &seg1 = segIter1.next();
    ASSERT_TRUE(segIter1.hasNext());
    ASSERT_EQ(0, seg1.getY());
    ASSERT_EQ(2, seg1.getEnd());
  }

  {
    ZObject3dScan obj1;
    obj1.addStripe(0, 1);
    obj1.addSegment(0, 0, 0, 2);
    obj1.addStripe(1, 0);
    obj1.addStripe(4, 5);
    obj1.addSegment(1, 1, 1, 3);
    ZObject3dScan::ConstSegmentIterator segIter1(&obj1);
    ASSERT_TRUE(segIter1.hasNext());
    const ZObject3dScan::Segment &seg1 = segIter1.next();
    ASSERT_TRUE(segIter1.hasNext());
    ASSERT_EQ(0, seg1.getY());
    ASSERT_EQ(2, seg1.getEnd());
    const ZObject3dScan::Segment &seg2 = segIter1.next();
    ASSERT_EQ(1, seg2.getY());
    ASSERT_EQ(3, seg2.getEnd());
    ASSERT_FALSE(segIter1.hasNext());
  }

  {
    ZObject3dScan obj1;
    obj1.addStripe(0, 1);
    obj1.addStripe(0, 2);
    obj1.addSegment(0, 0, 0, 2);
    obj1.addStripe(1, 0);
    obj1.addStripe(4, 9);

    ZObject3dScan::ConstSegmentIterator segIter1(&obj1);
    ASSERT_TRUE(segIter1.hasNext());
    const ZObject3dScan::Segment &seg1 = segIter1.next();
    ASSERT_EQ(0, seg1.getY());
    ASSERT_FALSE(segIter1.hasNext());

    const ZObject3dScan::Segment &seg2 = segIter1.next();
    ASSERT_TRUE(seg2.isEmpty());
  }

  {
    ZObject3dScan obj1;
    obj1.addSegment(0, 0, 0, 2);
    obj1.addSegment(1, 1, 1, 3);
    ZObject3dScan::ConstVoxelIterator voxelIter1(&obj1);
    ASSERT_TRUE(voxelIter1.hasNext());
    ZIntPoint pt = voxelIter1.next();
    ASSERT_TRUE(voxelIter1.hasNext());
    ASSERT_EQ(ZIntPoint(0, 0, 0), pt);
    ASSERT_EQ(ZIntPoint(1, 0, 0), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(2, 0, 0), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(1, 1, 1), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(2, 1, 1), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(3, 1, 1), voxelIter1.next());
    ASSERT_FALSE(voxelIter1.hasNext());
  }

  {
    ZObject3dScan obj1;
    obj1.addStripe(0, 1);
    obj1.addSegment(0, 0, 0, 2);
    obj1.addStripe(1, 0);
    obj1.addStripe(4, 5);
    obj1.addSegment(1, 1, 1, 3);
    obj1.addStripe(5, 6);
    ZObject3dScan::ConstVoxelIterator voxelIter1(&obj1);

    ASSERT_TRUE(voxelIter1.hasNext());
    ZIntPoint pt = voxelIter1.next();
    ASSERT_EQ(ZIntPoint(0, 0, 0), pt);
    ASSERT_EQ(ZIntPoint(1, 0, 0), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(2, 0, 0), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(1, 1, 1), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(2, 1, 1), voxelIter1.next());
    ASSERT_EQ(ZIntPoint(3, 1, 1), voxelIter1.next());
    ASSERT_FALSE(voxelIter1.hasNext());
  }

  {
    ZObject3dScan obj1;
    obj1.load(GET_BENCHMARK_DIR + "/29.sobj");
    int v = obj1.getVoxelNumber();

    int v2 = 0;
    ZObject3dScan::ConstVoxelIterator voxelIter1(&obj1);
    while (voxelIter1.hasNext()) {
      ++v2;
      voxelIter1.next();
    }

    ASSERT_EQ(v, v2);
  }

}

TEST(ZObject3dScan, Unify)
{
  ZObject3dScan obj1;
  obj1.addSegment(0, 0, 0, 2);
  obj1.addSegment(1, 1, 1, 3);

  ZObject3dScan obj2;
  obj2.addSegment(2, 0, 0, 2);
  obj2.addSegment(3, 1, 1, 3);

  obj1.unify(obj2);

  ZObject3dScan obj3;
  obj3.addSegment(2, 0, 0, 2);
  obj3.addSegment(3, 1, 1, 3);

  ZObject3dScan obj4;
  obj4.addSegment(0, 0, 0, 2);
  obj4.addSegment(1, 1, 1, 3);

  obj3.unify(obj4);

  obj1.equalsLiterally(obj3);
}

TEST(ZObject3dScan, subtract)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 5);

  ZObject3dScan obj2;
  obj2.addSegment(0, 0, 2, 3);

  ZObject3dScan obj1 = obj;

  ZObject3dScan subtracted = obj1.subtract(obj2);
  ASSERT_EQ(4, (int) obj1.getVoxelNumber());
  ASSERT_EQ(2, (int) obj2.getVoxelNumber());

  ASSERT_TRUE(obj1.intersect(subtracted).isEmpty());
  obj1.unify(subtracted);
  ASSERT_TRUE(obj.equalsLiterally(obj1));

  obj.addSegment(0, 1, 4, 7);
  obj1 = obj;
  subtracted = obj1.subtract(obj2);
  ASSERT_TRUE(obj1.intersect(subtracted).isEmpty());
  obj1.unify(subtracted);
  ASSERT_TRUE(obj.equalsLiterally(obj1));

  obj1 = obj;
  obj2.addSegment(0, 1, 2, 3);
  obj2.addSegment(0, 1, 5, 7);
  obj2.addSegment(0, 2, 2, 3);
  subtracted = obj1.subtract(obj2);
  ASSERT_TRUE(obj1.intersect(subtracted).isEmpty());
  obj1.unify(subtracted);
  ASSERT_TRUE(obj.equalsLiterally(obj1));

//  obj.print();
//  subtracted.print();
}

TEST(ZObject3dScan, Mainpulate)
{
  ZObject3dScan obj;
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);

  ZIntCuboid box;
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(1, 1, 1);

  ZObject3dScan subobj;
  ZObject3dScan remain;

  obj.subobject(box, &remain, &subobj);

  ASSERT_EQ(1, (int) subobj.getVoxelNumber());
  ASSERT_TRUE(remain.isEmpty());

  obj.clear();
  obj.addSegment(0, 0, 0, 2);
  obj.addSegment(0, 1, 1, 3);
  obj.addSegment(0, 2, 1, 4);
  obj.addSegment(0, 3, 0, 3);
  obj.addSegment(0, 4, 2, 4);

  box.setFirstCorner(1, 1, 0);
  box.setLastCorner(3, 3, 0);

  obj.subobject(box, &remain, &subobj);
//  subobj.print();
  ASSERT_EQ(9, (int) subobj.getVoxelNumber());
  ASSERT_EQ(8, (int) remain.getVoxelNumber());

  ASSERT_TRUE(subobj.intersect(remain).isEmpty());
  subobj.unify(remain);
  ASSERT_TRUE(obj.equalsLiterally(subobj));

  obj.load(GET_BENCHMARK_DIR + "/29.sobj");
  box.setFirstCorner(210, 759, 348);
  box.setLastCorner(694, 1001, 480);
  obj.subobject(box, &remain, &subobj);

//  remain.save(GET_TEST_DATA_DIR + "/test.sobj");
  ASSERT_TRUE(subobj.intersect(remain).isEmpty());
  subobj.unify(remain);
  ASSERT_TRUE(obj.equalsLiterally(subobj));

  obj.chopZ(500, &remain, &subobj);
  ASSERT_EQ(499, subobj.getMaxZ());
  ASSERT_EQ(500, remain.getMinZ());

  ASSERT_TRUE(subobj.intersect(remain).isEmpty());
  subobj.unify(remain);
  ASSERT_TRUE(obj.equalsLiterally(subobj));

  obj.chopX(500, &remain, &subobj);
  ASSERT_TRUE(subobj.intersect(remain).isEmpty());
  subobj.unify(remain);
  ASSERT_TRUE(obj.equalsLiterally(subobj));

  obj.chopY(750, &remain, &subobj);
  ASSERT_EQ(749, subobj.getMaxY());
  ASSERT_EQ(750, remain.getMinY());
  ASSERT_TRUE(subobj.intersect(remain).isEmpty());
  subobj.unify(remain);
  ASSERT_TRUE(obj.equalsLiterally(subobj));

  obj.clear();
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);

  obj.chopZ(0, &remain, &subobj);
  ASSERT_TRUE(subobj.isEmpty());
  ASSERT_TRUE(obj.equalsLiterally(remain));

  obj.chopZ(1, &remain, &subobj);
  ASSERT_TRUE(subobj.isEmpty());
  ASSERT_TRUE(obj.equalsLiterally(remain));

  obj.chopZ(2, &remain, &subobj);
  ASSERT_TRUE(remain.isEmpty());
  ASSERT_TRUE(obj.equalsLiterally(subobj));
}

TEST(ZObject3dScan, remove)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  ZIntCuboid box(ZIntPoint(0, 0, 0), ZIntPoint(0, 0, 0));
  obj.remove(box);

//  obj.print();
  ASSERT_EQ(1, int(obj.getVoxelNumber()));

  obj.addSegment(0, 1, 0, 2);
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(1, 0, 0);
  obj.remove(box);
//  obj.print();
  ASSERT_EQ(1, int(obj.getStripeNumber()));
  ASSERT_EQ(3, int(obj.getVoxelNumber()));

  box.setLastCorner(2, 1, 2);
  obj.remove(box);
  obj.print();
  ASSERT_TRUE(obj.isEmpty());

  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(0, 2);
  stripe.addSegment(4, 6);
  stripe.addSegment(8, 10);
  stripe.remove(5, 8);
  ZObject3dStripe stripe2;
  stripe2.setY(0);
  stripe2.setZ(0);
  stripe2.addSegment(0, 2);
  stripe2.addSegment(4, 4);
  stripe2.addSegment(9, 10);
  ASSERT_TRUE(stripe.equalsLiterally(stripe2));

}

#endif

#endif // ZOBJECT3DSCANTEST_H
