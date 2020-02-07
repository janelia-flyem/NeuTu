#include "zroiprovider.h"

#include "zabstractroifactory.h"
#include "zroimesh.h"

ZRoiProvider::ZRoiProvider(QObject *parent) : QObject(parent)
{
  m_colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
}


size_t ZRoiProvider::getRoiCount() const
{
  return m_roiList.size();
}

std::string ZRoiProvider::getRoiName(size_t index) const
{
  if (index < m_roiList.size()) {
    return m_roiList[index]->getName();
  }

  return "";
}

ZRoiMesh* ZRoiProvider::getRoiMesh(size_t index) const
{
  if (index < m_roiList.size()) {
    return m_roiList[index].get();
  }

  return nullptr;
}

std::string ZRoiProvider::getRoiStatus(size_t index) const
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    switch (mesh->getStatus()) {
    case ZRoiMesh::EStatus::READY:
      return "Ready";
    case ZRoiMesh::EStatus::EMPTY:
      return "Empty";
    case ZRoiMesh::EStatus::PENDING:
      return "Pending";
    }
  }

  return "N/A";
}

bool ZRoiProvider::isVisible(size_t index) const
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    return mesh->isVisible();
  }

  return false;
}

QColor ZRoiProvider::getRoiColor(size_t index) const
{
  return m_colorScheme.getColor(int(index));
}
