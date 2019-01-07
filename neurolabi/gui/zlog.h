#ifndef ZLOG_H
#define ZLOG_H

#include <memory>
#include <iostream>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>

#include "neuopentracing.h"

class ZLog
{
public:
  ZLog();
  virtual ~ZLog();

  virtual void start();
  virtual void log(const std::string &key, const neuopentracing::Value &value);
  virtual void end();

  bool isStarted() const { return m_started; }

  struct Tag {
    Tag(const std::string &key, const std::string &value) :
      m_key(key), m_value(value) {}
    Tag(const std::string &key, const int64_t &value) :
      m_key(key), m_value(value) {}

    std::string m_key;
    neuopentracing::Value m_value;
  };

  static void End(ZLog &log);

  struct Category : public Tag {
    Category(const std::string &value) : Tag("category", value) {}
  };

  struct Info : public Category {
    Info() : Category("INFO") {}
  };

  struct Warn : public Category {
    Warn() : Category("WARN") {}
  };

  struct Error : public Category {
    Error() : Category("ERROR") {}
  };

  struct Duration : public Tag {
    Duration(int64_t t) : Tag("duration", t) {}
  };

  struct Diagnostic : public Tag {
    Diagnostic(const std::string &value) : Tag("diagnostic", value) {}
  };

  struct Description : public Tag {
    Description(const std::string &value) : Tag("description", value) {}
  };

  struct Time : public Tag {
    Time();
    Time(uint64_t);
  };

  ZLog& operator << (const Tag &tag);
  ZLog& operator << (const std::function<void(ZLog&)> f);

private:
  void endLog();

private:
  bool m_started = false;
  QJsonObject m_tags;
};

class KLog : public ZLog
{
public:
  KLog();
  ~KLog();

  void start() override;
  void log(const std::string &key, const neuopentracing::Value &value) override;
  void end() override;
//  bool isStarted() const override;

private:
  void endKLog();

private:
  std::unique_ptr<neuopentracing::Span> m_span;
};


#endif // ZLOG_H
