#ifndef ZHISTOGRAMTEST_H
#define ZHISTOGRAMTEST_H

#include "ztestheader.h"

#include "zhistogram.h"
#include "zinthistogram.h"
#include "neurolabi/numericarray.h"

#ifdef _USE_GTEST_

TEST(ZHistogram, Basic)
{
  ZHistogram hist;
  hist.setStart(0.0);
  hist.setInterval(1.0);

  hist.increment(0.5);
  EXPECT_EQ(1.0, hist.getDensity(0.0));
  EXPECT_EQ(0.0, hist.getDensity(1.0));

  hist.clear();
  hist.setInterval(2.0);
  hist.increment(0.5);
  hist.increment(1.5);
  hist.increment(1.0);
  hist.increment(2.0);
  hist.increment(3.0);
  hist.increment(-1.0);
  hist.increment(-0.99);
  hist.increment(-2.0);

  EXPECT_EQ(3.0, hist.getCount(-1.0));
  EXPECT_EQ(3.0, hist.getCount(1.0));
  EXPECT_EQ(2.0, hist.getCount(2.0));
}

TEST(ZHistogram, getDensity)
{
  ZHistogram hist;
  hist.setStart(0.0);
  hist.setInterval(1.0);
  hist.addCount(0.5, 1.0);
  hist.addCount(2.5, 2.0);

  EXPECT_DOUBLE_EQ(3.0, hist.getCount(0, 3.0));
  EXPECT_DOUBLE_EQ(3.0, hist.getCount(-1.0, 5.0));
  EXPECT_DOUBLE_EQ(0.5, hist.getCount(-1, 0.5));
  EXPECT_DOUBLE_EQ(0.5, hist.getCount(0, 0.5));
}

TEST(ZHistogram, Rebin)
{
  ZHistogram hist;
  hist.setStart(0.0);
  hist.setInterval(1.0);

  EXPECT_DOUBLE_EQ(0.5, hist.getBinCenter(0));
  EXPECT_DOUBLE_EQ(1.5, hist.getBinCenter(1));
  EXPECT_DOUBLE_EQ(-0.5, hist.getBinCenter(-1));

  hist.rebin(2.0);
  EXPECT_DOUBLE_EQ(1.0, hist.getBinCenter(0));
  EXPECT_DOUBLE_EQ(3.0, hist.getBinCenter(1));
  EXPECT_DOUBLE_EQ(-1.0, hist.getBinCenter(-1));

  hist.addCount(0.5, 1.0);
  hist.addCount(2.5, 2.0);
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(-0.1));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.0));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.5));
  EXPECT_DOUBLE_EQ(1.0, hist.getDensity(3.0));
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(6.0));


  hist.rebin(1.0);
  hist.print();

  EXPECT_DOUBLE_EQ(0.5, hist.getBinCenter(0));
  EXPECT_DOUBLE_EQ(1.5, hist.getBinCenter(1));
  EXPECT_DOUBLE_EQ(-0.5, hist.getBinCenter(-1));
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(-0.1));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.0));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.5));
  EXPECT_DOUBLE_EQ(1.0, hist.getDensity(3.0));
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(6.0));

  hist.rebin(2.0);
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(-0.1));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.0));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.5));
  EXPECT_DOUBLE_EQ(1.0, hist.getDensity(3.0));

  hist.rebin(0.0);
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(-0.1));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.0));
  EXPECT_DOUBLE_EQ(0.5, hist.getDensity(0.5));
  EXPECT_DOUBLE_EQ(1.0, hist.getDensity(3.0));

  hist.rebin(3.0);
  EXPECT_DOUBLE_EQ(0.0, hist.getDensity(-0.1));
  EXPECT_DOUBLE_EQ(2.0, hist.getCount(0.0));
  EXPECT_DOUBLE_EQ(2.0, hist.getCount(0.5));
  EXPECT_DOUBLE_EQ(1.0, hist.getCount(3.0));
  EXPECT_DOUBLE_EQ(0.0, hist.getCount(6.0));
}

TEST(ZHistogram, Operation)
{
  ZHistogram hist;
  hist.setStart(0.0);
  hist.setInterval(1.0);

  hist.addCount(0.5, 1.0);
  hist.addCount(1.5, 2.0);

  ZHistogram hist2;
  hist2.setStart(0.0);
  hist2.setInterval(1.0);

  hist2.addCount(0.5, 3.0);
  hist2.addCount(1.5, 4.0);

  hist += hist2;

  EXPECT_DOUBLE_EQ(4.0, hist.getCount(0.5));
  EXPECT_DOUBLE_EQ(6.0, hist.getCount(1.5));
  EXPECT_DOUBLE_EQ(0.0, hist.getCount(2.5));

  hist2.addCount(2.5, 5.0);
  hist += hist2;
  EXPECT_DOUBLE_EQ(7.0, hist.getCount(0.5));
  EXPECT_DOUBLE_EQ(10.0, hist.getCount(1.5));
  EXPECT_DOUBLE_EQ(5.0, hist.getCount(2.5));

  hist.clear();
  hist.addCount(0.5, 1.0);
  hist.addCount(1.5, 2.0);

  hist2.clear();
  hist2.setInterval(2.0);
  hist2.addCount(0.5, 3.0);
  hist2.addCount(1.5, 4.0);

  hist += hist2;

  EXPECT_DOUBLE_EQ(4.5, hist.getCount(0.5));
  EXPECT_DOUBLE_EQ(5.5, hist.getCount(1.5));

  hist.clear();
  hist.addCount(0.5, 1.0);
  hist.addCount(1.5, 2.0);

  hist2.clear();
  hist2.setInterval(2.0);
  hist2.addCount(-0.5, 3.0);
  hist2.addCount(0.5, 4.0);
  hist2.addCount(2.5, 1.0);

  hist += hist2;
  EXPECT_DOUBLE_EQ(1.5, hist.getCount(-1.5));
  EXPECT_DOUBLE_EQ(1.5, hist.getCount(-0.5));
  EXPECT_DOUBLE_EQ(3.0, hist.getCount(0.5));
  EXPECT_DOUBLE_EQ(4.0, hist.getCount(1.5));
  EXPECT_DOUBLE_EQ(0.5, hist.getCount(2.5));
  EXPECT_DOUBLE_EQ(0.5, hist.getCount(3.5));
}

TEST(ZHistogram, Divergence)
{
  std::vector<double> hist1(10, 0);
  std::vector<double> hist2(10, 0);

  EXPECT_DOUBLE_EQ(0, ZHistogram::computeJsDivergence(hist1, hist2));

  hist1[3] = 1;
  hist2[3] = 1;
  EXPECT_DOUBLE_EQ(0, ZHistogram::computeJsDivergence(hist1, hist2));

  hist2[3] = 0;
  hist2[4] = 1.0;
  EXPECT_DOUBLE_EQ(0.69314718055994529, ZHistogram::computeJsDivergence(hist1, hist2));

  hist2[3] = 0.5;
  hist2[4] = 0.5;
  EXPECT_DOUBLE_EQ(0.21576155433883565, ZHistogram::computeJsDivergence(hist1, hist2));
}

TEST(ZIntHistogram, basic)
{
  int *hist = neutu::array::IntMalloc(5);
  hist[0] = 3;
  hist[1] = 1;
  hist[2] = 2;
  hist[3] = 3;
  hist[4] = 4;

  ZIntHistogram histObj(hist);
  histObj.print();

  ASSERT_EQ(1, histObj.getMinValue());
  ASSERT_EQ(3, histObj.getMaxValue());
  ASSERT_EQ(3, histObj.getCount(2));
  ASSERT_EQ(0, histObj.getCount(5));
  ASSERT_EQ(0, histObj.getCount(0));

  ZIntHistogram histObj2(histObj);
  ASSERT_EQ(1, histObj2.getMinValue());
  ASSERT_EQ(3, histObj2.getMaxValue());
  ASSERT_EQ(3, histObj2.getCount(2));
  ASSERT_EQ(0, histObj2.getCount(5));
  ASSERT_EQ(0, histObj2.getCount(0));


}

#endif

#endif // ZHISTOGRAMTEST_H
