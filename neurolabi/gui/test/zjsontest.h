#ifndef ZJSONTEST_H
#define ZJSONTEST_H

#include "c_json.h"
#include "ztestheader.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zstring.h"

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
  ASSERT_EQ(int64_t(0), ZJsonParser::integerValue(arrayObj.at(2)));

  ZJsonValue value2;
  ASSERT_TRUE(value2.shellOnly());

  value2.denull();
  ASSERT_FALSE(value2.shellOnly());

  ZJsonArray array;
  ASSERT_TRUE(array.shellOnly());

  array.denull();
  ASSERT_FALSE(array.shellOnly());

  ZJsonObject obj4;
  ASSERT_TRUE(obj4.shellOnly());

  obj4.denull();
  ASSERT_FALSE(obj4.shellOnly());
}

TEST(ZJsonValue, decode)
{
  ZJsonArray array;
  array.decodeString("[1, 2, 3]");
  ASSERT_EQ(3, (int) array.size());

  array.decodeString("1, 2, 3");
  array.decodeString("1, 2, 3", NULL);
  json_error_t error;
  array.decodeString("1, 2, 3", &error);

  ZJsonObject obj;
  obj.decodeString("");
  ASSERT_TRUE(obj.isEmpty());
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
  ASSERT_EQ(int64_t(12345), ZJsonParser::integerValue(arrayObj3.at(0)));
}

TEST(ZJsonObject, basic)
{
  {
    ZJsonObject obj;
    ASSERT_EQ(std::string("{}"), obj.dumpString());

    ZJsonObject entry;
    obj.setEntry("key", entry);
    ASSERT_TRUE(obj.hasKey("key"));

    ZJsonObject entry2;
    obj.addEntry("key2", entry2);
    ASSERT_TRUE(obj.hasKey("key"));

    obj.addEntry("key3", "test");
    ASSERT_EQ("test", ZJsonParser::stringValue(obj["key3"]));

    obj.addEntry("key3", "test2");
    ASSERT_EQ("test", ZJsonParser::stringValue(obj["key3"]));

    obj.addEntry("key4", "test2");
    ASSERT_EQ("test2", ZJsonParser::stringValue(obj["key4"]));

    ZJsonObject obj2;
    obj2.setEntry("key5", "test3");
    obj2.setEntry("key4", "test5");

    obj.addEntryFrom(obj2);
    ASSERT_EQ("test3", ZJsonParser::stringValue(obj["key5"]));
    ASSERT_EQ("test2", ZJsonParser::stringValue(obj["key4"]));

    obj.setNonEmptyEntry("key6", "");
    ASSERT_FALSE(obj.hasKey("key6"));

    obj.setNonEmptyEntry("key6", " ");
    ASSERT_TRUE(obj.hasKey("key6"));

    obj.setTrueEntry("key7", false);
    ASSERT_FALSE(obj.hasKey("key7"));

    obj.setTrueEntry("key7", true);
    ASSERT_TRUE(obj.hasKey("key7"));
  }

  {
    ZJsonObject obj;
    obj.setEntry("test", 1).
        setEntry("test2", true).
        setEntry("test3", "value").
        setEntry("test4", int64_t(100)).
        setEntry("test5", uint64_t(200)).
        setEntry("test6", 1.0).
        setEntry("test7", {"v1", "v2", "v3"});
    ASSERT_EQ(7, obj.countKey());
    ASSERT_EQ(1, ZJsonObjectParser::GetValue(obj, "test", 0));
    ASSERT_EQ(true, ZJsonObjectParser::GetValue(obj, "test2", false));
    ASSERT_EQ("value", ZJsonObjectParser::GetValue(obj, "test3", ""));
    ASSERT_EQ(100, ZJsonObjectParser::GetValue(obj, "test4", int64_t(0)));
    ASSERT_EQ(uint64_t(200), ZJsonObjectParser::GetValue(obj, "test5", uint64_t(0)));
    ASSERT_EQ(1.0, ZJsonObjectParser::GetValue(obj, "test6", 0.0));
    ZJsonArray array(obj.value("test7"));
    ASSERT_EQ(3, int(array.size()));
    auto stringArray = array.toStringArray();
    ASSERT_EQ("v1", stringArray[0]);
    ASSERT_EQ("v2", stringArray[1]);
    ASSERT_EQ("v3", stringArray[2]);

    std::vector<double> darray{1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<int> iarray{1, 2, 3, 4};
    obj.setEntry("test8", darray.data(), darray.size()).
        setEntry("test9", iarray.data(), iarray.size());
    auto darray2 = ZJsonArray(obj.value("test8")).toNumberArray();
    auto iarray2 = ZJsonArray(obj.value("test9")).toIntegerArray();
    ASSERT_EQ(5, int(darray2.size()));
    ASSERT_EQ(4, int(iarray2.size()));
    ASSERT_EQ(1.0, darray2[0]);
    ASSERT_EQ(5.0, darray2[4]);
    ASSERT_EQ(1, iarray2[0]);
    ASSERT_EQ(4, iarray2[3]);
  }

  {
    ZJsonObject obj;
    obj.setEntry("test", 1).
        setEntry("test2", true).
        setEntry("test3", "value").
        setEntry("test4", int64_t(100)).
        setEntry("test5", uint64_t(200)).
        setEntry("test6", 1.0).
        setEntry("test7", {"v1", "v2", "v3"});
    ASSERT_EQ(7, obj.removeKeys([&](const std::string &key, ZJsonValue /*value*/) {
      return ZString(key).startsWith("test");
    }));
    ASSERT_TRUE(obj.isEmpty());

    obj.setEntry("test", 1).
        setEntry("test2", true).
        setEntry("test3", "value").
        setEntry("test4", int64_t(100)).
        setEntry("test5", uint64_t(200)).
        setEntry("test6", 1.0).
        setEntry("test7", {"v1", "v2", "v3"});
    ASSERT_EQ(3, obj.removeKeys([&](const std::string &/*key*/, ZJsonValue value) {
      return value.isInteger();
    }));

    obj.setEntry("test", ZJsonValue::MakeNull());
    obj.setEntry("test2", ZJsonValue::MakeNull());
    ASSERT_EQ(2, obj.removeNullFields());
  }
}

TEST(ZJsonParser, basic)
{
  ASSERT_TRUE(ZJsonParser::IsObject("{\"test\": \"object\"}"));
  ASSERT_TRUE(ZJsonParser::IsObject("{}"));
  ASSERT_FALSE(ZJsonParser::IsObject("test"));
  ASSERT_FALSE(ZJsonParser::IsObject("[1, 2, 3]"));
}

TEST(ZJsonObjectParser, Basic)
{
  ZJsonObject obj;
  obj.setEntry("test1", 10);
  obj.setEntry("test3", true);
  obj.setEntry(("key1"), "v1");

  ASSERT_EQ(10, ZJsonObjectParser::GetValue(obj, "test1", 1));
  ASSERT_EQ(1, ZJsonObjectParser::GetValue(obj, "test2", 1));
  ASSERT_EQ(true, ZJsonObjectParser::GetValue(obj, "test3", false));
  ASSERT_EQ("v1", ZJsonObjectParser::GetValue(obj, "key1", ""));

  ASSERT_EQ(10, ZJsonObjectParser::GetValue(obj, std::vector<std::string>{"test1", "test2"}, 1));
  ASSERT_EQ(10, ZJsonObjectParser::GetValue(obj, std::vector<std::string>{"test2", "test1"}, 1));
  ASSERT_EQ(1, ZJsonObjectParser::GetValue(obj, std::vector<std::string>{"test4", "test5"}, 1));
  ASSERT_EQ("v1", ZJsonObjectParser::GetValue(obj, std::vector<std::string>{"key2", "key1"}, std::string("")));
  ASSERT_EQ("", ZJsonObjectParser::GetValue(obj, std::vector<std::string>{"key2", "key3"}, std::string()));

  ZJsonObjectParser parser(obj);
  ASSERT_EQ(10, parser.getValue("test1", 1));
  ASSERT_EQ(1, parser.getValue("test2", 1));
  ASSERT_EQ(true, parser.getValue("test3", false));
  ASSERT_EQ("v1", parser.getValue("key1", ""));
}

TEST(ZJsonArray, Iter)
{
  {
    ZJsonArray array;
    array.append(1);
    array.append(2);
    array.append(3);
    array.append(4);
    array.append(5);

    ZJsonArray result = array.filter([](const ZJsonValue &value) {
      return (ZJsonParser::integerValue(value.getData()) > 3);
    });
    ASSERT_EQ(2, int(result.size()));
    auto intArray = result.toIntegerArray();
    ASSERT_EQ(4, intArray[0]);
    ASSERT_EQ(5, intArray[1]);
  }
}

#endif

#endif // ZJSONTEST_H
