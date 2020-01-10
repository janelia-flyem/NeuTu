#ifndef ZSTRINGBUILDER_H
#define ZSTRINGBUILDER_H

#include <type_traits>
#include <string>
#include <cstdint>

class ZStringBuilder
{
public:
  ZStringBuilder(const std::string &str);

  operator std::string ();

  ZStringBuilder& append(const std::string &str);

  template<typename T,
           typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
  ZStringBuilder& append(
      const T& v) {
    m_result += std::to_string(v);
    return *this;
  }

  template<typename T,
           typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
  ZStringBuilder& append(T n, int pad);

private:
  std::string m_result;
};

extern template
ZStringBuilder& ZStringBuilder::append<int>(int num, int pad);

extern template
ZStringBuilder& ZStringBuilder::append<int64_t>(int64_t num, int pad);

extern template
ZStringBuilder& ZStringBuilder::append<uint64_t>(uint64_t num, int pad);

#endif // ZSTRINGBUILDER_H
