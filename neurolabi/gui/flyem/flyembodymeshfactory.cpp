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

void FlyEmBodyMeshFactory::setResRange(int minResLevel, int maxResLevel)
{
  m_minResLevel = minResLevel;
  m_maxResLevel = maxResLevel;
}

void FlyEmBodyMeshFactory::setMinResLevel(int level)
{
  m_minResLevel = level;
}

void FlyEmBodyMeshFactory::setMaxResLevel(int level)
{
  m_maxResLevel = level;
}

void FlyEmBodyMeshFactory::setPostProcess(std::function<void (FlyEmBodyMesh &)> f)
{
  _postProcess = f;
}
