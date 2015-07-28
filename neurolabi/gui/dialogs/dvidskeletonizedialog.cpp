#include "dvidskeletonizedialog.h"
#include "ui_dvidskeletonizedialog.h"
#include "neutubeconfig.h"
#include "zdialogfactory.h"

DvidSkeletonizeDialog::DvidSkeletonizeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidSkeletonizeDialog)
{
  ui->setupUi(this);

  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);
  m_bodyDlg = new FlyEmBodyFilterDialog(this);

  connect(ui->dvidDialogPushButton, SIGNAL(clicked()),
          this, SLOT(setDvidTarget()));
  connect(ui->selectBodyButton, SIGNAL(clicked()), this, SLOT(selectBody()));
  updateDvidTarget();
  updateBodySelection();
}

DvidSkeletonizeDialog::~DvidSkeletonizeDialog()
{
  delete ui;
}

void DvidSkeletonizeDialog::updateDvidTarget()
{
  m_dvidTarget = m_dvidDlg->getDvidTarget();
  ui->dvidServerEdit->setText(m_dvidTarget.getSourceString().c_str());
}

void DvidSkeletonizeDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    updateDvidTarget();
  }
}

int DvidSkeletonizeDialog::getMinBodySize() const
{
  return m_bodyDlg->getMinBodySize();
}

bool DvidSkeletonizeDialog::hasUpperBodySize() const
{
  return m_bodyDlg->hasUpperBodySize();
}

int DvidSkeletonizeDialog::getMaxBodySize() const
{
  return m_bodyDlg->getMaxBodySize();
}

std::set<int> DvidSkeletonizeDialog::getExcludedBodySet() const
{
  return m_bodyDlg->getExcludedBodySet();
}

void DvidSkeletonizeDialog::updateBodySelection()
{
  QString maxBodySize;
  if (hasUpperBodySize()) {
    maxBodySize = QString("%1").arg(getMaxBodySize());
  }

  QString text = QString("Body size range: [%1, %2]; Excluded bodies: ").
      arg(getMinBodySize()).arg(maxBodySize);
  std::set<int> bodySet = m_bodyDlg->getExcludedBodySet();
  for (std::set<int>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    text += QString("%1, ").arg(*iter);
  }

  ui->bodyInfoWidget->setPlainText(text);
}

void DvidSkeletonizeDialog::selectBody()
{
  if (m_bodyDlg->exec()) {
    updateBodySelection();
  }
}

bool DvidSkeletonizeDialog::noOverwriting() const
{
  return !ui->overwriteCheckBox->isChecked();
}
