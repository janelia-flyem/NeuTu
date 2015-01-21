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

  /*!
   * \brief Set DVID target.
   *
   * This function does not check if \a target is valid.
   */
  void setDvidTarget(const ZDvidTarget &target);

  /*!
   * \brief Set the current body ID.
   *
   * \param id The ID of the body to work on.
   */
  void setBodyId(int id);

  /*!
   * \brief Get the current body ID.
   */
  int getBodyId() const;

  const ZDvidTarget& getDvidTarget() const;

  MainWindow* getMainWindow();
  QProgressDialog* getProgressDialog();

  /*!
   * \brief Set the data frame.
   */
  void setDataFrame(ZStackFrame *frame);

  void closeEvent(QCloseEvent *event);
  void setLoadBodyDialog(ZFlyEmNewBodySplitProjectDialog *dlg);

  void updateButton();
  void updateWidget();
  void updateBookmarkTable();

  bool isBodyLoaded() const;

  void downloadSeed();

signals:
  void progressStarted(const QString &title, int nticks);
  void progressDone();
  void progressAdvanced(double dp);

  void messageDumped(const QString &message, bool appending = true);
  void sideViewReady();
  void sideViewCanceled();

public slots:
  void clear();
  void shallowClear();
  void shallowClearResultWindow();
  void shallowClearDataFrame();

  bool showData2d();
  void showData3d();
  void showResult3d();
  void showResult3dQuick();
  bool loadBody();
  void loadBookmark();
  void locateBookmark(const QModelIndex &index);
  void quickView();
  void viewPreviousSlice();
  void viewNextSlice();
  void viewFullGrayscale();
  void saveSeed();

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
  void checkAllSeed();

  /*!
   * \brief Process all stored seeds.
   *
   * After processing the seeds will be labeled as "processed"
   */
  void processAllSeed();

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
