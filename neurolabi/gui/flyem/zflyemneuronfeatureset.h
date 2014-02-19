#ifndef ZFLYEMNEURONFEATURESET_H
#define ZFLYEMNEURONFEATURESET_H

#include <vector>
#include "zflyemneuronfeature.h"

class ZFlyEmNeuronFeatureSet : public std::vector<ZFlyEmNeuronFeature>
{
public:
  ZFlyEmNeuronFeatureSet();

  ZFlyEmNeuronFeatureSet& operator<< (const ZFlyEmNeuronFeature& v);
  ZFlyEmNeuronFeatureSet& operator<< (ZFlyEmNeuronFeature::EFeatureId id);
  ZFlyEmNeuronFeatureSet& operator<< (const std::string &name);
};

#endif // ZFLYEMNEURONFEATURESET_H
