#include "zflyembodystatus.h"
#if _QT_GUI_USED_
#include "neutube.h"
#endif
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
#if _QT_GUI_USED_
  if (ZString(status).lower() == "roughly traced") {
    return neutube::IsAdminUser();
  }
#endif

  return true;
}

std::string ZFlyEmBodyStatus::GetExpertStatus()
{
  return "Roughly traced";
}
