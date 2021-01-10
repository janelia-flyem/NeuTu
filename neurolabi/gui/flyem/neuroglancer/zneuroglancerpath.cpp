#include "zneuroglancerpath.h"

#include "zjsonobject.h"
#include "zjsonarray.h"

#include "zneuroglancerlayerspec.h"

const char* ZNeuroglancerPath::DATA_START_TAG = "!";

ZNeuroglancerPath::ZNeuroglancerPath()
{

}

/*
void ZNeuroglancerPath::appendSetting(
    std::string &currentPath, const std::string &setting)
{
  if (!setting.empty()) {
    if (currentPath.empty()) {
      currentPath += str::string("/#") + ZNeuroglancerPath::DATA_START_TAG + setting;
    } else {
      currentPath += "," + setting;
    }
  }
}
*/

/*
QString ZNeuroglancerPath::getLayerString() const
{
  ZJsonObject rootObj;
  ZJsonArray layerJson;
  for (auto layer : m_layerList) {
    ZJsonObject layerObj = layer->toJsonObject();
    if (!layerObj.isEmpty()) {
      layerJson.append(layerObj);
    }
  }
  if (!layerJson.isEmpty()) {
    rootObj.setEntry("layers", layerJson);
  }

  return QString::fromStdString(rootObj.dumpString(0));
}
*/

void ZNeuroglancerPath::setNavigation(const ZNeuroglancerNavigation &nav)
{
  m_navigation = nav;
}

void ZNeuroglancerPath::addLayer(
    const std::shared_ptr<ZNeuroglancerLayerSpec> &layer, bool selected)
{
  if (layer) {
    m_layerList.push_back(layer);
    if (selected) {
      m_selectedLayer = layer;
    }
  }
}

ZJsonObject ZNeuroglancerPath::toJsonObject() const
{
  ZJsonObject rootObj;
  ZJsonArray layerJson;
  for (auto layer : m_layerList) {
    ZJsonObject layerObj = layer->toJsonObject();
    if (!layerObj.isEmpty()) {
      layerJson.append(layerObj);
    }
  }
  if (!layerJson.isEmpty()) {
    rootObj.setEntry("layers", layerJson);
  }

//  ZJsonObject navObj = m_navigation.toJsonObject();
//  rootObj.setEntry("navigation", navObj);
  m_navigation.configureJson(rootObj);
  if (m_selectedLayer) {
    ZJsonObject obj;
    obj.setEntry("layer", m_selectedLayer->getName());
    obj.setEntry("visible", true);
    rootObj.setEntry("selectedLayer", obj);
  }
  rootObj.setNonEmptyEntry("layout", m_layout);
  if (m_perspectiveScale > 0) {
    rootObj.setEntry("projectionScale", m_perspectiveScale);
  }

  return rootObj;
}

std::string ZNeuroglancerPath::getPath() const
{
  return std::string("/#") + DATA_START_TAG + toJsonObject().dumpString(0);
}

