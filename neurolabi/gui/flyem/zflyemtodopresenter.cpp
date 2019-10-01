#include "zflyemtodopresenter.h"

#include "flyem/zflyemtodoitem.h"

ZFlyEmTodoPresenter::ZFlyEmTodoPresenter()
{
  // keep this in sync with the TodoTableColumns enum in .h!
  m_fieldList << " Body ID " << "Action" << "Comment" << "Priority"
              << "  Z   " << "  X  " << "  Y  " << "  User  ";
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
      case TODO_BODYID_COLUMN:
        return QString("%1").arg(item.getBodyId());
      case TODO_ACTION_COLUMN:
        return QString::fromStdString(item.getActionName());
      case TODO_COMMENT_COLUMN:
        return QString::fromStdString(item.getComment());
      case TODO_PRIORITY_COLUMN:
        if (item.getPriority() > 0) {
          return QString::number(item.getPriority()) + "-"
              + item.getPriorityName().c_str();
        } else {
          return item.getPriorityName().c_str();
        }
      case TODO_Z_COLUMN:
        return item.getPosition().getZ();
      case TODO_X_COLUMN:
        return item.getPosition().getX();
      case TODO_Y_COLUMN:
        return item.getPosition().getY();
      case TODO_USERNAME_COLUMN:
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
