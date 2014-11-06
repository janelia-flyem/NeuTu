#ifndef FLYEMBODYSPLITPROJECTDIALOG_H
#define FLYEMBODYSPLITPROJECTDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include "flyem/zflyembodysplitproject.h"
#include "dvid/zdvidtarget.h"

class MainWindow;
class ZFlyEmNewBodySplitProjectDialog;
class QProgressDialog;
class ZDvidDialog;
class QMenu;
class QAction;

namespace Ui {
class FlyEmBodySplitProjectDialog;
}

class FlyEmBodySplitProjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodySplitProjectDialog(QWidget *parent = 0);
  ~FlyEmBodySplitProjectDialog();

  void setDvidTarget(const ZDvidTarget &target);
  void setBodyId(int id);

  int getBodyId() const;
  const ZDvidTarget& getDvidTarget() const;

  MainWindow* getMainWindow();
  QProgressDialog* getProgressDialog();

  void setDataFrame(ZStackFrame *frame);

  void closeEvent(QCloseEvent *event);
  void setLoadBodyDialog(ZFlyEmNewBodySplitProjectDialog *dlg);

  void updateButton();
  void updateWidget();
  void updateBookmarkTable();

  bool isBodyLoaded() const;

  void downloadSeed();

signals:
  //void progressStarted();
  void progressDone();
  void messageDumped(const QString &message, bool appending);
  void sideViewReady();
  void sideViewCanceled();

public slots:
  void clear();
  void shallowClear();
  void shallowClearResultWindow();
  void shallowClearDataFrame();

  void showData2d();
  void showData3d();
  void showResult3d();
  void loadBody();
  void loadBookmark();
  void locateBookmark(const QModelIndex &index);
  void quickView();
  void viewPreviousSlice();
  void viewNextSlice();

  void resetSideView();

  /*!
   * \brief Dump information
   */
  void dump(const QString &info, bool appending = false);


private slots:
  void on_pushButton_clicked();

  void on_dvidPushButton_clicked();

  void on_commitPushButton_clicked();
  void showBodyMask(bool on);

private:
  void updateSideView();
  void updateSideViewFunc();
  void initSideViewScene();
  void startProgress(const QString &label);
  void connectSignalSlot();
  void createMenu();

private:
  Ui::FlyEmBodySplitProjectDialog *ui;
  ZFlyEmNewBodySplitProjectDialog *m_loadBodyDlg;
  ZFlyEmBodySplitProject m_project;
  ZFlyEmBookmarkListModel m_bookmarkList;
  QGraphicsScene *m_sideViewScene;
  ZDvidDialog *m_dvidDlg;
  QMenu *m_mainMenu;
  QAction *m_showBodyMaskAction;
};

#endif // FLYEMBODYSPLITPROJECTDIALOG_H
