#include "zlog.h"

#include <QDateTime>
#include <sstream>

#include "common/utilities.h"
#include "neutubeconfig.h"
#include "zqslog.h"

/* Implementation details
 *
 * ZLog is a class of managing elk-style logging, which is designed to leverage
 * the Kafka system. The class name might be too general because I don't have
 * a clear model in my mind yet. The class is designed to work like streaming:
 *   ZLog() << ... << ...
 * It relies on a flag to start logging, and the destructor to end logging.
 * The input to the stream should be a ZLog::Tag object, which has several
 * derived forms shaped by the ELK specification.
 *
 * KLog is derived from ZLog to serve a wrapper of neuopentracing APIs. When
 * no Kafka broker is available, KLog works like ZLog, which tries to save messages
 * in a local log file.
 */

ZLog::ZLog(const std::string &topic, EDestination dest) :
  m_topic(topic), m_dest(dest)
{
}

ZLog::~ZLog()
{
  endLog();
}

void ZLog::start()
{
  m_tags = QJsonObject();
  m_started = true;
}

void ZLog::endLog()
{
  if (!m_tags.isEmpty()) {
    QJsonDocument jsonDoc(m_tags);
    if (m_dest == EDestination::KAFKA_AND_LOCAL ||
        m_dest == EDestination::AUTO) {
      LINFO_NLN() << jsonDoc.toJson(QJsonDocument::Compact).toStdString().c_str();
    }
    m_tags = QJsonObject();
  }
  m_started = false;
}

void ZLog::end()
{
  endLog();
}

bool ZLog::hasTag(const std::string &key) const
{
  return m_tags.contains(key.c_str());
}

void ZLog::log(
    const std::string &key, const neuopentracing::Value &value, bool appending)
{
  if (appending && m_tags.contains(key.c_str())) {
    m_tags[key.c_str()] = neuopentracing::ToString(m_tags.value(key.c_str()))
        + " " +
        neuopentracing::ToString(value);
  } else {
    m_tags[key.c_str()] = value.toJson();
  }
}

ZLog::Time::Time() :
  Tag("time", neutu::GetTimestamp())
{
}

ZLog::Time::Time(uint64_t t) : Tag("time", t)
{
}

ZLog::Level::Level(int level) : Tag("level", level)
{
}

ZLog::Handle::Handle(void *p) : Tag("", "")
{
  std::ostringstream stream;
  stream << p;
  set("handle", stream.str());
}

void ZLog::End(ZLog &log)
{
  log.end();
}

ZLog& ZLog::operator << (const ZLog::Tag &tag)
{
  bool appending = (
        tag.m_key == Description::KEY ||
        tag.m_key == Diagnostic::KEY
      );

  log(tag.m_key, tag.m_value, appending);

  return *this;
}

ZLog& ZLog::operator << (const std::function<void(ZLog&)> f)
{
  f(*this);

  return *this;
}

namespace {
#if defined(_DEBUG_)
const char* DEFAULT_OPERATION_NAME = "debug";
#else
const char* DEFAULT_OPERATION_NAME = "app";
#endif
}

const char* ZLog::Category::KEY = "category";
const char* ZLog::Title::KEY = "title";
const char* ZLog::Description::KEY = "description";
const char* ZLog::Duration::KEY = "duration";
const char* ZLog::Time::KEY = "time";
const char* ZLog::Diagnostic::KEY = "diagnostic";
const char* ZLog::Level::KEY = "level";

std::string KLog::m_operationName = DEFAULT_OPERATION_NAME;

void KLog::SetOperationName(const std::string &name)
{
  m_operationName = name;
}

void KLog::ResetOperationName()
{
  m_operationName = DEFAULT_OPERATION_NAME;
}

KLog::KLog(const std::string &topic, EDestination dest) : ZLog(topic, dest)
{
}

KLog::~KLog()
{
  _end();
}

//bool KLog::isStarted() const
//{
//  return m_span.get() != nullptr;
//}

void KLog::start()
{
  if (neuopentracing::Tracer::Global()) {
    // Kafka topic naming:
    //   * Lower case only
    //   * Use - to separate domains. Although using . seems more readable, it
    //     complicates topic regex specification on the consumer side. Another
    // Example: neutu-app-proofreading-merge.
    m_span = neuopentracing::Tracer::Global()->StartSpan(
          m_operationName + (m_topic.empty() ? "" : ("-" + m_topic)));

    if (m_span) {
      neutu::UserInfo userInfo = NeutubeConfig::GetUserInfo();
      m_span->SetTag("user", userInfo.getUserName());
      if (!userInfo.getOrganization().empty()) {
        m_span->SetTag("organization", userInfo.getOrganization());
      }
      if (!userInfo.getLocation().empty()) {
        m_span->SetTag("location", userInfo.getLocation());
      }

      if (!m_span->hasTag("time")) {
        m_span->SetTag("time", neutu::GetTimestamp());
      }
      m_span->SetTag(
            "version", GET_SOFTWARE_NAME + " " + neutu::GetVersionString());
    }
  }

  ZLog::start();
}

bool KLog::localLogging() const
{
  bool local = (m_dest == EDestination::KAFKA_AND_LOCAL);

  if (!local) {
    if (!m_span) {
      local = (m_dest == EDestination::AUTO);
    }
  }

  return local;
}

void KLog::log(
    const std::string &key, const neuopentracing::Value &value, bool appending)
{
  if (!isStarted()) {
    start();
  }

  if (m_span) {
    if (appending) {
      m_span->appendTag(key, value);
    } else {
      m_span->SetTag(key, value);
    }
//    m_span->SetTag(key, value);
  }

  if (localLogging()) {
    ZLog::log(key, value, appending);
  }
}

void KLog::endKLog()
{
  if (isStarted()) {
#ifdef _DEBUG_2
    std::cout << "KLog::end() called." << std::endl;
#endif

    if (m_span) {
      m_span->Finish();
    }
  }
}

// For avoiding calling virtual end() in destructor
void KLog::_end()
{
  endKLog();

  ZLog::end();
}

void KLog::end()
{
  _end();
}

bool KLog::hasTag(const std::string &key) const
{
  if (m_span) {
    return m_span->hasTag(key);
  }

  return ZLog::hasTag(key);
}

//KInfo::KInfo()
//{
//}

//KInfo::KInfo(bool localLogging) : ZLog(localLogging)
//{
//}

KInfo& KInfo::operator << (const char* info)
{
  if (info) {
    if (hasTag(ZLog::Description::KEY)) {
      dynamic_cast<KLog&>(*this) << ZLog::Description(info);
    } else {
      dynamic_cast<KLog&>(*this) << ZLog::Info() << ZLog::Description(info);
    }
  }

  return (*this);
}

KInfo& KInfo::operator << (const std::string &info)
{
  (*this) << info.c_str();

  return (*this);
}

KInfo& KInfo::operator << (const QString &info)
{
  (*this) << info.toStdString();

  return (*this);
}

KWarn& KWarn::operator << (const char* info)
{
  if (info) {
    if (hasTag(ZLog::Description::KEY)) {
      dynamic_cast<KLog&>(*this) << ZLog::Description(info);
    } else {
      dynamic_cast<KLog&>(*this) << ZLog::Warn() << ZLog::Description(info);
    }
  }
//  dynamic_cast<KLog&>(*this) << ZLog::Warn() << ZLog::Description(info);

  return (*this);
}

KWarn& KWarn::operator << (const std::string &info)
{
  (*this) << info.c_str();

  return (*this);
}

KWarn& KWarn::operator << (const QString &info)
{
  (*this) << info.toStdString();

  return (*this);
}

KError& KError::operator << (const char* info)
{
  if (info) {
    if (hasTag(ZLog::Description::KEY)) {
      dynamic_cast<KLog&>(*this) << ZLog::Description(info);
    } else {
      dynamic_cast<KLog&>(*this) << ZLog::Error() << ZLog::Description(info);
    }
  }
//  dynamic_cast<KLog&>(*this) << ZLog::Error() << ZLog::Description(info);

  return (*this);
}

KError& KError::operator << (const std::string &info)
{
  (*this) << info.c_str();

  return (*this);
}

KError& KError::operator << (const QString &info)
{
  (*this) << info.toStdString();

  return (*this);
}
