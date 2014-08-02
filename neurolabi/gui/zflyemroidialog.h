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

class ZFlyEmRoiDialog : public QDialog, ZQtBarProgressReporter
{
  Q_OBJECT

public:
  explicit ZFlyEmRoiDialog(QWidget *parent = 0);
  ~ZFlyEmRoiDialog();

public:
  void dump(const QString &str);

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

private slots:
  void on_searchPushButton_clicked();

private:
  void loadGrayscaleFunc(int z);
  ZIntCuboid estimateBoundBox(const ZStack &stack);

private:
  Ui::ZFlyEmRoiDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZSpinBoxDialog *m_zDlg;
  ZFlyEmRoiProject m_project;
  ZStackDocReader m_docReader;
};

#endif // ZFLYEMROIDIALOG_H
