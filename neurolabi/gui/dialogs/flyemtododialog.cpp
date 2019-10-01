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
  connect(ui->todoTableView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(onDoubleClicked(QModelIndex)));
  connect(ui->filterLineEdit, &QLineEdit::textChanged,
          this, &FlyEmTodoDialog::updateTextFilter);
  connect(ui->allRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
  connect(ui->checkedRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
  connect(ui->uncheckedRadioButton, &QRadioButton::toggled,
          this, &FlyEmTodoDialog::updateVisibility);
  connect(ui->checkPushButton, &QPushButton::clicked,
          this, &FlyEmTodoDialog::checkSelected);
  connect(ui->uncheckPushButton, &QPushButton::clicked,
          this, &FlyEmTodoDialog::uncheckSelected);
  connect(ui->autoCheckedCheckBox, SIGNAL(stateChanged(int)), this,
          SLOT(onAutoCheckedChanged(int)));
  connect(m_model, &ZFlyEmTodoListModel::checkingTodoItem,
          this, &FlyEmTodoDialog::checkingTodoItem);
}

void FlyEmTodoDialog::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_model->setDocument(doc);
}

void FlyEmTodoDialog::setChecked(bool checked)
{
  m_model->setSelectedChecked(ui->todoTableView->selectionModel(), checked);
}

void FlyEmTodoDialog::checkSelected()
{
  setChecked(true);
}

void FlyEmTodoDialog::uncheckSelected()
{
  setChecked(false);
}

void FlyEmTodoDialog::onDoubleClicked(QModelIndex index) {
  if (ui->autoCheckedCheckBox->isChecked()) {
    // if the setting is enabled, also set the to do checked state when
    //  double-clicking; this makes it faster to move through the items
    checkSelected();
  }
}

void FlyEmTodoDialog::updateTable()
{
  m_model->update();

  // control column widths explicitly; all of them should resize to contents, and
  //    comments column should take all the extra space
  for (int i=0; i<m_model->columnCount(); i++) {
      ui->todoTableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
  }
  ui->todoTableView->horizontalHeader()->setSectionResizeMode(ZFlyEmTodoPresenter::TODO_COMMENT_COLUMN,
                                                              QHeaderView::Stretch);
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
