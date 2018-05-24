#ifndef FLYEMBODYIDDIALOG_H
#define FLYEMBODYIDDIALOG_H

#include <QDialog>
#include <QVector>

#include "tz_stdint.h"

namespace Ui {
class FlyEmBodyIdDialog;
}

class FlyEmBodyIdDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyIdDialog(QWidget *parent = 0);
  ~FlyEmBodyIdDialog();

  int getBodyId() const;
  std::vector<uint64_t> getBodyIdArray() const;
  std::vector<int> getDownsampleInterval() const;

private:
  Ui::FlyEmBodyIdDialog *ui;
};

#endif // FLYEMBODYIDDIALOG_H
