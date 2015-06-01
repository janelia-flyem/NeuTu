#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>
#include "tz_stdint.h"
//class ZDvidDialog;

class QMenu;
class ZDvidTarget;

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
  void splitTriggered();
  void labelSizeChanged(int width, int height);
  void coarseBodyViewTriggered();
  void savingMerge();
  void zoomingTo(int x, int y, int z);
  void locatingBody(uint64_t);

public slots:
  void setInfo(const QString &info);
  void setDvidInfo(const ZDvidTarget &target);

private slots:
  void setSegmentSize();
  void incSegmentSize();
  void decSegmentSize();
  void goToPosition();
  void goToBody();

private:
  void createMenu();

private:
  Ui::FlyEmProofControlForm *ui;
  QMenu *m_mainMenu;

//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H
