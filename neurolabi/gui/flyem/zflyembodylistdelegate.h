#ifndef ZFLYEMBODYLISTDELEGATE_H
#define ZFLYEMBODYLISTDELEGATE_H

#include <QStyledItemDelegate>

class ZFlyEmBodyListDelegate : public QStyledItemDelegate
{
public:
  explicit ZFlyEmBodyListDelegate(QObject *parent = 0);

  QWidget* createEditor(
      QWidget *parent, const QStyleOptionViewItem &option,
      const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;

};

#endif // ZFLYEMBODYLISTDELEGATE_H
