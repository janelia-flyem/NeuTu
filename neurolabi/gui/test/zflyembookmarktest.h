#ifndef ZFLYEMBOOKMARKTEST_H
#define ZFLYEMBOOKMARKTEST_H

#include "ztestheader.h"

#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "flyem/zflyembookmark.h"
#include "flyem/zflyembookmarkarray.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBookmark, basic)
{
  ZFlyEmBookmark bookmark;
  bookmark.setBookmarkType("Split");
  ASSERT_EQ(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE,
            bookmark.getBookmarkType());
  bookmark.setUser("test");
  ASSERT_EQ("test", bookmark.getUserName());
  bookmark.setUser("test2");
  ASSERT_EQ("test2", bookmark.getUserName());
  ASSERT_EQ("test", bookmark.getPrevUser());
  std::cout << bookmark.toString() << std::endl;

  bookmark.clear();
  ASSERT_EQ("", bookmark.getUserName());
  bookmark.setUser("test");
  ASSERT_EQ("test", bookmark.getUserName());
  bookmark.updateUser("");
  ASSERT_EQ("test", bookmark.getUserName());
  bookmark.updateUser("test2");
  ASSERT_EQ("test2", bookmark.getUserName());
  ASSERT_EQ("test", bookmark.getPrevUser());
  bookmark.setUser("test3");
  ASSERT_EQ("test3", bookmark.getUserName());
  ASSERT_EQ("test2", bookmark.getPrevUser());
  bookmark.updateUser("test4");
  ASSERT_EQ("test4", bookmark.getUserName());
  ASSERT_EQ("test3", bookmark.getPrevUser());
  bookmark.setUser("");
  ASSERT_EQ("", bookmark.getUserName());
  ASSERT_EQ("test4", bookmark.getPrevUser());
  bookmark.setUser("test5");
  ASSERT_EQ("test5", bookmark.getUserName());
  ASSERT_EQ("test4", bookmark.getPrevUser());
  bookmark.setUser("test5");
  ASSERT_EQ("test4", bookmark.getPrevUser());
  bookmark.setUser("test4");
  ASSERT_EQ("test5", bookmark.getPrevUser());

}

TEST(ZFlyEmBookmark, json)
{
  ZFlyEmBookmark bookmark;

  {
    ZJsonObject obj;
    obj.decode("{\"kind\": 1}", false);
    ASSERT_FALSE(bookmark.loadJsonObject(obj));
  }

  {
    ZJsonObject obj;
    obj.decode("{\"kind\": \"point\", \"pos\": [1802, 1106, 1025], "
               "\"prop\": {\"timestamp\": \"1612556183314\", \"user\": \"test\"},"
               "\"tags\": [], \"user\": \"test2\" }", false);
    ASSERT_TRUE(bookmark.loadJsonObject(obj));
    ASSERT_EQ(ZIntPoint(1802, 1106, 1025), bookmark.getLocation())
        << bookmark.getLocation();
    ZJsonObjectParser parser(bookmark.getPropJson());
    ASSERT_EQ("test2", bookmark.getUserName());
    ASSERT_EQ("test", bookmark.getPrevUser());
    ASSERT_EQ(1612556183314, bookmark.getTimestamp());

//    std::cout << bookmark.getTime().toStdString() << std::endl;
//    ASSERT_EQ("1612556183314", parser.getValue("timestamp", ""));
  }

  {
    ZJsonObject obj;
    obj.decode("{\"kind\": \"point\", \"pos\": [1802, 1106, 1025], "
               "\"prop\": {\"timestamp\": \"1612556183314\", \"user\": \"test\"},"
               "\"tags\": [], \"timestamp\": 1612556183.313, \"user\": \"\" }", false);
    ASSERT_TRUE(bookmark.loadJsonObject(obj));
    ZJsonObjectParser parser(bookmark.getPropJson());
    ASSERT_EQ("test", bookmark.getUserName());
//    ASSERT_EQ("", bookmark.getUserName());
    ASSERT_EQ("", bookmark.getPrevUser());
    ASSERT_EQ(1612556183313, bookmark.getTimestamp());
    ASSERT_FALSE(bookmark.isChecked());
  }

  {
    ZJsonObject obj;
    obj.decode(
          "{\"kind\": \"point\", \"pos\": [1802, 1106, 1025], "
          "\"prop\": {\"body ID\": \"1536878688\",\"user\": \"test\", "
          "  \"status\": \"traced\", \"type\":\"Split\"},"
          "\"tags\": [\"tag1\", \"tag2\"], \"user\": \"test2\","
          "\"timestamp\": 1612556183.312, \"verified\": true}",
          false);
    ASSERT_TRUE(bookmark.loadJsonObject(obj));
    ZJsonObjectParser parser(bookmark.getPropJson());
    ASSERT_EQ("test", parser.getValue("user", ""));
    ASSERT_EQ(1612556183312, bookmark.getTimestamp());
    ASSERT_EQ("traced", bookmark.getStatus());
    ASSERT_TRUE(bookmark.isChecked());
    ASSERT_EQ(1536878688, bookmark.getBodyId());
    ASSERT_EQ(2, bookmark.getTags().size());
    ASSERT_EQ("tag1", bookmark.getTags()[0]);
    ASSERT_EQ("tag2", bookmark.getTags()[1]);
    ASSERT_EQ("test2", bookmark.getUserName());
    ASSERT_EQ("test", bookmark.getPrevUser());
  }
}

TEST(ZFlyEmBookmarkArray, json)
{
  ZFlyEmBookmarkArray bookmarkArray;
  bookmarkArray.importJsonFile(
        GET_TEST_DATA_DIR + "/_test/json/clio_annotations_test.json");
  ASSERT_EQ(1, bookmarkArray.size());
  ZFlyEmBookmark bookmark = bookmarkArray[0];
  ASSERT_EQ(1536878688, bookmark.getBodyId());
  ASSERT_EQ("test", bookmark.getUserName());

  std::cout << bookmarkArray.toAnnotationJson() << std::endl;

  bookmarkArray.importJsonFile(
        GET_TEST_DATA_DIR + "/_test/json/clio_annotations.json");
  ASSERT_EQ(24, bookmarkArray.size());

//  std::cout << bookmarkArray.toAnnotationJson().dumpString(0) << std::endl;

  bookmarkArray.importJsonFile(
        GET_TEST_DATA_DIR + "/_test/json/bookmarks.json");
  ASSERT_EQ(8, bookmarkArray.size());

  bookmark = bookmarkArray[1];
  ASSERT_EQ(0, bookmark.getBodyId());
  ASSERT_EQ(1606934682320, bookmark.getTimestamp());
  ASSERT_EQ(
        ZFlyEmBookmark::EBookmarkType::FALSE_MERGE, bookmark.getBookmarkType());
  ASSERT_EQ("zhaot", bookmark.getUserName());
  ASSERT_EQ(ZIntPoint(1021, 931, 1024), bookmark.getLocation());
  ASSERT_EQ(1, bookmark.getTags().size());
  ASSERT_EQ("user:zhaot", bookmark.getTags()[0]);

  ZFlyEmBookmarkArray filtered = bookmarkArray.getBookmarkArray(
        ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
  std::cout << filtered.size() << std::endl;
  bookmark = filtered[0];
  ASSERT_EQ(ZIntPoint(1021, 931, 1024), bookmark.getLocation());

//  for (auto bookmark : bookmarkArray) {
//    bookmark.toJsonObject().print();
//  }
//  bookmarkArray.print();

}


#endif

#endif // ZFLYEMBOOKMARKTEST_H
