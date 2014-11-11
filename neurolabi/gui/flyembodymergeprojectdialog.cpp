#include "flyembodymergeprojectdialog.h"
#include "ui_flyembodymergeprojectdialog.h"

FlyEmBodyMergeProjectDialog::FlyEmBodyMergeProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyMergeProjectDialog)
{
  ui->setupUi(this);
}

FlyEmBodyMergeProjectDialog::~FlyEmBodyMergeProjectDialog()
{
  delete ui;
}

void FlyEmBodyMergeProjectDialog::setPushButtonSlots()
{

}
