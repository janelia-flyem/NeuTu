#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>

namespace Ui {
class FlyEmProofControlForm;
}

class FlyEmProofControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmProofControlForm(QWidget *parent = 0);
  ~FlyEmProofControlForm();

signals:
  void segmentVisibleChanged(bool visible);
  void mergingSelected();

private:
  Ui::FlyEmProofControlForm *ui;
};

#endif // FLYEMPROOFCONTROLFORM_H
