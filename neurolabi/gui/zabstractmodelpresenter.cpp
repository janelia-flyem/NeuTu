#include "zabstractmodelpresenter.h"

ZAbstractModelPresenter::ZAbstractModelPresenter(QObject *parent) :
  QObject(parent)
{
}

ZAbstractModelPresenter::~ZAbstractModelPresenter()
{

}

QVariant ZAbstractModelPresenter::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      return m_fieldList[section];
    } else if (orientation == Qt::Vertical) {
      return section + 1;
    }
  }

  return QVariant();
}

int ZAbstractModelPresenter::columnCount() const
{
  return m_fieldList.size();
}

const QString& ZAbstractModelPresenter::getColumnName(int index) const
{
  if (index < 0 || index >= m_fieldList.size()) {
    return m_emptyField;
  }

  return m_fieldList[index];
}
