#include "zflyemneuronfeatureset.h"
#include "zflyemneuronfeaturefactory.h"

ZFlyEmNeuronFeatureSet::ZFlyEmNeuronFeatureSet()
{
}

ZFlyEmNeuronFeatureSet& ZFlyEmNeuronFeatureSet::operator<< (
    const ZFlyEmNeuronFeature& v)
{
  push_back(v);
  return *this;
}

ZFlyEmNeuronFeatureSet& ZFlyEmNeuronFeatureSet::operator<< (
    ZFlyEmNeuronFeature::EFeatureId id)
{
  push_back(ZFlyEmNeuronFeatureFactory::makeFeature(id));
  return *this;
}

ZFlyEmNeuronFeatureSet& ZFlyEmNeuronFeatureSet::operator<< (
    const std::string &name)
{
  push_back(ZFlyEmNeuronFeatureFactory::makeFeature(name));
  return *this;
}
