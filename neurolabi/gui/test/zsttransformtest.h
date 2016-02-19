#ifndef ZSTTRANSFORMTEST_H
#define ZSTTRANSFORMTEST_H

#include "ztestheader.h"
#include <QPoint>
#include <QPointF>
#include <QRectF>

#include "neutubeconfig.h"
#include "zsttransform.h"

#ifdef _USE_GTEST_

TEST(ZStTransform, Basic) {
  ZStTransform transform;
  ASSERT_TRUE(transform.isIdentity());
  ASSERT_FALSE(transform.hasOffset());
  ASSERT_FALSE(transform.hasScale());

  ZPoint pt(1, 2, 3);

  ZPoint pt2 = transform.transform(pt);

  ASSERT_TRUE(pt2 == pt);

  transform.setOffset(3, 4, 5);
  pt2 = transform.transform(pt);
  ASSERT_EQ(4.0, pt2.x());
  ASSERT_EQ(6.0, pt2.y());
  ASSERT_EQ(8.0, pt2.z());

  transform.setScale(2, 3, 4);
  pt2 = transform.transform(pt);
  ASSERT_EQ(5.0, pt2.x());
  ASSERT_EQ(10.0, pt2.y());
  ASSERT_EQ(17.0, pt2.z());

  ASSERT_EQ(3.0, transform.getTx());
  ASSERT_EQ(4.0, transform.getTy());
  ASSERT_EQ(5.0, transform.getTz());

  ASSERT_EQ(2.0, transform.getSx());
  ASSERT_EQ(3.0, transform.getSy());
  ASSERT_EQ(4.0, transform.getSz());

  pt2 = transform.getInverseTransform().transform(pt2);
  ASSERT_DOUBLE_EQ(pt2.x(), pt.x());
  ASSERT_DOUBLE_EQ(pt2.y(), pt.y());
  ASSERT_DOUBLE_EQ(pt2.z(), pt.z());

  QPointF qpt(1, 2);
  QPointF qpt2 = transform.transform(qpt);
  ASSERT_EQ(5.0, qpt2.x());
  ASSERT_EQ(10.0, qpt2.y());

  QRectF rect(1, 2, 3, 4);
  QRectF rect2 = transform.transform(rect);
  ASSERT_EQ(5.0, rect2.left());
  ASSERT_EQ(10.0, rect2.top());
  ASSERT_EQ(6.0, rect2.width());
  ASSERT_EQ(12.0, rect2.height());

//  std::cout << pt2.toString() << std::endl;
}

#endif


#endif // ZSTTRANSFORMTEST_H
