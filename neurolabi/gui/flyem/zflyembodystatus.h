#ifndef ZFLYEMBODYSTATUS_H
#define ZFLYEMBODYSTATUS_H

#include <string>

class ZFlyEmBodyStatus
{
public:
  ZFlyEmBodyStatus(const std::string &status);

  bool isAccessible() const;

  static bool IsAccessible(const std::string &status);

private:
  std::string m_status;
  int m_priority = 999;
  int m_protection = 0; //0: visible to all
};

#endif // ZFLYEMBODYSTATUS_H
