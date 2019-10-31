#ifndef ZNEUROGLANCERPATH_H
#define ZNEUROGLANCERPATH_H

#include <vector>
#include <memory>
#include<string>

#include "zneuroglancernavigation.h"

class ZNeuroglancerLayerSpec;
class ZJsonObject;

class ZNeuroglancerPath
{
public:
  ZNeuroglancerPath();

  std::string getPath() const;

  void addLayer(
      const std::shared_ptr<ZNeuroglancerLayerSpec> &layer, bool selected = false);
  void setNavigation(const ZNeuroglancerNavigation &nav);

private:
//  QString getLayerString() const;
//  void appendSetting(std::string &currentPath, const std::string &setting);

public:
  static const char* DATA_START_TAG;

private:
  std::vector<std::shared_ptr<ZNeuroglancerLayerSpec>> m_layerList;
  std::shared_ptr<ZNeuroglancerLayerSpec> m_selectedLayer;
  ZNeuroglancerNavigation m_navigation;
  double m_perspectiveZoom = 128;
  std::string m_layout = "4panel";
};

#endif // ZNEUROGLANCERPATH_H
