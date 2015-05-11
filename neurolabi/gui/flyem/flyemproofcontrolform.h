#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>
#include "tz_stdint.h"
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
  void splitTriggered(uint64_t bodyId);
  void labelSizeChanged(int width, int height);
  void coarseBodyViewTriggered();
  void savingMerge();

private slots:
  void setSegmentSize();
  void incSegmentSize();
  void decSegmentSize();

private:
  Ui::FlyEmProofControlForm *ui;

//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H
