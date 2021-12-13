#ifndef ZSEGMENTANNOTATIONTEST_H
#define ZSEGMENTANNOTATIONTEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"

#include "zjsonobjectparser.h"
#include "mvc/annotation/zsegmentannotationbuilder.h"

TEST(ZSegmentAnnotationBuilder, Basic)
{
  ZJsonObject obj;
  obj.setEntry("test", 1);

  ZJsonObject obj2 = ZSegmentAnnotationBuilder().copy(obj);
  ASSERT_EQ(1, ZJsonObjectParser::GetValue(obj2, "test", 0));

  obj.setEntry("test", 2);
  ASSERT_EQ(2, ZJsonObjectParser::GetValue(obj, "test", 0));
  ASSERT_EQ(1, ZJsonObjectParser::GetValue(obj2, "test", 0));

  ZJsonObject obj3;
  obj3.setEntry("key2", 5);
  obj2 = ZSegmentAnnotationBuilder().copy(obj).join(obj3);
  ASSERT_EQ(2, ZJsonObjectParser::GetValue(obj2, "test", 0));
  ASSERT_EQ(5, ZJsonObjectParser::GetValue(obj2, "key2", 0));

  ZJsonObject obj4;
  obj4.setEntry("test", 4);
  obj4.setEntry("key3", "");
  ZSegmentAnnotationBuilder(obj2).join(obj4);;
  ASSERT_EQ(4, ZJsonObjectParser::GetValue(obj2, "test", 0));
  ASSERT_EQ(5, ZJsonObjectParser::GetValue(obj2, "key2", 0));
  ASSERT_TRUE(obj4.hasKey("key3"));
  ASSERT_EQ("", ZJsonObjectParser::GetValue(obj4, "key3", "default"));

//  std::cout << obj2 << std::endl;
//  std::cout << obj4 << std::endl;
}

#endif

#endif // ZSEGMENTANNOTATIONTEST_H
