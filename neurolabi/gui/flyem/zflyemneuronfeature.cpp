#include "zflyemneuronfeature.h"

#define ZFLYEMNEURONFEATURE_INIT(id, name, value) \
  m_id(id), m_name(name), m_value(value)

ZFlyEmNeuronFeature::ZFlyEmNeuronFeature() :
    ZFLYEMNEURONFEATURE_INIT(UNDEFINED, "", 0.0)
{
}

ZFlyEmNeuronFeature::ZFlyEmNeuronFeature(
    EFeatureId id, const std::string &name) :
  ZFLYEMNEURONFEATURE_INIT(id, name, 0.0)
{
}
