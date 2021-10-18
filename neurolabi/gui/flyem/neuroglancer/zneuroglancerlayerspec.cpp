#include "zneuroglancerlayerspec.h"

#include "zjsonobject.h"

const char* ZNeuroglancerLayerSpec::KEY_NAME = "name";
const char* ZNeuroglancerLayerSpec::KEY_SOURCE = "source";
const char* ZNeuroglancerLayerSpec::KEY_TYPE = "type";
const char* ZNeuroglancerLayerSpec::KEY_SHADER = "shader";

const char* ZNeuroglancerLayerSpec::TYPE_SEGMENTATION = "segmentation";
const char* ZNeuroglancerLayerSpec::TYPE_GRAYSCALE = "image";
const char* ZNeuroglancerLayerSpec::TYPE_ANNOTATION = "annotation";
const char* ZNeuroglancerLayerSpec::TYPE_SKELETON = "skeletons";

ZNeuroglancerLayerSpec::ZNeuroglancerLayerSpec()
{

}

ZJsonObject ZNeuroglancerLayerSpec::toJsonObject() const
{
  ZJsonObject obj;
  if (!m_source.empty()) {
    ZJsonObject sourceObj;
    sourceObj.setEntry("url", m_source);
    obj.setNonEmptyEntry(KEY_SOURCE, sourceObj);
  }
  obj.setNonEmptyEntry(KEY_TYPE, m_type);
  obj.setNonEmptyEntry(KEY_NAME, m_name);
  obj.setNonEmptyEntry(KEY_SHADER, m_shader);

  return obj;
}

/*
std::string ZNeuroglancerLayerSpec::toPathString() const
{
  return toJsonObject().dumpString(0);
}
*/
