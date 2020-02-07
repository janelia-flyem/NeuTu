#include "zroimesh.h"

#include "zmesh.h"
#include "zstackobjectsourcefactory.h"

ZRoiMesh::ZRoiMesh()
{

}

ZRoiMesh::~ZRoiMesh()
{
}

void ZRoiMesh::setMesh(ZMesh *mesh)
{
  m_mesh = std::shared_ptr<ZMesh>(mesh);
  updateMeshSource();
}

void ZRoiMesh::setMesh(const std::shared_ptr<ZMesh> &mesh)
{
  m_mesh = mesh;
  updateMeshSource();
}

void ZRoiMesh::updateMeshSource()
{
  if (m_mesh) {
    m_mesh->setSource(getSourceName());
  }
}

void ZRoiMesh::setMesh(
    const std::string &name, const std::shared_ptr<ZMesh> &mesh)
{
  m_name = name;
  setMesh(mesh);
}

void ZRoiMesh::setMesh(const std::string &name, ZMesh *mesh)
{
  m_name = name;
  setMesh(mesh);
}

ZMesh* ZRoiMesh::getMesh() const
{
  return m_mesh.get();
}

ZMesh* ZRoiMesh::makeMesh() const
{
  ZMesh *mesh = nullptr;
  if (isLoaded()) {
    mesh = new ZMesh(*m_mesh);
  }

  return mesh;
}

bool ZRoiMesh::isLoaded() const
{
  return bool(m_mesh);
}

bool ZRoiMesh::isVisible() const
{
  ZMesh *mesh = getMesh();
  if (mesh) {
    return mesh->isVisible();
  }

  return false;
}

void ZRoiMesh::setName(const std::string &name)
{
  m_name = name;
  updateMeshSource();
}

std::string ZRoiMesh::getName() const
{
  return m_name;
}

std::string ZRoiMesh::getSourceName() const
{
  return m_name.empty()
      ? "" : ZStackObjectSourceFactory::MakeFlyEmRoiSource(m_name);
}

ZRoiMesh::EStatus ZRoiMesh::getStatus() const
{
  if (m_mesh) {
    if (m_mesh->empty()) {
      return EStatus::EMPTY;
    } else {
      return EStatus::READY;
    }
  }

  return EStatus::PENDING;
}
