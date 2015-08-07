#include "dvidimagedialog.h"
#include "ui_dvidimagedialog.h"
#include "neutubeconfig.h"
#include "zdviddialog.h"
#include "zdialogfactory.h"
#include "dvid/zdvidreader.h"

DvidImageDialog::DvidImageDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidImageDialog), m_dvidDlg(NULL)
{
  ui->setupUi(this);
#if defined(_FLYEM_)
  setAddress( NeutubeConfig::getInstance().getFlyEmConfig().
              getDvidTarget().getSourceString(false).c_str());
#endif
  connect(ui->dvidServerPushButton, SIGNAL(clicked()),
          this, SLOT(showDvidDialog()));
  connect(ui->singlePlanePushButton, SIGNAL(clicked()),
          this, SLOT(setSinglePlane()));

  ui->dvidServerPushButton->setEnabled(false);

  m_singlePlaneDialog = ZDialogFactory::makeSpinBoxDialog(this);
}

DvidImageDialog::~DvidImageDialog()
{
  delete ui;
}

int DvidImageDialog::getX() const
{
  return ui->xSpinBox->value();
}

int DvidImageDialog::getY() const
{
  return ui->ySpinBox->value();
}

int DvidImageDialog::getZ() const
{
  return ui->zSpinBox->value();
}

int DvidImageDialog::getWidth() const
{
  return ui->widthSpinBox->value();
}

int DvidImageDialog::getHeight() const
{
  return ui->heightSpinBox->value();
}

void DvidImageDialog::setDvidDialog(ZDvidDialog *dlg)
{
  m_dvidDlg = dlg;

  updateDvidTarget();
  ui->dvidServerPushButton->setEnabled(m_dvidDlg != NULL);
}

int DvidImageDialog::getDepth() const
{
  return ui->depthSpinBox->value();
}
#if 0
QString DvidImageDialog::getAddress() const
{
  return ui->addressWidget->text();
}
#endif
void DvidImageDialog::setAddress(const QString address)
{
  ui->addressWidget->setText(address);
}

void DvidImageDialog::updateDvidTarget() {
  if (m_dvidDlg != NULL) {
    m_dvidTarget = m_dvidDlg->getDvidTarget();
    ui->addressWidget->setText(m_dvidTarget.getSourceString().c_str());
  }
}

void DvidImageDialog::showDvidDialog()
{
  if (m_dvidDlg->exec()) {
    updateDvidTarget();
  }
}

void DvidImageDialog::setSinglePlane()
{
  if (m_singlePlaneDialog->exec()) {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      QString infoString = reader.readInfo("grayscale");

      qDebug() << infoString;

      ZDvidInfo dvidInfo;
      dvidInfo.setFromJsonString(infoString.toStdString());

      int z = m_singlePlaneDialog->getValue();

      ui->xSpinBox->setValue(dvidInfo.getStartCoordinates().getX());
      ui->ySpinBox->setValue(dvidInfo.getStartCoordinates().getY());
      ui->zSpinBox->setValue(z);

      ui->widthSpinBox->setValue(dvidInfo.getStackSize()[0]);
      ui->heightSpinBox->setValue(dvidInfo.getStackSize()[1]);
      ui->depthSpinBox->setValue(1);
    }
  }
}
