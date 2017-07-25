#include "zflyembodylistview.h"

#include <QDebug>

#include "zflyembodylistmodel.h"
#include "zflyembodylistdelegate.h"

ZFlyEmBodyListView::ZFlyEmBodyListView(QWidget *parent) : QListView(parent)
{
  setItemDelegate(new ZFlyEmBodyListDelegate);
}

ZFlyEmBodyListModel* ZFlyEmBodyListView::getModel() const
{
  return qobject_cast<ZFlyEmBodyListModel*>(model());
}

void ZFlyEmBodyListView::processSelectionChange()
{
  emit bodySelectionChanged(getSelectedSet());
}

QSet<uint64_t> ZFlyEmBodyListView::getSelectedSet() const
{
  QSet<uint64_t> selectedSet;

  QItemSelectionModel *selModel = selectionModel();

  ZFlyEmBodyListModel *model = getModel();

  QModelIndexList indexes = selModel->selectedIndexes();
  foreach (const QModelIndex &index, indexes) {
    selectedSet.insert(model->getBodyId(index));
  }

  return selectedSet;
}
