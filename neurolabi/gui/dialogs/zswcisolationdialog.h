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

  double getXScale() const;
  double getYScale() const;
  double getZScale() const;

  void setScale(double xs, double ys, double zs);

private:
  Ui::ZSwcIsolationDialog *ui;
};

#endif // ZSWCISOLATIONDIALOG_H
