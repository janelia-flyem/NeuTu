#include "dvidoperatedialog.h"
#include <QInputDialog>
#include <QMessageBox>

#include "ui_dvidoperatedialog.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dialogs/zcontrastprotocaldialog.h"

DvidOperateDialog::DvidOperateDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidOperateDialog)
{
  ui->setupUi(this);

  m_dvidDlg = NULL;
  m_contrastDlg = new ZContrastProtocalDialog(this);
//  m_dvidDlg = new ZDvidDialog(this);
}

DvidOperateDialog::~DvidOperateDialog()
{
  delete ui;
}

void DvidOperateDialog::on_dvidPushButton_clicked()
{
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    ui->dvidLabel->setText(target.getSourceString(false).c_str());
  }
}

void DvidOperateDialog::on_creatDataPushButton_clicked()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Create Data"),
                                       tr("Data name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok && !text.isEmpty()) {
    ZDvidWriter writer;
    std::string dataType = ui->typeComboBox->currentText().toStdString();
    if (writer.open(m_dvidDlg->getDvidTarget())) {
//      writer.createKeyvalue(text.toStdString());
      writer.createData(dataType, text.toStdString());
      if (writer.getStatusCode() != 200) {
        QMessageBox::warning(this, "Operation Failed", "Failed to create data.");
      }
    } else {
      QMessageBox::warning(
            this, "Operation Failed", "Please specify a valid DVID target.");
    }
  }
}

void DvidOperateDialog::setDvidDialog(ZDvidDialog *dlg)
{
  m_dvidDlg = dlg;
}

void DvidOperateDialog::on_contrastPushButton_clicked()
{
  if (m_contrastDlg->exec()) {
    ZJsonObject obj = m_contrastDlg->getContrastProtocal();
    ZDvidReader reader;
    if (reader.open(m_dvidDlg->getDvidTarget())) {
      ZDvidWriter writer;
      writer.open(m_dvidDlg->getDvidTarget());

      if (!reader.hasData("neutu_config")) {
        writer.createData("keyvalue", "neutu_config", false);
      }

      writer.writeJson("neutu_config", "contrast", obj);
    }
  }
}

void DvidOperateDialog::on_addMasterPushButton_clicked()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("Create Master Node"),
                                       tr("uuid:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok && !text.isEmpty()) {
    ZDvidTarget target = m_dvidDlg->getDvidTarget();
    target.setUuid(text.toStdString());
    ZDvidReader reader;
    if (reader.open(target)) {
      ZDvidWriter writer;
      writer.open(m_dvidDlg->getDvidTarget());
      writer.writeMasterNode(text.toStdString());
      if (writer.getStatusCode() == 200) {
        QMessageBox::information(
              this, "Master Node Updated",
              QString("The master node is changed to %1").arg(text));
      } else {
        QMessageBox::warning(this, "Update Failed", "Unable to set the master node.");
      }
    } else {
      QMessageBox::warning(this, "Update Failed",
                           QString("Unable to open the uuid %1.").arg(text));
    }
  }
}
