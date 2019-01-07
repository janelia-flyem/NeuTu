#include "neuopentracing.h"
#include <librdkafka/rdkafkacpp.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <chrono>
#include <iostream>

namespace neuopentracing {

Value::Value()
{
}

Value::Value(uint64_t value)
  : m_value(qint64(value))
{
}

Value::Value(int64_t value)
  : m_value(qint64(value))
{
}

Value::Value(int value)
  : m_value(qint64(value))
{
}

Value::Value(const std::vector<double>& value)
{
  QJsonArray array;
  for (double x : value) {
    array.append(x);
  }
  m_value = array;
}

Value::Value(const std::string& value)
  : m_value(value.c_str())
{
}

Value::Value(const char* value)
  : m_value(value)
{
}

Value::Value(QJsonValue value)
  : m_value(value)
{
}

QJsonValue Value::toJson() const
{
  return m_value;
}

//

namespace {
  std::uint64_t getTimestamp()
  {
      return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
  }
}

class Span::Impl
{
public:
  Impl(const std::shared_ptr<const Tracer> &tracer,
       const std::string &operationName)
    : m_tracer(tracer), m_operationName(operationName),
      m_startTimestamp(getTimestamp()) {}
  std::shared_ptr<const Tracer> m_tracer;
  std::string m_operationName;
  uint64_t m_startTimestamp;
  uint64_t m_finishTimestamp;
  std::map<std::string, Value> m_tags;
  bool m_finished = false;
};

Span::Span(const std::shared_ptr<const Tracer> &tracer,
           const std::string &operationName)
  : m_impl(new Impl(tracer, operationName))
{
}

// Mimics jaegertracing/Span.h
Span::~Span()
{
  Finish();
}

void Span::SetTag(const std::string& key, const Value& value)
{
  m_impl->m_tags[key] = value;
}

const std::string& Span::operationName() const
{
  return m_impl->m_operationName;

}
const std::map<std::string, Value>& Span::tags() const
{
  return m_impl->m_tags;
}

void Span::Finish()
{
  if (!m_impl->m_finished) {
    m_impl->m_finished = true;
    m_impl->m_finishTimestamp = getTimestamp();
    if (m_impl->m_tracer) {
      m_impl->m_tracer->reportSpan(*this);
    }
  }
}

uint64_t Span::startTimestamp() const
{
  return m_impl->m_startTimestamp;
}

uint64_t Span::finishTimestamp() const
{
  return m_impl->m_finishTimestamp;
}

//

class Config::Impl
{
public:
  Impl(const std::string& kafkaBrokers) : m_kafkaBrokers(kafkaBrokers) {}
  std::string m_kafkaBrokers;
  std::uint32_t m_kafkaPartition = 0;
};

Config::Config(const std::string &kafkaBrokers)
  : m_impl(new Impl(kafkaBrokers))
{
}

Config::Config(const Config &other)
  : m_impl(new Impl(*other.m_impl.get()))
{
}

Config::~Config()
{
}

const std::string& Config::kafkaBrokers() const
{
  return m_impl->m_kafkaBrokers;
}

std::uint32_t Config::kafkaPartition() const
{
  return m_impl->m_kafkaPartition;
}

//

class Tracer::Impl
{
public:
  Impl(const std::string& serviceName, const Config& config)
    : m_serviceName(serviceName), m_config(config) {}
  std::string m_serviceName;
  Config m_config;
  RdKafka::Conf* m_kafkaConf = nullptr;
  RdKafka::Producer* m_kafkaProducer = nullptr;
};

std::shared_ptr<Tracer> Tracer::make(const std::string& serviceName,
                                     const Config& config)
{
  if (serviceName.empty()) {
      throw std::invalid_argument("no service name provided");
  }

  return std::shared_ptr<Tracer>(new Tracer(serviceName, config));
}

std::unique_ptr<Span> Tracer::StartSpan(const std::string& operationName)
{
  return std::unique_ptr<Span>(new Span(shared_from_this(), operationName));
}

namespace {

  static std::shared_ptr<Tracer>& getGlobalTracer()
  {
    static std::shared_ptr<Tracer> globalTracer = nullptr;
    return globalTracer;
  }
}

std::shared_ptr<Tracer> Tracer::Global() noexcept
{
  return getGlobalTracer();
}

std::shared_ptr<Tracer> Tracer::InitGlobal(std::shared_ptr<Tracer> tracer) noexcept
{
  getGlobalTracer().swap(tracer);
  return tracer;
}

Tracer::Tracer(const std::string& serviceName, const Config& config)
  : m_impl(new Impl(serviceName, config))
{
  m_impl->m_kafkaConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  std::string errstr;
  if (m_impl->m_kafkaConf->set("metadata.broker.list", config.kafkaBrokers(), errstr) !=
      RdKafka::Conf::CONF_OK) {
    // TODO: Report errstr?
    throw "Failed to set kafka brokders.";
    return;
  }

  m_impl->m_kafkaProducer = RdKafka::Producer::create(m_impl->m_kafkaConf, errstr);
  if (!m_impl->m_kafkaProducer) {
    // TODO: Report errstr?
    throw "Failed to create kafka producer.";
    return;
  }
}

void Tracer::reportSpan(const Span &span) const
{
  bool kafkaSucceeded = false;
  if (m_impl->m_kafkaProducer) {
    QJsonObject json;
    for (auto it : span.tags()) {
      json[it.first.c_str()] = it.second.toJson();
    }
    QJsonDocument jsonDoc(json);
    std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());

    std::string topic = m_impl->m_serviceName + "-" + span.operationName();
    RdKafka::ErrorCode resp = m_impl->m_kafkaProducer->
        produce(topic, m_impl->m_config.kafkaPartition(),
                RdKafka::Producer::RK_MSG_COPY,
                const_cast<char *>(jsonStr.c_str()), jsonStr.size(),
                nullptr, 0, span.finishTimestamp(), nullptr);
    if (resp == RdKafka::ERR_NO_ERROR) {
      kafkaSucceeded = true;
    } else {
      // TODO: Report RdKafka::err2str(resp)?
    }
  }
  if (!kafkaSucceeded) {
    // TODO: Fall back to an alternative form of logging, e.g., to a DVID instance.
  }
}

}
