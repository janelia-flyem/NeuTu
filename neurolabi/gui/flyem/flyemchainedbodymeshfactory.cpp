#include "flyemchainedbodymeshfactory.h"

FlyEmChainedBodyMeshFactory::FlyEmChainedBodyMeshFactory()
{

}

FlyEmChainedBodyMeshFactory::~FlyEmChainedBodyMeshFactory()
{
}

void FlyEmChainedBodyMeshFactory::append(FlyEmBodyMeshFactory *factory)
{
  m_factoryList.push_back(std::shared_ptr<FlyEmBodyMeshFactory>(factory));
}

FlyEmBodyMesh FlyEmChainedBodyMeshFactory::make_(const FlyEmBodyConfig &config)
{
  for (auto factory : m_factoryList) {
    FlyEmBodyMesh bodyMesh = factory->make(config);
    if (bodyMesh.hasData()) {
      return bodyMesh;
    }
  }

  return {nullptr, config};
}
