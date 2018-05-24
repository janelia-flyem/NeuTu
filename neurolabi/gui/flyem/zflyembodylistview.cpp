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
  emit bodyItemSelectionChanged(getSelectedSet());
}

QSet<uint64_t> ZFlyEmBodyListView::getSelectedSet() const
{
  QSet<uint64_t> selectedSet;

  QItemSelectionModel *selModel = selectionModel();

  ZFlyEmBodyListModel *model = getModel();

  QModelIndexList indexes = selModel->selectedIndexes();
  foreach (const QModelIndex &index, indexes) {
    uint64_t bodyId = model->getBodyId(index);
    if (bodyId > 0) {
      selectedSet.insert(bodyId);
    }
  }

  return selectedSet;
}

void ZFlyEmBodyListView::setBodySelection(
    uint64_t bodyId, bool selected, bool silent)
{
  QModelIndex index = getModel()->getIndex(bodyId);

  setIndexSelection(index, selected, silent);
}

void ZFlyEmBodyListView::setIndexSelection(
    const QModelIndex &index, bool selected, bool silent)
{
  QItemSelectionModel *selModel = selectionModel();

  if (index.isValid()) {
    if (selected) {
      selModel->select(index, QItemSelectionModel::Select);
    } else {
      selModel->select(index, QItemSelectionModel::Deselect);
    }
  }

  if (!silent) {
    processSelectionChange();
  }
}

void ZFlyEmBodyListView::setIndexSelectionSliently(
    const QModelIndex &index, bool selected)
{
  setIndexSelection(index, selected, true);
}

void ZFlyEmBodyListView::setBodySelection(uint64_t bodyId, bool selected)
{
  setBodySelection(bodyId, selected, false);
}

void ZFlyEmBodyListView::setBodySelectionSliently(
    uint64_t bodyId, bool selected)
{
  setBodySelection(bodyId, selected, true);
}
