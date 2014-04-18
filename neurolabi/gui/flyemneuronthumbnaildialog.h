#ifndef FLYEMNEURONTHUMBNAILDIALOG_H
#define FLYEMNEURONTHUMBNAILDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmNeuronThumbnailDialog;
}

class FlyEmNeuronThumbnailDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmNeuronThumbnailDialog(QWidget *parent = 0);
  ~FlyEmNeuronThumbnailDialog();

  void setOutputDirectory(const QString &path, const QString &subdir);
  QString getOutputDirectory();
  int getXIntv();
  int getYIntv();
  int getZIntv();
  bool updatingDataBundle();


private:
  Ui::FlyEmNeuronThumbnailDialog *ui;
};

#endif // FLYEMNEURONTHUMBNAILDIALOG_H
