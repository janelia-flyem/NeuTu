#include "zdvidannotation.h"

#include "zjsonobject.h"

ZDvidAnnotation::ZDvidAnnotation()
{
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, const std::string &value)
{
  ZJsonObject propJson = json.value("Prop");
  propJson.setEntry(key, value);
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, bool value)
{
  ZJsonObject propJson = json.value("Prop");
  if (value == true) {
    propJson.setEntry(key, "1");
  } else {
    propJson.setEntry(key, "0");
  }
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}
