#include "flyemtododialog.h"
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
  ui->todoTableView->setModel(m_model);

  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(updateTable()));
}

void FlyEmTodoDialog::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_model->setDocument(doc);
}

void FlyEmTodoDialog::updateTable()
{
  m_model->update();
}
