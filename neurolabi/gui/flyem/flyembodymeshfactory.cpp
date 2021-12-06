#include "flyembodymeshfactory.h"

FlyEmBodyMeshFactory::FlyEmBodyMeshFactory()
{

}

FlyEmBodyMeshFactory::~FlyEmBodyMeshFactory()
{

}

FlyEmBodyMesh FlyEmBodyMeshFactory::make(const FlyEmBodyConfig &config)
{
  FlyEmBodyMesh bodyMesh = _make(config);
  if (_postProcess) {
    _postProcess(bodyMesh);
  }

  return bodyMesh;
}

void FlyEmBodyMeshFactory::setPostProcess(std::function<void (FlyEmBodyMesh &)> f)
{
  _postProcess = f;
}
