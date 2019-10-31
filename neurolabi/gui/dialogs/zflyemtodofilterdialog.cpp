#include "zflyemtodofilterdialog.h"
#include "ui_zflyemtodofilterdialog.h"

#include "flyem/zflyemtodoitem.h"

ZFlyEmTodoFilterDialog::ZFlyEmTodoFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmTodoFilterDialog)
{
  ui->setupUi(this);
}

ZFlyEmTodoFilterDialog::~ZFlyEmTodoFilterDialog()
{
  delete ui;
}

bool ZFlyEmTodoFilterDialog::passed(const ZFlyEmToDoItem *item) const
{
  if (item) {
    return passed(*item);
  }

  return false;
}

bool ZFlyEmTodoFilterDialog::passed(const ZFlyEmToDoItem &item) const
{
  if (ui->todoComboBox->currentText() == "All") {
    return true;
  } else if (ui->todoComboBox->currentText() == "Done") {
    return item.isChecked();
  } else if (ui->todoComboBox->currentText() == "Normal") {
    return item.getAction() == neutu::EToDoAction::TO_DO;
  }

  return false;
}
