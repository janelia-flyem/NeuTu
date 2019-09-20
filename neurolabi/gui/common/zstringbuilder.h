#ifndef ZSTRINGBUILDER_H
#define ZSTRINGBUILDER_H

#include <string>

class ZStringBuilder
{
public:
  ZStringBuilder(const std::string &str);

  operator std::string ();

  ZStringBuilder& append(const std::string &str);


private:
  std::string m_result;
};

#endif // ZSTRINGBUILDER_H
