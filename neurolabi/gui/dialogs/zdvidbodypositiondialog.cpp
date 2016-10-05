#include "zdvidbodypositiondialog.h"

#include <fstream>
#include <QMessageBox>

#include "ui_zdvidbodypositiondialog.h"
#include "zdialogfactory.h"
#include "zdviddialog.h"
#include "dvid/zdvidreader.h"
#include "zstring.h"
#include "zdialogfactory.h"

ZDvidBodyPositionDialog::ZDvidBodyPositionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidBodyPositionDialog)
{
  ui->setupUi(this);

  m_dvidDlg = NULL;

  connect(ui->dvidPushButton, SIGNAL(clicked()), this, SLOT(setDvid()));
  connect(ui->generatePushButton, SIGNAL(clicked()),
          this, SLOT(generatePosition()));
  connect(ui->filePushButton, SIGNAL(clicked()), this, SLOT(loadInputFile()));
}

ZDvidBodyPositionDialog::~ZDvidBodyPositionDialog()
{
  delete ui;
}

void ZDvidBodyPositionDialog::setDvidDialog(ZDvidDialog *dlg)
{
  m_dvidDlg = dlg;
}

void ZDvidBodyPositionDialog::loadInputFile()
{
  QString filePath = ZDialogFactory::GetOpenFileName("Body File", "", this);
  if (!filePath.isEmpty()) {
    ui->fileLineEdit->setText(filePath);
  }
}

void ZDvidBodyPositionDialog::generatePosition()
{
  QString filePath = ui->fileLineEdit->text();
  QList<uint64_t> bodyList;
  if (!filePath.isEmpty()) {
    FILE *fp = fopen(filePath.toStdString().c_str(), "r");
    ZString str;
    while (str.readLine(fp)) {
      std::vector<uint64_t> bodyIdArray = str.toUint64Array();
      for (std::vector<uint64_t>::const_iterator iter = bodyIdArray.begin();
           iter != bodyIdArray.end(); ++iter) {
        uint64_t bodyId = *iter;
        if (bodyId > 0) {
          bodyList.append(bodyId);
        }
      }
    }
  }

  ZDvidReader reader;
  if (!bodyList.isEmpty() && reader.open(m_dvidDlg->getDvidTarget())) {
    QString filePath = ZDialogFactory::GetSaveFileName("Position File", "", this);
    if (!filePath.isEmpty()) {
      std::ofstream stream;
      stream.open(filePath.toStdString().c_str());
      if (stream.is_open()) {
        foreach (uint64_t bodyId, bodyList) {
          ZIntPoint pt = reader.readBodyLocation(bodyId);
          stream << pt.getX() << ", " << pt.getY() << ", " << pt.getZ() << std::endl;
        }
      }

      QMessageBox::information(
            this, "Export Done", QString("%1 has been saved.").arg(filePath));
    }
  }
}

void ZDvidBodyPositionDialog::setDvid()
{
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    ui->dvidLabel->setText(target.getSourceString(false).c_str());
  }
}
