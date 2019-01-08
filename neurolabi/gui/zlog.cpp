#include "zlog.h"

#include <QDateTime>

#include "core/utilities.h"
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
 * no Kafka broker is available, KLog works like ZLog, which saves messages
 * in a local log file.
 */

ZLog::ZLog()
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
    LINFO() << jsonDoc.toJson(QJsonDocument::Compact).toStdString().c_str();
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
  m_tags[key.c_str()] = value.toJson();
}

ZLog::Time::Time() :
  Tag("time", neutube::GetTimestamp())
{
}

ZLog::Time::Time(uint64_t t) : Tag("time", t)
{
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

KLog::KLog()
{
}

KLog::~KLog()
{
  endKLog();
}

//bool KLog::isStarted() const
//{
//  return m_span.get() != nullptr;
//}

void KLog::start()
{
  if (neuopentracing::Tracer::Global()) {
    m_span = neuopentracing::Tracer::Global()->StartSpan("app");
    if (m_span) {
      m_span->SetTag("user", NeutubeConfig::GetUserName());
      if (!m_span->hasTag("time")) {
        m_span->SetTag("time", neutube::GetTimestamp());
      }
      m_span->SetTag(
            "version", GET_SOFTWARE_NAME + " " + neutube::GetVersionString());
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
    m_span->SetTag(key, value);
  } else {
    ZLog::log(key, value);
  }
}

void KLog::endKLog()
{
  if (isStarted()) {
#ifdef _DEBUG_
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
