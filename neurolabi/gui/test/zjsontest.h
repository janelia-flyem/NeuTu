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
  obj.setEntry("test3", true);
  ASSERT_EQ(true, ZJsonParser::booleanValue(obj["test3"]));

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

  ZJsonValue value2;
  ASSERT_TRUE(value2.isNull());

  value2.denull();
  ASSERT_FALSE(value2.isNull());

  ZJsonArray array;
  ASSERT_TRUE(array.isNull());

  array.denull();
  ASSERT_FALSE(array.isNull());

  ZJsonObject obj4;
  ASSERT_TRUE(obj4.isNull());

  obj4.denull();
  ASSERT_FALSE(obj4.isNull());
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

  ZJsonArray myList;
  obj.setEntry("key", myList);

  ASSERT_TRUE(obj.hasKey("key"));

  myList.append(12345);

  ZJsonArray arrayObj3(obj.value("key"));
  ASSERT_EQ(12345, ZJsonParser::integerValue(arrayObj3.at(0)));
}

TEST(ZJsonObject, basic)
{
  ZJsonObject obj;
  ASSERT_EQ(std::string("{}"), obj.dumpString());

  ZJsonObject entry;
  obj.setEntry("key", entry);
  ASSERT_TRUE(obj.hasKey("key"));
}

#endif

#endif // ZJSONTEST_H
