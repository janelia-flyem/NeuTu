#include "zflyemnewbodysplitprojectdialog.h"
#include "ui_zflyemnewbodysplitprojectdialog.h"
#include "zdviddialog.h"

ZFlyEmNewBodySplitProjectDialog::ZFlyEmNewBodySplitProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmNewBodySplitProjectDialog),
  m_dvidDlg(NULL)
{
  ui->setupUi(this);
  connect(ui->dvidDialogPushButton, SIGNAL(clicked()),
          this, SLOT(showDvidDialog()));
}

ZFlyEmNewBodySplitProjectDialog::~ZFlyEmNewBodySplitProjectDialog()
{
  delete ui;
}

void ZFlyEmNewBodySplitProjectDialog::showDvidDialog()
{
  if (m_dvidDlg != NULL) {
    if (m_dvidDlg->exec()) {
      setDvidTarget(m_dvidDlg->getDvidTarget());
    }
  }
}

void ZFlyEmNewBodySplitProjectDialog::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  ui->dvidServerEdit->setText(m_dvidTarget.getSourceString(false).c_str());
}

void ZFlyEmNewBodySplitProjectDialog::setDvidDialog(ZDvidDialog *dlg)
{
  m_dvidDlg = dlg;
  setDvidTarget(m_dvidDlg->getDvidTarget());
}

int ZFlyEmNewBodySplitProjectDialog::getBodyId() const
{
  return ui->bodyIdWidget->value();
}
