#ifndef ZCONTRASTPROTOCALDIALOG_H
#define ZCONTRASTPROTOCALDIALOG_H

#include <QDialog>
//#include <QVector>
//#include <QRadioButton>

class ZJsonObject;

namespace Ui {
class ZContrastProtocalDialog;
}

class ZContrastProtocalDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZContrastProtocalDialog(QWidget *parent = 0);
  ~ZContrastProtocalDialog();

  ZJsonObject getContrastProtocal() const;
  void setContrastProtocol(const ZJsonObject &protocolJson); 

signals:
  void protocolChanged();
  void canceled();
  void committing();

private:
  double getOffsetStep() const;
  double getScaleStep() const;
  int getNonlinearMode() const;

private slots:
  void updateOffsetStep();
  void updateScaleStep();

private:
  Ui::ZContrastProtocalDialog *ui;
//  QVector<QRadioButton*> m_nonlinarGroup;
};

#endif // ZCONTRASTPROTOCALDIALOG_H
