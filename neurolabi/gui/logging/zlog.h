#ifndef ZLOG_H
#define ZLOG_H

#include <memory>
#include <iostream>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "neuopentracing.h"


class ZLog
{
public:
  enum class EDestination {
    AUTO, //Kafka if it's available; otherwise to local
    KAFKA, //Kafka only
    KAFKA_AND_LOCAL //Kafka if it's available; local logging as well
  };

  ZLog(EDestination dest = EDestination::AUTO);
  virtual ~ZLog();

  virtual void start();
  virtual void log(
      const std::string &key, const neuopentracing::Value &value, bool appending);
  virtual void end();
  virtual bool hasTag(const std::string &key) const;

  bool isStarted() const { return m_started; }

  struct Tag {
    Tag() {}

    Tag(const std::string &key, const neuopentracing::Value &value) :
      m_key(key), m_value(value) {}

    inline void set(const std::string &key, const neuopentracing::Value &value) {
      m_key = key;
      m_value = value;
    }

    std::string m_key;
    neuopentracing::Value m_value;
  };

  static void End(ZLog &log);

  struct Category : public Tag {
    static const char *KEY;
    Category(const std::string &value) : Tag(KEY, value) {}
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

  struct Debug : public Category {
    Debug() : Category("DEBUG") {}
  };

  struct Profile : public Category {
    Profile() : Category("PROFILE") {}
  };

  struct Interact : public Category {
    Interact() : Category("INTERACT") {}
  };

  struct Feedback: public Category {
    Feedback(): Category("FEEDBACK") {}
  };

  struct Duration : public Tag {
    static const char *KEY;
    Duration(int64_t t) : Tag(KEY, t) {}
  };

  struct Diagnostic : public Tag {
    static const char *KEY;
    Diagnostic(const std::string &value) : Tag(KEY, value) {}
  };

  struct Title : public Tag {
    static const char *KEY;
    Title(const std::string &value) : Tag(KEY, value) {}
  };

  struct Description : public Tag {
    static const char *KEY;
    Description(const std::string &value) : Tag(KEY, value) {}
  };

  struct Action : public Tag {
    Action(const std::string &value) : Tag("action", value) {}
  };

  struct Object : public Tag {
//    Object(const std::string &value)
//    {
//      QJsonObject obj{{"name", value.c_str()}};
//      set("object", neuopentracing::Value(obj));
//    }

    Object(const std::string &type, const std::string &name = "",
           const std::string &oid = "")
    {
      QJsonObject obj{
        {"type", type.c_str()},
        {"name", name.c_str()},
        {"id", oid.c_str()}
      };
      set("object", neuopentracing::Value(obj));
    }
  };

  struct Handle : public Tag {
    Handle(void *p);
  };

  struct Time : public Tag {
    static const char *KEY;
    Time();
    Time(uint64_t);
  };

  struct Level : public Tag {
    static const char *KEY;
    Level(int);
  };

  struct Window : public Tag {
    Window(const std::string &value) : Tag("window", value) {}
  };

  struct User: public Tag {
    User(const std::string &value): Tag("user", value) {}
  };

  struct AnonymousUser: public User {
    AnonymousUser(): User("****") {}
  };

  ZLog& operator << (const Tag &tag);
  ZLog& operator << (const std::function<void(ZLog&)> f);

private:
  void endLog();

protected:
  bool m_started = false;
  QJsonObject m_tags;
  EDestination m_dest = EDestination::AUTO;
//  bool m_localLogging = true;
};

class KLog : public ZLog
{
public:
  KLog(EDestination dest = EDestination::KAFKA);
  ~KLog() override;

  void start() override;
  void log(const std::string &key, const neuopentracing::Value &value,
           bool appending) override;
  void end() override;
  bool hasTag(const std::string &key) const override;

  static void SetOperationName(const std::string &name);
  static void ResetOperationName(); //reset to default

private:
  void _end();
  void endKLog();
  bool localLogging() const;

protected:
  std::unique_ptr<neuopentracing::Span> m_span;
  static std::string m_operationName;
};

class KInfo : public KLog
{
public:
//  KInfo();
//  KInfo(bool localogging);
  using KLog::KLog;
  using KLog::hasTag;

  KInfo& operator << (const char *info);
  KInfo& operator << (const std::string &info);
  KInfo& operator << (const QString &info);
};

class KWarn : public KLog
{
public:
  using KLog::KLog;

  KWarn& operator << (const char *info);
  KWarn& operator << (const std::string &info);
  KWarn& operator << (const QString &info);
};

class KError : public KLog
{
public:
  using KLog::KLog;

  KError& operator << (const char *info);
  KError& operator << (const std::string &info);
  KError& operator << (const QString &info);
};


#define KLOG KLog()
#define KINFO KInfo()
#define KWARN KWarn()
#define KERROR KError()

//Send to both kafka and local file
#define LKLOG KLog(ZLog::EDestination::KAFKA_AND_LOCAL)
#define LKINFO KInfo(ZLog::EDestination::KAFKA_AND_LOCAL)
#define LKWARN KWarn(ZLog::EDestination::KAFKA_AND_LOCAL)
#define LKERROR KError(ZLog::EDestination::KAFKA_AND_LOCAL)

//Auto
#define ZLOG KLog(ZLog::EDestination::AUTO)
#define ZINFO KInfo(ZLog::EDestination::AUTO)
#define ZWARN KWarn(ZLog::EDestination::AUTO)
#define ZERROR KError(ZLog::EDestination::AUTO)
#if defined(_DEBUG_)
#  define ZDEBUG ZINFO
#else
#  define ZDEBUG if (1) {} else ZINFO
#endif

#if defined(_DEBUG_)
#  define KDEBUG KLog()
#else
#  define KDEBUG if (1) {} else KLog()
#endif

#endif // ZLOG_H
