#ifndef ZFLYEMROIDIALOG_H
#define ZFLYEMROIDIALOG_H

#include <QDialog>
#include "zdialogfactory.h"
#include "flyem/zflyemroiproject.h"
#include "zqtbarprogressreporter.h"
#include "zstackdoc.h"
#include "zintcuboid.h"

class MainWindow;

namespace Ui {
class ZFlyEmRoiDialog;
}

class ZFlyEmRoiDialog : public QDialog, ZProgressable
{
  Q_OBJECT

public:
  explicit ZFlyEmRoiDialog(QWidget *parent = 0);
  ~ZFlyEmRoiDialog();

public:
  void dump(const QString &str);
  void loadGrayscale(int z);

public slots:
  void loadGrayscale();
  void setDvidTarget();
  MainWindow* getMainWindow();
  void updateWidget();
  void setDataFrame(ZStackFrame *frame);
  void shallowClearDataFrame();
  void addRoi();

  void clear();

  void newDataFrame();
  void setZ(int z);
  void previewFullRoi();
  void uploadRoi();
  void estimateRoi();

signals:
  void newDocReady();
  void progressFailed();
  void progressAdvanced(double);

protected:
    void closeEvent(QCloseEvent*event);

private slots:
  void on_searchPushButton_clicked();

  void on_testPushButton_clicked();

  void on_xIncPushButton_clicked();

  void on_xDecPushButton_clicked();

  void on_yDecPushButton_clicked();

  void on_yIncPushButton_clicked();

  void on_rotateLeftPushButton_clicked();

  void on_rotateRightPushButton_clicked();

  void on_xyDecPushButton_clicked();

  void on_xyIncPushButton_clicked();

  void on_movexyDecPushButton_clicked();

  void on_movexyIncPushButton_clicked();

  void on_movexDecPushButton_clicked();

  void on_movexIncPushButton_clicked();

  void on_moveyDecPushButton_clicked();

  void on_moveyIncPushButton_clicked();

  void advanceProgressSlot(double p);

private:
  void loadGrayscaleFunc(int z);

private:
  Ui::ZFlyEmRoiDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZSpinBoxDialog *m_zDlg;
  ZFlyEmRoiProject m_project;
  ZStackDocReader m_docReader;
};

#endif // ZFLYEMROIDIALOG_H
