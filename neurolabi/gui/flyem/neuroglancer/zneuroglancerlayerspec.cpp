#include "zneuroglancerlayerspec.h"

#include "zjsonobject.h"

ZNeuroglancerLayerSpec::ZNeuroglancerLayerSpec()
{

}

ZJsonObject ZNeuroglancerLayerSpec::toJsonObject() const
{
  ZJsonObject obj;
  obj.setNonEmptyEntry("source", m_source);
  obj.setNonEmptyEntry("type", m_type);
  obj.setNonEmptyEntry("name", m_name);

  return obj;
}

/*
std::string ZNeuroglancerLayerSpec::toPathString() const
{
  return toJsonObject().dumpString(0);
}
*/
