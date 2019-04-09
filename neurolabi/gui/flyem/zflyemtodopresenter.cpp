#include "zflyemtodopresenter.h"

#include "flyem/zflyemtodoitem.h"

ZFlyEmTodoPresenter::ZFlyEmTodoPresenter()
{
  m_fieldList << " Body ID " << "Action" << "Comment" << "  Z   " << "  X  " << "  Y  " << "  User  ";
}

void ZFlyEmTodoPresenter::setVisibleTest(
    std::function<bool (const ZFlyEmToDoItem &)> f)
{
  m_visible = f;
}

QVariant ZFlyEmTodoPresenter::data(
    const ZFlyEmToDoItem &item, int index, int role) const
{
  if (m_visible(item)) {
    switch(role) {
    case Qt::DisplayRole:
      switch (index) {
      case 0:
        return QString("%1").arg(item.getBodyId());
      case 1:
        return QString::fromStdString(item.getActionName());
      case 2:
        return QString::fromStdString(item.getComment());
      case 3:
        return item.getPosition().getZ();
      case 4:
        return item.getPosition().getX();
      case 5:
        return item.getPosition().getY();
      case 6:
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
  }

  return QVariant();
}
