#ifndef ZFLYEMNEURONFEATUREFACTORY_H
#define ZFLYEMNEURONFEATUREFACTORY_H

#include <map>
#include <string>

#include "zflyemneuronfeature.h"

class ZFlyEmNeuronFeatureFactory
{
public:
  ZFlyEmNeuronFeatureFactory();

  static ZFlyEmNeuronFeature makeFeature(ZFlyEmNeuronFeature::EFeatureId id);
  static ZFlyEmNeuronFeature makeFeature(const std::string &name);

private:
  static std::pair<ZFlyEmNeuronFeature::EFeatureId, std::string> makeFeature(
      ZFlyEmNeuronFeature::EFeatureId key, const std::string &value);
  static std::pair<std::string, ZFlyEmNeuronFeature::EFeatureId> makeFeature2(
      ZFlyEmNeuronFeature::EFeatureId key, const std::string &value);

private:
  static std::map<ZFlyEmNeuronFeature::EFeatureId, std::string>
  m_featureIdNameMap;
  static std::map<std::string, ZFlyEmNeuronFeature::EFeatureId>
  m_featureNameIdMap;
};

#endif // ZFLYEMNEURONFEATUREFACTORY_H
