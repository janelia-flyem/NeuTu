#ifndef FLYEMCHAINEDBODYMESHFACTORY_H
#define FLYEMCHAINEDBODYMESHFACTORY_H

#include <vector>
#include <memory>

#include "flyembodymeshfactory.h"

class FlyEmChainedBodyMeshFactory : public FlyEmBodyMeshFactory
{
public:
  FlyEmChainedBodyMeshFactory();
  ~FlyEmChainedBodyMeshFactory();

  void append(FlyEmBodyMeshFactory *factory);

protected:
  FlyEmBodyMesh _make(const FlyEmBodyConfig &config) override;

private:
  std::vector<std::shared_ptr<FlyEmBodyMeshFactory>> m_factoryList;
};

#endif // FLYEMCHAINEDBODYMESHFACTORY_H
