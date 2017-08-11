#include "zflyembodylistdelegate.h"

#include "zflyembodyideditor.h"

ZFlyEmBodyListDelegate::ZFlyEmBodyListDelegate(QObject *parent) :
  QStyledItemDelegate(parent)
{

}

QWidget* ZFlyEmBodyListDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &/*option*/,
    const QModelIndex &/*index*/) const
{
  ZFlyEmBodyIdEditor *widget = new ZFlyEmBodyIdEditor(parent);

  return widget;
}

void ZFlyEmBodyListDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  ZFlyEmBodyIdEditor *lineEdit = qobject_cast<ZFlyEmBodyIdEditor*>(editor);

  model->setData(index, lineEdit->text(), Qt::EditRole);
}
