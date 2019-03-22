#ifndef ZIMAGETEST_H
#define ZIMAGETEST_H

#include "ztestheader.h"
#include "zimage.h"
#include "zstackfactory.h"
#include "zstack.hxx"
#include "zcontrastprotocol.h"

#ifdef _USE_GTEST_

TEST(ZImage, basic)
{
  ZImage image(10, 5);

  image.setBackground();
  QRgb color = image.pixel(0, 0);
  ASSERT_EQ(0, qRed(color));
  ASSERT_EQ(0, qBlue(color));
  ASSERT_EQ(0, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));
}

TEST(ZImage, setData)
{
  ZImage image(10, 5);
  ZStack *stack = ZStackFactory::makeIndexStack(2, 3, 4);
  image.setData(stack, 0);

  QRgb color = image.pixel(0, 0);
  ASSERT_EQ(0, qRed(color));
  ASSERT_EQ(0, qBlue(color));
  ASSERT_EQ(0, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  color = image.pixel(1, 0);
  ASSERT_EQ(1, qRed(color));
  ASSERT_EQ(1, qBlue(color));
  ASSERT_EQ(1, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  color = image.pixel(1, 1);
  ASSERT_EQ(3, qRed(color));
  ASSERT_EQ(3, qBlue(color));
  ASSERT_EQ(3, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  image.setData(stack, 1);
  color = image.pixel(0, 0);
  ASSERT_EQ(6, qRed(color));
  ASSERT_EQ(6, qBlue(color));
  ASSERT_EQ(6, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  image.setBackground();
  stack->setOffset(7, 3, 3);
  image.setData(stack, 0);
  color = image.pixel(0, 0);
  ASSERT_EQ(0, qRed(color));
  ASSERT_EQ(0, qBlue(color));
  ASSERT_EQ(0, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  color = image.pixel(7, 4);
  ASSERT_EQ(0, qRed(color));
  ASSERT_EQ(0, qBlue(color));
  ASSERT_EQ(0, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  image.setData(stack, 3);
  color = image.pixel(0, 0);
  ASSERT_EQ(0, qRed(color));
  ASSERT_EQ(0, qBlue(color));
  ASSERT_EQ(0, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));

  color = image.pixel(7, 4);
  ASSERT_EQ(2, qRed(color));
  ASSERT_EQ(2, qBlue(color));
  ASSERT_EQ(2, qGreen(color));
  ASSERT_EQ(255, qAlpha(color));
}

TEST(ZContrastProtocol, Basic)
{
  ZContrastProtocol cp(0, 1, ZContrastProtocol::ENonlinearMode::NONE);
  ASSERT_TRUE(cp.hasNoEffect());
  ASSERT_EQ(1, cp.mapGrey(1));
}

#endif


#endif // ZIMAGETEST_H
