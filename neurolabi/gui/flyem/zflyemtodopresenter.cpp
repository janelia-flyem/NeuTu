#include "zflyemtodopresenter.h"

ZFlyEmTodoPresenter::ZFlyEmTodoPresenter()
{
  m_fieldList << " Body ID " << "  Z   " << "  X  " << "  Y  " << "  User  ";
}

QVariant ZFlyEmTodoPresenter::data(
    const ZFlyEmToDoItem &item, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return QString("%1").arg(item.getBodyId());
    case 1:
      return item.getPosition().getZ();
    case 2:
      return item.getPosition().getX();
    case 3:
      return item.getPosition().getY();
    case 4:
      return item.getUserName().c_str();
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to locate");
    default:
      break;
    }
    break;
  case Qt::ForegroundRole:
    if (item.isChecked()) {
      return QColor(0, 128, 0);
    }
    break;
  default:
    break;
  }

  return QVariant();
}
