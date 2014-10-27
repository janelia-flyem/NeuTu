#ifndef DVIDIMAGEDIALOG_H
#define DVIDIMAGEDIALOG_H

#include <QDialog>
#include "dvid/zdvidtarget.h"

namespace Ui {
class DvidImageDialog;
}

class ZDvidDialog;
class ZSpinBoxDialog;

class DvidImageDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidImageDialog(QWidget *parent = 0);
  ~DvidImageDialog();

  int getX() const;
  int getY() const;
  int getZ() const;
  int getWidth() const;
  int getHeight() const;
  int getDepth() const;
  //QString getAddress() const;
  void setAddress(const QString address);
  void setDvidDialog(ZDvidDialog *dlg);
  inline const ZDvidTarget& getDvidTarget() {
    return m_dvidTarget;
  }

public slots:
  void showDvidDialog();
  void setSinglePlane();

private:
  void updateDvidTarget();

private:
  Ui::DvidImageDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZDvidTarget m_dvidTarget;
  ZSpinBoxDialog *m_singlePlaneDialog;
};

#endif // DVIDIMAGEDIALOG_H
