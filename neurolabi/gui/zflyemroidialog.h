#ifndef ZFLYEMROIDIALOG_H
#define ZFLYEMROIDIALOG_H

#include <QDialog>
#include <QVector>
#include "zdialogfactory.h"
#include "flyem/zflyemroiproject.h"
#include "zqtbarprogressreporter.h"
#include "zstackdoc.h"
#include "zintcuboid.h"

class MainWindow;
class ZDvidTarget;

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
  void dump(const QString &str, bool appending = false);
  void loadGrayscale(int z);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  bool appendProject(ZFlyEmRoiProject *project);
  ZFlyEmRoiProject* getProject(size_t index);

  ZFlyEmRoiProject* newProject(const std::string &name);

  bool isValidName(const std::string &name) const;

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
  void loadProject(int index);

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

  void on_pushButton_clicked();

private:
  void loadGrayscaleFunc(int z);
  void downloadAllProject();
  void uploadProjectList();

private:
  Ui::ZFlyEmRoiDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZSpinBoxDialog *m_zDlg;
  QVector<ZFlyEmRoiProject*> m_projectList;
  ZFlyEmRoiProject *m_project;
  ZDvidTarget m_dvidTarget;
  ZStackDocReader m_docReader;
};

#endif // ZFLYEMROIDIALOG_H
