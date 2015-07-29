#ifndef ZJSONTEST_H
#define ZJSONTEST_H

#include "ztestheader.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "c_json.h"

#ifdef _USE_GTEST_

TEST(Json, basic)
{
  ZJsonObject obj;
  ASSERT_TRUE(obj.isEmpty());

  obj.setEntry("test1", 10);
  ASSERT_EQ(10, ZJsonParser::integerValue(obj["test1"]));
  obj.setEntry("test2", true);
  ASSERT_EQ(true, ZJsonParser::booleanValue(obj["test2"]));

  obj.print();

  json_t *value = json_integer(1);
  ZJsonObject obj2(value, ZJsonValue::SET_AS_IT_IS);
  ASSERT_TRUE(obj2.isEmpty());

  ZJsonValue obj3(value, ZJsonValue::SET_AS_IT_IS);
  ASSERT_FALSE(obj.isEmpty());

  ASSERT_EQ(1, obj3.toInteger());

  ZJsonArray arrayObj;
  ASSERT_TRUE(arrayObj.isEmpty());
  arrayObj.append(obj);
  ASSERT_EQ(1, (int) arrayObj.size());

  arrayObj.append(2);
  ASSERT_EQ(2, (int) arrayObj.size());
  ASSERT_EQ(2, ZJsonParser::integerValue(arrayObj.at(1)));

  arrayObj.append(3.0);
  ASSERT_EQ(3, (int) arrayObj.size());
  ASSERT_EQ(0, ZJsonParser::integerValue(arrayObj.at(2)));
}

TEST(ZJsonArray, basic)
{
  ZJsonArray arrayObj;
  ASSERT_TRUE(arrayObj.isEmpty());
  arrayObj.append(ZJsonObject());
  ASSERT_EQ(0, (int) arrayObj.size());

  ZJsonObject obj;
  obj.setEntry("test", 1);
  arrayObj.append(obj);
  ASSERT_EQ(1, (int) arrayObj.size());

  arrayObj.append(2);
  ASSERT_EQ(2, (int) arrayObj.size());
  ASSERT_EQ(2, ZJsonParser::integerValue(arrayObj.at(1)));

  arrayObj.append(3.0);
  ASSERT_EQ(3, (int) arrayObj.size());
  ASSERT_EQ(0, ZJsonParser::integerValue(arrayObj.at(2)));

  ZJsonArray arrayObj2;
  ASSERT_EQ(std::string("[]"), arrayObj2.dumpString());

}

TEST(ZJsonObject, basic)
{
  ZJsonObject obj;
  ASSERT_EQ(std::string("{}"), obj.dumpString());
}

#endif

#endif // ZJSONTEST_H
