#ifndef NEUOPENTRACING_H
#define NEUOPENTRACING_H

#include <QJsonValue>
#include <map>
#include <memory>
#include <string>
#include <vector>

// A very cheap and cheerful mimic of a bit of the OpenTracing C++ API
// (https://github.com/opentracing/opentracing-cpp)
// and a bit of the Jaeger implemenation of it
// (https://github.com/jaegertracing/jaeger-client-cpp).
// All that is supported is enough functionality to send simple spans
// with simple tags to Kafka.  But the API looks enough like Jaeger
// that it would not be difficult to switch to that system, if more
// powerful capabilities are needed.

// Member functions whose first letter is capitalized are defined in the
// OpenTracing C++ API.

namespace neuopentracing {

class Value
{
public:
  Value();
  Value(uint64_t value);
  Value(int64_t value);
  Value(int value);
  Value(bool value);
  Value(const std::vector<double>& value);
  Value(const std::string& value);
  Value(const char* value);
  Value(QJsonValue value);

  QJsonValue toJson() const;

private:
  QJsonValue m_value;
};

QString ToString(const Value &v);

class Tracer;

class Span
{
public:
  // Mimics jaegertracing/Span.h
  explicit Span(const std::shared_ptr<const Tracer>& tracer = nullptr,
                const std::string& operationName = "");

  ~Span();

  void SetTag(const std::string& key, const Value& value);

  void appendTag(const std::string& key, const Value& value);
  bool hasTag(const std::string &key) const;
  const std::string& operationName() const;
  const std::map<std::string, Value>& tags() const;
  std::uint64_t startTimestamp() const;
  std::uint64_t finishTimestamp() const;

  void Finish();

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

class Config
{
public:
  Config(const std::string& kafkaBrokers = "kafka.int.janelia.org:9092");
  Config(const Config& other);
  ~Config();

  const std::string& kafkaBrokers() const;
  std::uint32_t kafkaPartition() const;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

class Tracer : public std::enable_shared_from_this<Tracer>
{
public:
  // Mimics jaegertracing/Tracer.h
  static std::shared_ptr<Tracer> make(const std::string& serviceName,
                                      const Config& config);

  virtual ~Tracer() = default;

  std::unique_ptr<Span> StartSpan(const std::string& operationName);

  static std::shared_ptr<Tracer> Global() noexcept;

  static std::shared_ptr<Tracer> InitGlobal(
      std::shared_ptr<Tracer> tracer) noexcept;

  // Mimics jaegertracing/Tracer.h
  void reportSpan(const Span& span) const;

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;

  // Mimics jaegertracing/Tracer.h
  Tracer(const std::string& serviceName, const Config& config);
};

}

#endif // NEUOPENTRACING_H
