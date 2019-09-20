#include "zstringbuilder.h"

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
