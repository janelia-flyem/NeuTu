#ifndef FLYEMFUNCTIONBODYMESHFACTORY_H
#define FLYEMFUNCTIONBODYMESHFACTORY_H

#include "flyembodymeshfactory.h"

class FlyEmFunctionBodyMeshFactory : public FlyEmBodyMeshFactory
{
public:
  FlyEmFunctionBodyMeshFactory();
  FlyEmFunctionBodyMeshFactory(
      std::function<FlyEmBodyMesh(const FlyEmBodyConfig&)> getMesh);

protected:
  FlyEmBodyMesh make_(const FlyEmBodyConfig &config) override;

private:
  std::function<FlyEmBodyMesh(const FlyEmBodyConfig&)> m_getMesh;
};

#endif // FLYEMFUNCTIONBODYMESHFACTORY_H
