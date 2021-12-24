#include "flyemfunctionbodymeshfactory.h"

FlyEmFunctionBodyMeshFactory::FlyEmFunctionBodyMeshFactory()
{

}

FlyEmFunctionBodyMeshFactory::FlyEmFunctionBodyMeshFactory(
    std::function<FlyEmBodyMesh(const FlyEmBodyConfig&)> getMesh)
{
  m_getMesh = getMesh;
}

FlyEmBodyMesh FlyEmFunctionBodyMeshFactory::make_(
    const FlyEmBodyConfig &config)
{
  if (m_getMesh) {
    return m_getMesh(config);
  }

  return {nullptr, config};
}
