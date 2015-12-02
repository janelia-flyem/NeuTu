#include "dvidoperatedialog.h"
#include <QInputDialog>
#include <QMessageBox>

#include "ui_dvidoperatedialog.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"

DvidOperateDialog::DvidOperateDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidOperateDialog)
{
  ui->setupUi(this);

  m_dvidDlg = NULL;
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
