#include "zroiitemmodel.h"

#include <QBrush>

#include "zroiprovider.h"

ZRoiItemModel::ZRoiItemModel(QObject *parent) : QAbstractItemModel(parent)
{

}

void ZRoiItemModel::setRoiProvider(
    const std::shared_ptr<ZRoiProvider> &roiProvider)
{
  m_roiProvider = roiProvider;
  m_visibleList.resize(m_roiProvider->getRoiCount(), false);
  m_colorList.resize(m_roiProvider->getRoiCount());
  for (size_t i = 0; i < m_colorList.size(); ++i) {
    m_visibleList[i] = m_roiProvider->isVisible(i);
    m_colorList[i] = m_roiProvider->getRoiColor(i);
  }
}

int ZRoiItemModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return int(m_roiProvider->getRoiCount());
}

int ZRoiItemModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 4;
}

QVariant ZRoiItemModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  size_t row = size_t(index.row());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return "Visible";
    case 1:
      return m_roiProvider->getRoiName(row).c_str();
    case 2:
      return m_roiProvider->getRoiName(row).c_str();
    case 3:
      return "@Color";
    }
  } else if (role == Qt::CheckStateRole) {
    if (index.column() == 0) {

    }
  }

  return "";
}

bool ZRoiItemModel::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid()) {
    if (index.column() == 0) {
      if (role == Qt::CheckStateRole) {
        m_visibleList[size_t(index.row())] =
            static_cast<Qt::CheckState>(value.toInt());
        return true;
      }
    } else if (index.column() == 3) {
      if (role == Qt::ForegroundRole) {
        m_colorList[size_t(index.row())] =
            qvariant_cast<QBrush>(value).color();
        return true;
      }
    }
  }

  return false;
}

void ZRoiItemModel::updateModel()
{
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
