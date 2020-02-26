#ifndef QTCORETEST_H
#define QTCORETEST_H

#include "ztestheader.h"
#include "qt/core/utilities.h"

#ifdef _USE_GTEST_

TEST(qtcore, utilities) {

  ASSERT_EQ("https://test.com", neutu::NormalizeServerAddress("https://test.com", "https").
            toStdString());
  ASSERT_EQ("https://test.com", neutu::NormalizeServerAddress("test.com", "https").
            toStdString());
  ASSERT_EQ("http://test.com", neutu::NormalizeServerAddress("test.com", "http").
            toStdString());
  ASSERT_EQ("http://test.com", neutu::NormalizeServerAddress("test.com").
            toStdString());

  ASSERT_EQ("https://test.com",
            neutu::NormalizeServerAddress("https://test.com", "https").
            toStdString());
  ASSERT_EQ("https://test.com",
            neutu::NormalizeServerAddress("https://test.com/", "https").toStdString());
  ASSERT_EQ("https://test.com",
            neutu::NormalizeServerAddress("  https://test.com//", "https").toStdString());
  ASSERT_EQ("https://test.com",
            neutu::NormalizeServerAddress(" https://test.com/test", "https").toStdString());

  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress("https://test.com:8080", "https").
            toStdString());
  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress("test.com:8080", "https").
            toStdString());
  ASSERT_EQ("http://test.com:8080",
            neutu::NormalizeServerAddress("test.com:8080", "http").
            toStdString());

  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress("https://test.com:8080", "https").
            toStdString());
  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress("https://test.com:8080/", "https").toStdString());
  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress("  https://test.com:8080//", "https").toStdString());
  ASSERT_EQ("https://test.com:8080",
            neutu::NormalizeServerAddress(" https://test.com:8080/test", "https").toStdString());

}

#endif

#endif // QTCORETEST_H
