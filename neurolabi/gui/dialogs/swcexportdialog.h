#ifndef SWCEXPORTDIALOG_H
#define SWCEXPORTDIALOG_H

#include <QDialog>

#include "flyem/zflyemcoordinateconverter.h"

namespace Ui {
class SwcExportDialog;
}

class SwcExportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SwcExportDialog(QWidget *parent = 0);
  ~SwcExportDialog();

  QString getSavePath() const;
  ZFlyEmCoordinateConverter::ESpace getCoordSpace() const;


private slots:
  void on_pathPushButton_clicked();

private:
  Ui::SwcExportDialog *ui;
};

#endif // SWCEXPORTDIALOG_H
