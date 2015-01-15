#ifndef ZSEGMENTATIONPROJECTDIALOG_H
#define ZSEGMENTATIONPROJECTDIALOG_H

#include <QDialog>

class ZSegmentationProjectModel;
class MainWindow;
class ZStackFrame;
class ZStackDocReader;

namespace Ui {
class ZSegmentationProjectDialog;
}

class ZSegmentationProjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZSegmentationProjectDialog(QWidget *parent = 0);
  ~ZSegmentationProjectDialog();

  MainWindow* getMainWindow();

  ZStackFrame *newDataFrame(ZStackDocReader &reader);

private slots:
  void on_testPushButton_clicked();

  void on_updatePushButton_clicked();

  void on_openPushButton_clicked();

  void on_savePushButton_clicked();

  void on_donePushButton_clicked();

private:
  Ui::ZSegmentationProjectDialog *ui;
  ZSegmentationProjectModel *m_model;
  //ZStackFrame *m_dataFrame;
};

#endif // ZSEGMENTATIONPROJECTDIALOG_H
