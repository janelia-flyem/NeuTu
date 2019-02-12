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

ZLog::ZLog(bool localLogging) : m_localLogging(localLogging)
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
    if (m_localLogging) {
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

void ZLog::log(const std::string &key, const neuopentracing::Value &value)
{
  if (m_tags.contains(key.c_str())) {
    m_tags[key.c_str()] = neuopentracing::ToString(m_tags.value(key.c_str())) +
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
  log(tag.m_key, tag.m_value);

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

std::string KLog::m_operationName = DEFAULT_OPERATION_NAME;

void KLog::SetOperationName(const std::string &name)
{
  m_operationName = name;
}

void KLog::ResetOperationName()
{
  m_operationName = DEFAULT_OPERATION_NAME;
}

KLog::KLog(bool localLogging) : ZLog(localLogging)
{
}

KLog::~KLog()
{
  end();
}

//bool KLog::isStarted() const
//{
//  return m_span.get() != nullptr;
//}

void KLog::start()
{
  if (neuopentracing::Tracer::Global()) {
    m_span = neuopentracing::Tracer::Global()->StartSpan(m_operationName);

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

void KLog::log(const std::string &key, const neuopentracing::Value &value)
{
  if (!isStarted()) {
    start();
  }

  if (m_span) {
    m_span->appendTag(key, value);
//    m_span->SetTag(key, value);
  }

  if (localLogging()) {
    ZLog::log(key, value);
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

void KLog::end()
{
  endKLog();

  ZLog::end();
}

//KInfo::KInfo()
//{
//}

//KInfo::KInfo(bool localLogging) : ZLog(localLogging)
//{
//}

const char* ZLog::Description::KEY = "description";

KInfo& KInfo::operator << (const char* info)
{
  if (info) {
    if (m_tags.contains(ZLog::Description::KEY)) {
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
    if (m_tags.contains(ZLog::Description::KEY)) {
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
    if (m_tags.contains(ZLog::Description::KEY)) {
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
