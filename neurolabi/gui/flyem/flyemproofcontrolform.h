#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>

//class ZDvidDialog;

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
  void dvidSetTriggered();
  void splitTriggered(int64_t bodyId);

private:
  Ui::FlyEmProofControlForm *ui;
//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H
