#ifndef ZEXCEPTION_H
#define ZEXCEPTION_H

#include <QString>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

// all exceptions are defined here
// always catch ZException unless you can make sure of the possible exception type

class ZException
{
public:
  explicit inline ZException(const char* what)
    : m_what(what)
  {}

  explicit inline ZException(const std::string& what)
  {
    m_what = QString::fromUtf8(what.c_str());
  }

  explicit inline ZException(const QString& what)
    : m_what(what)
  {}

  ZException(ZException&&) = default;

  ZException& operator=(ZException&&) = default;

  ZException(const ZException&) = default;

  ZException& operator=(const ZException&) = default;

  virtual ~ZException() noexcept;

  inline const QString& what() const noexcept
  { return m_what; }

protected:
  QString m_what;
};

// io exception, generated while reading or writing
class ZIOException : public ZException
{
public:
  explicit inline ZIOException(const char* what)
    : ZException(what)
  {
    if (errno != 0) {
      m_what = m_what + QString(" <errno: %1>").arg(std::strerror(errno));
      errno = 0;
    }
  }

  explicit inline ZIOException(const std::string& what)
    : ZException(what)
  {
    if (errno != 0) {
      m_what = m_what + QString(" <errno: %1>").arg(std::strerror(errno));
      errno = 0;
    }
  }

  explicit inline ZIOException(const QString& what)
    : ZException(what)
  {
    if (errno != 0) {
      m_what = m_what + QString(" <errno: %1>").arg(std::strerror(errno));
      errno = 0;
    }
  }
};

class ZProcessAbortException : public ZException
{
public:
  using ZException::ZException;
};

class ZGLException : public ZException
{
public:
  using ZException::ZException;
};

#endif // ZEXCEPTION_H
