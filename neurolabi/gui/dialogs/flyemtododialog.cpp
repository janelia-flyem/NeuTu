#include "flyemtododialog.h"

#include <QSortFilterProxyModel>

#include "ui_flyemtododialog.h"
#include "flyem/zflyemtodolistmodel.h"


FlyEmTodoDialog::FlyEmTodoDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmTodoDialog)
{
  ui->setupUi(this);

  init();
}

FlyEmTodoDialog::~FlyEmTodoDialog()
{
  delete ui;
}

void FlyEmTodoDialog::init()
{
  m_model = new ZFlyEmTodoListModel(this);

  ui->todoTableView->setModel(m_model->getSortProxy());
  ui->todoTableView->setSortingEnabled(true);

  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(updateTable()));
  connect(ui->todoTableView, SIGNAL(doubleClicked(QModelIndex)),
          m_model, SLOT(processDoubleClick(QModelIndex)));
}

void FlyEmTodoDialog::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_model->setDocument(doc);
}

void FlyEmTodoDialog::updateTable()
{
  m_model->update();
}
