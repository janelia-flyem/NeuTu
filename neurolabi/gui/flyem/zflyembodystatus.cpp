#include "zflyembodystatus.h"
#if _QT_GUI_USED_
#include "neutube.h"
#endif
#include "zstring.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"

const char *ZFlyEmBodyStatus::KEY_NAME = "name";
const char *ZFlyEmBodyStatus::KEY_PRIORITY = "priority";
const char *ZFlyEmBodyStatus::KEY_PROTECTION = "protection";
const char *ZFlyEmBodyStatus::KEY_EXPERT = "expert";
const char *ZFlyEmBodyStatus::KEY_FINAL = "final";
const char *ZFlyEmBodyStatus::KEY_MERGABLE = "mergable";

ZFlyEmBodyStatus::ZFlyEmBodyStatus(const std::string &status) :
  m_status(status)
{
}

void ZFlyEmBodyStatus::reset()
{
  m_status.clear();
  m_priority = 999;
  m_protection = 0;
  m_isExpertStatus = false;
  m_isFinal = false;
  m_isMergable = true;
}

int ZFlyEmBodyStatus::getPriority() const
{
  return m_priority;
}

void ZFlyEmBodyStatus::setProtectionLevel(int level)
{
  m_protection = level;
}

void ZFlyEmBodyStatus::setPriority(int p)
{
  m_priority = p;
}

void ZFlyEmBodyStatus::setExpert(bool on)
{
  m_isExpertStatus = on;
}

void ZFlyEmBodyStatus::setFinal(bool on)
{
  m_isFinal = on;
}

void ZFlyEmBodyStatus::setMergable(bool on)
{
  m_isMergable = on;
}

void ZFlyEmBodyStatus::loadJsonObject(const ZJsonObject &obj)
{
  reset();

  ZJsonObjectParser parser;
  m_status = parser.getValue(obj, KEY_NAME, "");
  m_priority = parser.getValue(obj, KEY_PRIORITY, 999);
  m_protection = parser.getValue<int>(obj, KEY_PROTECTION, 0);
  m_isExpertStatus = parser.getValue(obj, KEY_EXPERT, false);
  m_isFinal = parser.getValue(obj, KEY_FINAL, false);
  m_isMergable = parser.getValue(obj, KEY_MERGABLE, true);
}

ZJsonObject ZFlyEmBodyStatus::toJsonObject() const
{
   ZJsonObject obj;
   obj.setEntry(KEY_NAME, m_status);
   obj.setEntry(KEY_PRIORITY, m_priority);
   obj.setEntry(KEY_PROTECTION, m_protection);
   obj.setEntry(KEY_EXPERT, m_isExpertStatus);
   obj.setEntry(KEY_FINAL, m_isFinal);
   obj.setEntry(KEY_MERGABLE, m_isMergable);

   return obj;
}

std::string ZFlyEmBodyStatus::getName() const
{
  return m_status;
}

bool ZFlyEmBodyStatus::isAdminAccessible() const
{
  return (m_protection >= 5 && m_protection < 9);
}

bool ZFlyEmBodyStatus::isAccessible() const
{
  if (m_protection >= 9) {
    return false;
  } else if (isAdminAccessible()) {
    return neutu::IsAdminUser();
  }

  return true;
}

bool ZFlyEmBodyStatus::IsAccessible(const std::string &status)
{
#if _QT_GUI_USED_
  if (ZString(status).lower() == "roughly traced") {
    return neutu::IsAdminUser();
  }
#endif

  return true;
}

std::string ZFlyEmBodyStatus::GetExpertStatus()
{
  return "Roughly traced";
}

bool ZFlyEmBodyStatus::isFinal() const
{
  return m_isFinal;
}

bool ZFlyEmBodyStatus::isMergable() const
{
  return m_isMergable;
}

bool ZFlyEmBodyStatus::isExpertStatus() const
{
  return m_isExpertStatus;
}

std::string ZFlyEmBodyStatus::getStatusKey() const
{
  return ZString(m_status).lower();
}

void ZFlyEmBodyStatus::print() const
{
  std::cout << toJsonObject().dumpString(0) << std::endl;
}
