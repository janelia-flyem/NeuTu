#ifndef ZLOGTEST_H
#define ZLOGTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "logging/zlog.h"

namespace zlogtest {

class TestLog : public ZLog {
public:
  TestLog(const std::string &topic, EDestination dest = EDestination::KAFKA) :
    ZLog(topic, dest)
  {
  }

  ~TestLog() {
    _end();
  }

  std::string getResult() const {
    return m_result;
  }

  void end() override {
    _end();
  }

private:
  void endTestLog() {
    m_result = "";
    if (!m_tags.isEmpty()) {
      QJsonDocument jsonDoc(m_tags);
      m_result = jsonDoc.toJson(QJsonDocument::Compact).toStdString();
    }
  }

  void _end() {
    endTestLog();
    ZLog::end();
  }

private:
  std::string m_result;
};

}

TEST(ZLog, Tag)
{
  {
    ZLog log("topic");
    log << ZLog::Description("test");
  }

  {
    zlogtest::TestLog log("topic");
    log << ZLog::Description("test");
    log.end();
//    std::cout << log.getResult() << std::endl;
    ASSERT_EQ("{\"description\":\"test\"}", log.getResult());

    log << ZLog::Title("testtitle");
    log.end();
//    std::cout << log.getResult() << std::endl;
    ASSERT_EQ("{\"title\":\"testtitle\"}", log.getResult());

    log << ZLog::Title("title") << ZLog::Description("description");
    log.end();
    ASSERT_EQ("{\"description\":\"description\",\"title\":\"title\"}", log.getResult());
//    std::cout << log.getResult() << std::endl;
  }

}

#endif

#endif // ZLOGTEST_H
