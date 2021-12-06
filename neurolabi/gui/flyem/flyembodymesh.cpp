#include "flyembodymesh.h"

FlyEmBodyMesh::FlyEmBodyMesh(ZMesh *mesh, const FlyEmBodyConfig &config)
  : m_mesh{mesh}, m_config(config)
{
}

FlyEmBodyMesh::FlyEmBodyMesh(
    std::unique_ptr<ZMesh> &&mesh, const FlyEmBodyConfig &config):
  m_config(config)
{
  m_mesh = std::move(mesh);
}

bool FlyEmBodyMesh::hasData() const
{
  return bool(m_mesh);
}

ZMesh* FlyEmBodyMesh::getData() const
{
  return m_mesh.get();
}


ZMesh* FlyEmBodyMesh::releaseData()
{
  return m_mesh.release();
}

FlyEmBodyConfig FlyEmBodyMesh::getBodyConfig() const
{
  return m_config;
}

void FlyEmBodyMesh::setDsLevel(int level)
{
  m_config.setDsLevel(level);
}

void FlyEmBodyMesh::disableNextDsLevel()
{
  m_config.disableNextDsLevel();
}

