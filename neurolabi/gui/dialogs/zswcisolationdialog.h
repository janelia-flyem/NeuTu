#ifndef ZSWCISOLATIONDIALOG_H
#define ZSWCISOLATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ZSwcIsolationDialog;
}

class ZSwcIsolationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZSwcIsolationDialog(QWidget *parent = 0);
  ~ZSwcIsolationDialog();

  double getLengthThreshold() const;
  double getDistanceThreshold() const;

private:
  Ui::ZSwcIsolationDialog *ui;
};

#endif // ZSWCISOLATIONDIALOG_H
