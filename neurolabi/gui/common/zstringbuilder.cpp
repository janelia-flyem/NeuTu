#include "zstringbuilder.h"

#include <sstream>

ZStringBuilder::ZStringBuilder(const std::string &str)
{
  m_result = str;
}

ZStringBuilder::operator std::string()
{
  return m_result;
}

ZStringBuilder &ZStringBuilder::append(const std::string &str)
{
  m_result += str;

  return *this;
}

template<typename T,
         typename std::enable_if<std::is_integral<T>::value, T>::type>
ZStringBuilder& ZStringBuilder::append(T n, int pad)
{
  static_assert(
        std::is_integral<T>::value, "The first argument must be integral.");
  std::ostringstream stream;
  stream.fill('0');
  stream.width(pad);
  stream << n;

  m_result += stream.str();

  return *this;
}

template
ZStringBuilder& ZStringBuilder::append<int>(int num, int pad);

template
ZStringBuilder& ZStringBuilder::append<uint64_t>(uint64_t num, int pad);

template
ZStringBuilder& ZStringBuilder::append<int64_t>(int64_t num, int pad);
