#include "zflyembodystatus.h"
#include "neutube.h"
#include "zstring.h"

ZFlyEmBodyStatus::ZFlyEmBodyStatus(const std::string &status) :
  m_status(status)
{

}

bool ZFlyEmBodyStatus::isAccessible() const
{
  return true;
}

bool ZFlyEmBodyStatus::IsAccessible(const std::string &status)
{
  if (ZString(status).lower() == "prelim roughly traced") {
    return neutube::IsAdminUser();
  }

  return true;
}
