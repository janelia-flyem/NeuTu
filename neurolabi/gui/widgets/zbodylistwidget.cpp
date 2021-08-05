#include "zbodylistwidget.h"
#include <QDebug>

#include "logging/zqslog.h"
#include "ui_zbodylistwidget.h"
#include "flyem/zflyembodylistmodel.h"

ZBodyListWidget::ZBodyListWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZBodyListWidget)
{
  ui->setupUi(this);
  ZFlyEmBodyListModel *model = new ZFlyEmBodyListModel(this);
  ui->listView->setModel(model);
  ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  qDebug() << ui->listView->selectionModel();

  connect(ui->listView->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          ui->listView, SLOT(processSelectionChange()));
  connect(ui->listView, SIGNAL(bodyItemSelectionChanged(QSet<uint64_t>)),
          this, SLOT(processBodySelectionChange(QSet<uint64_t>)));
  connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addString()));
  connect(ui->deletePushButton, SIGNAL(clicked()),
          this, SLOT(removeSelectedString()));
  connect(model, SIGNAL(bodyAdded(uint64_t)),
          this, SIGNAL(bodyAdded(uint64_t)));
  connect(model, SIGNAL(bodyRemoved(uint64_t)),
          this, SIGNAL(bodyRemoved(uint64_t)));

  ui->bodyListEdit->setConfirmTitle("☟");
  connect(ui->bodyListEdit, &FlyEmBodySupplyWidget::confirmed,
          this, &ZBodyListWidget::addBodies);
}

ZBodyListWidget::~ZBodyListWidget()
{
  delete ui;
}


ZFlyEmBodyListModel* ZBodyListWidget::getModel() const
{
  return getView()->getModel();
}

ZFlyEmBodyListView* ZBodyListWidget::getView() const
{
  return ui->listView;
}

void ZBodyListWidget::addString()
{
  ZFlyEmBodyListModel *model = getModel();
  int row = model->rowCount();
  if (row > 0) {
    if (model->getBodyId(row - 1) == 0) {
      row = row - 1;
    }
  }

  if (row == model->rowCount()){
    model->insertRow(row);
  }

  QModelIndex index = model->index(row);
  ui->listView->setCurrentIndex(index);
  ui->listView->edit(index);
  ui->listView->setIndexSelectionSliently(index, false);
}

void ZBodyListWidget::removeSelectedString()
{
  QItemSelectionModel *model = ui->listView->selectionModel();

  QModelIndexList indexes = model->selectedIndexes();
  QList<int> rowList;
  foreach (const QModelIndex &index, indexes) {
    rowList.append(index.row());
  }
  getModel()->removeRowList(rowList);
}

void ZBodyListWidget::removeBody(uint64_t bodyId)
{
  ZFlyEmBodyListModel *model = getModel();
  model->removeBody(bodyId);
}

void ZBodyListWidget::addBody(uint64_t bodyId)
{
  ZFlyEmBodyListModel *model = getModel();
  model->addBody(bodyId);
}

void ZBodyListWidget::addBodies(QList<uint64_t> bodyList)
{
  ZFlyEmBodyListModel *model = getModel();
  foreach (uint64_t bodyId, bodyList) {
    model->addBody(bodyId);
  }
}

void ZBodyListWidget::processBodySelectionChange(
    const QSet<uint64_t> &selectedSet)
{
  emit bodyItemSelectionChanged(selectedSet);
}

void ZBodyListWidget::selectBodyItem(uint64_t bodyId)
{
  ui->listView->setBodySelection(bodyId, true);
}

void ZBodyListWidget::deselectBodyItem(uint64_t bodyId)
{
  ui->listView->setBodySelection(bodyId, false);
}

void ZBodyListWidget::selectBodyItemSliently(uint64_t bodyId)
{
  ui->listView->setBodySelectionSliently(bodyId, true);
}

void ZBodyListWidget::deselectBodyItemSliently(uint64_t bodyId)
{
  ui->listView->setBodySelectionSliently(bodyId, false);
}

void ZBodyListWidget::diagnose()
{
  LDEBUG() << "#Bodies in the list:" << getModel()->getBodySet().size();
  LDEBUG() << "#Selected:" << ui->listView->getSelectedSet().size();
}
