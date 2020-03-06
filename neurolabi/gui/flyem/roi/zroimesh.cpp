#include "zroimesh.h"

#include "zmesh.h"
#include "zstackobjectsourcefactory.h"

ZRoiMesh::ZRoiMesh()
{
}

ZRoiMesh::ZRoiMesh(const std::string &name)
{
  m_name = name;
}

ZRoiMesh::ZRoiMesh(const std::string &name, const QColor &color)
{
  m_name = name;
  m_color = color;
}

ZRoiMesh::~ZRoiMesh()
{
}

void ZRoiMesh::setMesh(ZMesh *mesh)
{
  m_mesh = std::shared_ptr<ZMesh>(mesh);
  updateMeshProperty();
}

void ZRoiMesh::setMesh(const std::shared_ptr<ZMesh> &mesh)
{
  m_mesh = mesh;
  updateMeshProperty();
}

void ZRoiMesh::updateMeshProperty()
{
  if (m_mesh) {
    m_mesh->addRole(ZStackObjectRole::ROLE_ROI);
    m_mesh->setSource(getSourceName());
    m_mesh->pushObjectColor(m_color);
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
  return m_isVisible;
}

void ZRoiMesh::setVisible(bool on)
{
  ZMesh *mesh = getMesh();
  if (mesh) {
    mesh->setVisible(on);
  }
  m_isVisible = on;
}

void ZRoiMesh::setName(const std::string &name)
{
  m_name = name;
  updateMeshProperty();
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

QColor ZRoiMesh::getColor() const
{
  return m_color;
}

void ZRoiMesh::setColor(const QColor &color)
{
  m_color = color;
  ZMesh *mesh = getMesh();
  if (mesh) {
    mesh->setColor(color);
  }
}

