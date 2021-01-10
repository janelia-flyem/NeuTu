#include "zinfodialog.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>

#include "ui_zinfodialog.h"

#include "zglobal.h"
#include "zdialogfactory.h"

ZInfoDialog::ZInfoDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZInfoDialog)
{
  ui->setupUi(this);
  ui->infoWidget->setReadOnly(true);
  connect(ui->savePushButton, SIGNAL(clicked()), this, SLOT(save()));
  connect(ui->copyPushButton, SIGNAL(clicked()), this, SLOT(copyToClipBoard()));
}

ZInfoDialog::~ZInfoDialog()
{
  delete ui;
}

void ZInfoDialog::setText(const QString &text)
{
  ui->infoWidget->setPlainText(text);
}

void ZInfoDialog::copyToClipBoard() const
{
  ZGlobal::CopyToClipboard(ui->infoWidget->toPlainText().toStdString());
}

void ZInfoDialog::save() const
{
  QString fileName = ZDialogFactory::GetSaveFileName("Save Info", "", nullptr);
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      ZDialogFactory::Warn(
            "Save Failed", "Cannot open " + fileName + " to save", nullptr);
      return;
    } else {
      QTextStream out(&file);
      out << ui->infoWidget->toPlainText();
    }
  }
}
