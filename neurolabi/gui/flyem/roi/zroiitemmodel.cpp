#include "zroiitemmodel.h"

#include <QBrush>
#include <QFont>

#include "zroiprovider.h"

ZRoiItemModel::ZRoiItemModel(QObject *parent) : QAbstractTableModel(parent)
{

}

void ZRoiItemModel::setRoiProvider(
    const std::shared_ptr<ZRoiProvider> &roiProvider)
{
  if (m_roiProvider) {
    disconnect(m_roiProvider.get(), &ZRoiProvider::roiListUpdated,
               this, &ZRoiItemModel::updateModel);
    disconnect(m_roiProvider.get(), SIGNAL(roiLoaded(const QString&)),
               this, SLOT(updateRoiItem(const QString&)));
    disconnect(this, SIGNAL(roiRequested(int)),
               m_roiProvider.get(), SLOT(requestRoi(int)));
  }

  m_roiProvider = roiProvider;

  if (m_roiProvider) {
    m_visibleList.resize(m_roiProvider->getRoiCount(), false);
    m_colorList.resize(m_roiProvider->getRoiCount());
    for (size_t i = 0; i < m_colorList.size(); ++i) {
      m_visibleList[i] = m_roiProvider->isVisible(i);
      m_colorList[i] = m_roiProvider->getDefaultRoiColor(i);
    }

    connect(m_roiProvider.get(), &ZRoiProvider::roiListUpdated,
               this, &ZRoiItemModel::updateModel);
    connect(m_roiProvider.get(), SIGNAL(roiLoaded(const QString&)),
            this, SLOT(updateRoiItem(const QString&)));
    connect(this, SIGNAL(roiRequested(int)),
               m_roiProvider.get(), SLOT(requestRoi(int)));
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
  return COLUMN_COUNT;
}

Qt::ItemFlags ZRoiItemModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags =  QAbstractTableModel::flags(index);

  if (index.isValid()) {
    if (index.column() == COLUMN_NAME) {
      flags |= Qt::ItemIsUserCheckable;
    }/* else if (index.column() == COLUMN_COLOR) {
      flags &= ~Qt::ItemIsSelectable;
    }*/
  }

  return flags;
}

QVariant ZRoiItemModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid()) {
    size_t row = size_t(index.row());

    switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
      case COLUMN_NAME:
        return m_roiProvider->getRoiName(row).c_str();
        //    case COLUMN_STATUS:
        //      return m_roiProvider->getRoiStatus(row).c_str();
      case COLUMN_COLOR:
        return QString("\u25fc") + " " + m_colorList[row].name();
      default:
        return "";
      }
    case Qt::CheckStateRole:
      if (index.column() == COLUMN_NAME) {
        return m_visibleList[row] ? Qt::Checked : Qt::Unchecked;
      }
      break;
    case Qt::ForegroundRole:
      if (index.column() == COLUMN_COLOR) {
        return m_colorList[row];
      } else if (index.column() == COLUMN_NAME) {
        if (m_roiProvider->getRoiStatus(row) == ZRoiProvider::ROI_STATUS_PENDING) {
          return QColor(128, 128, 128);
        }
      }
      break;
    case Qt::FontRole:
      if (index.column() == COLUMN_NAME) {
        if (m_roiProvider->getRoiStatus(row) == ZRoiProvider::ROI_STATUS_EMPTY) {
          QFont font;
          font.setStrikeOut(true);
          return font;
        }
      }
      break;
    case Qt::ToolTipRole:
      if (index.column() == COLUMN_NAME) {
        std::string status = m_roiProvider->getRoiStatus(row);
        return QString(status.c_str());
      }
      break;
    default:
      break;
    }
  }

  return QVariant();
}

bool ZRoiItemModel::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
  bool succ = false;
  if (index.isValid()) {
    if (index.column() == COLUMN_NAME) {
      if (role == Qt::CheckStateRole) {
        bool checked = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
//        m_roiProvider->setVisible(size_t(index.row()), checked);
        if (m_visibleList[size_t(index.row())] != checked) {
          m_visibleList[size_t(index.row())] = checked;
          if (checked) {
            if (m_roiProvider->isRoiMeshReady(size_t(index.row()))) {
              updateRoi(index.row());
            } else {
              emit roiRequested(index.row());
            }
          } else {
            updateRoi(index.row());
          }
          succ = true;
        }
      }
    } else if (index.column() == COLUMN_COLOR) {
      if (role == Qt::ForegroundRole) {
//        m_roiProvider->setColor(
//              size_t(index.row()), qvariant_cast<QBrush>(value).color());
        m_colorList[size_t(index.row())] =
            qvariant_cast<QBrush>(value).color();

        updateRoi(index.row());
        succ = true;
      }
    }
  }

  if (succ) {
    emit dataChanged(index, index);
  }

  return succ;
}

QVariant ZRoiItemModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case COLUMN_NAME:
      return "Name";
    case COLUMN_COLOR:
      return "Color";
//    case COLUMN_STATUS:
//      return "Status";
    default:
      return "";
    }
  }

  return QVariant();
}

void ZRoiItemModel::updateModel()
{
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

bool ZRoiItemModel::isChecked(int row) const
{
  if (row >= 0 && row < m_visibleList.size()) {
    return m_visibleList[size_t(row)];
  }

  return false;
}

QColor ZRoiItemModel::getColor(int row) const
{
  if (row >= 0 && row < m_colorList.size()) {
    return m_colorList[size_t(row)];
  }

  return Qt::black;
}

void ZRoiItemModel::setColor(int row, const QColor &color)
{
  if (m_colorList[row] != color) {
    m_colorList[row] = color;
    updateRoiItem(row, COLUMN_COLOR);
  }
}

void ZRoiItemModel::toggleChecked(const std::vector<int> &rowList)
{
  QList<QString> nameList;
  QList<bool> checkedList;
  QList<QColor> colorList;
  for (int row : rowList) {
    m_visibleList[row] = !m_visibleList[row];
    emit dataChanged(index(row, COLUMN_NAME), index(row, COLUMN_NAME));
    if (requestRoiNecessarily(row, COLUMN_NAME) == false) {
      nameList.append(getName(row));
      checkedList.append(isChecked(row));
      colorList.append(getColor(row));
    }
  }

  if (!nameList.isEmpty()) {
    emit roiListUpdated(nameList, checkedList, colorList);
  }
}

void ZRoiItemModel::setChecked(const std::vector<int> &rowList, bool checked)
{
  QList<QString> nameList;
  QList<bool> checkedList;
  QList<QColor> colorList;
  for (int row : rowList) {
    if (m_visibleList[row] != checked) {
      m_visibleList[row] = checked;
      emit dataChanged(index(row, COLUMN_NAME), index(row, COLUMN_NAME));
      if (requestRoiNecessarily(row, COLUMN_NAME) == false) {
        nameList.append(getName(row));
        checkedList.append(isChecked(row));
        colorList.append(getColor(row));
      }
    }
  }

  if (!nameList.isEmpty()) {
    emit roiListUpdated(nameList, checkedList, colorList);
  }
}

void ZRoiItemModel::setChecked(int row, bool checked)
{
  if (m_visibleList[row] != checked) {
    m_visibleList[row] = checked;
    updateRoiItem(row, COLUMN_NAME);
  }
}

bool ZRoiItemModel::requestRoiNecessarily(int row, int column)
{
  if (column == COLUMN_NAME) {
    if (m_roiProvider->getRoiStatus(row) == ZRoiProvider::ROI_STATUS_PENDING) {
      emit roiRequested(row);
      return true;
    }
  }

  return false;
}

void ZRoiItemModel::updateRoiItem(int row)
{
  emit dataChanged(index(row, 0), index(row, columnCount() - 1));
  if (requestRoiNecessarily(row, COLUMN_NAME) == false) {
    updateRoi(row);
  }
}

void ZRoiItemModel::updateRoiItem(int row, int column)
{
  emit dataChanged(index(row, column), index(row, column));
  if (requestRoiNecessarily(row, column) == false) {
    updateRoi(row);
  }
}

QString ZRoiItemModel::getName(int row) const
{
  return data(QAbstractItemModel::createIndex(
                row, COLUMN_NAME), Qt::DisplayRole).toString();
}

void ZRoiItemModel::updateRoiItem(const QString &name)
{
  if (!name.isEmpty()) {
    for (int i = 0; i < rowCount(); ++i) {
      if (getName(i) == name) {
        updateRoiItem(i);
        break;
      }
    }
  }
}

void ZRoiItemModel::updateRoi(int row)
{
  const QString &name = getName(row);
  if (!name.isEmpty()) {
    bool isVisible = isChecked(row);
    QColor color = getColor(row);
    emit roiUpdated(name, isVisible, color);
  }
}

