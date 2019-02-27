#include "flyemdataconfig.h"

#include "zjsonobject.h"

const char* FlyEmDataConfig::KEY_BODY_STATUS = "body_status";
const char* FlyEmDataConfig::KEY_CONTRAST = "contrast";

FlyEmDataConfig::FlyEmDataConfig()
{

}

void FlyEmDataConfig::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey(KEY_CONTRAST)) {
    ZJsonObject subobj(obj.value(KEY_CONTRAST));
    loadContrastProtocol(subobj);
  }

  if (obj.hasKey(KEY_BODY_STATUS)) {
    ZJsonObject subobj(obj.value(KEY_BODY_STATUS));
    loadBodyStatusProtocol(subobj);
  }
}


void FlyEmDataConfig::loadContrastProtocol(const ZJsonObject &obj)
{
  m_contrastProtocol.load(obj);
}

void FlyEmDataConfig::loadBodyStatusProtocol(const ZJsonObject &obj)
{
  m_bodyStatusProtocol.loadJsonObject(obj);
}
