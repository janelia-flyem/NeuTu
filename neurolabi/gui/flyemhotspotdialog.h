#ifndef FLYEMHOTSPOTDIALOG_H
#define FLYEMHOTSPOTDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmHotSpotDialog;
}

class FlyEmHotSpotDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmHotSpotDialog(QWidget *parent = 0);
  ~FlyEmHotSpotDialog();

  int getId() const;

private:
  Ui::FlyEmHotSpotDialog *ui;
};

#endif // FLYEMHOTSPOTDIALOG_H
