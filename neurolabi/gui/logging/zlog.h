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
  ZLog(bool localLogging = true);
  virtual ~ZLog();

  virtual void start();
  virtual void log(const std::string &key, const neuopentracing::Value &value);
  virtual void end();

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

  struct Debug : public Category {
    Debug() : Category("DEBUG") {}
  };

  struct Profile : public Category {
    Profile() : Category("PROFILE") {}
  };

  /*
  struct Interact : public Category {
    Interact() : Category("interact") {}
  };
  */

  struct Duration : public Tag {
    Duration(int64_t t) : Tag("duration", t) {}
  };

  struct Diagnostic : public Tag {
    Diagnostic(const std::string &value) : Tag("diagnostic", value) {}
  };

  struct Description : public Tag {
    Description(const std::string &value) : Tag("description", value) {}
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
    Time();
    Time(uint64_t);
  };

  struct Window : public Tag {
    Window(const std::string &value) : Tag("window", value) {}
  };

  ZLog& operator << (const Tag &tag);
  ZLog& operator << (const std::function<void(ZLog&)> f);

protected:
  inline bool localLogging() {
    return m_localLogging;
  }

private:
  void endLog();

private:
  bool m_started = false;
  QJsonObject m_tags;
  bool m_localLogging = true;
};

class KLog : public ZLog
{
public:
  KLog(bool localLogging = false);
  ~KLog();

  void start() override;
  void log(const std::string &key, const neuopentracing::Value &value) override;
  void end() override;
//  bool isStarted() const override;

  static void SetOperationName(const std::string &name);
  static void ResetOperationName(); //reset to default

private:
  void endKLog();

private:
  std::unique_ptr<neuopentracing::Span> m_span;
  static std::string m_operationName;
};

class KInfo : public KLog
{
public:
//  KInfo();
//  KInfo(bool localogging);
  using KLog::KLog;

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
#define LKLOG KLog(true)
#define LKINFO KInfo(true)
#define LKWARN KWarn(true)
#define LKERROR KError(true)

#if defined(_DEBUG_)
#  define KDEBUG KLog()
#else
#  define KDEBUG if (1) {} else KLog()
#endif

#endif // ZLOG_H
