#include "flyemtododialog.h"

#include <iostream>
#include <QSortFilterProxyModel>

#include "ui_flyemtododialog.h"
#include "flyem/zflyemtodolistmodel.h"
#include "flyem/widgets/zflyemtodolistview.h"

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

  ui->todoTableView->setModel(m_model->getProxy());
  ui->todoTableView->setSortingEnabled(true);

  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(updateTable()));
  connect(ui->todoTableView, SIGNAL(doubleClicked(QModelIndex)),
          m_model, SLOT(processDoubleClick(QModelIndex)));
  connect(ui->filterLineEdit, &QLineEdit::textChanged,
          this, &FlyEmTodoDialog::updateTextFilter);
  connect(ui->allRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
  connect(ui->checkedRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
  connect(ui->uncheckedRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
}

void FlyEmTodoDialog::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_model->setDocument(doc);
}

void FlyEmTodoDialog::updateTable()
{
  m_model->update();
}

void FlyEmTodoDialog::updateVisibility(bool on)
{
  if (on) {
#ifdef _DEBUG_
    std::cout << "Update visibility" << std::endl;
#endif
    if (ui->allRadioButton->isChecked()) {
      m_model->setAllVisible();
    } else if (ui->checkedRadioButton->isChecked()) {
      m_model->setCheckedVisibleOnly();
    } else if (ui->uncheckedRadioButton->isChecked()) {
      m_model->setUncheckedVisibleOnly();
    }
  }
}

void FlyEmTodoDialog::updateTextFilter(QString text)
{
  ZFlyEmTodoListView *view = ui->todoTableView;
  view->setStringFilter(text);
  view->sort();
}
