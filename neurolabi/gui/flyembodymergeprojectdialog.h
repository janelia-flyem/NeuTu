#ifndef FLYEMBODYMERGEPROJECTDIALOG_H
#define FLYEMBODYMERGEPROJECTDIALOG_H

#include <QDialog>

#include "flyem/zflyembodymergeproject.h"
#include "flyemprojectdialog.h"

namespace Ui {
class FlyEmBodyMergeProjectDialog;
}

class FlyEmBodyMergeProjectDialog : public FlyEmProjectDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyMergeProjectDialog(QWidget *parent = 0);
  ~FlyEmBodyMergeProjectDialog();

  void createMenu();
  void setPushButtonSlots();
  void connectSignalSlot();
  void dump(const QString &str, bool appending = false);
  void showInfo(const QString &str, bool appending = false);

  void updateDataFrame(ZStackDocReader &docReader, bool readyForPaint);
  void clear();

  void setupProgress();

  void updateInfo();

public slots:
  void test();
  void setDvidTarget();
  void consumeNewDoc(ZStackDocReader *docReader, bool readyForPaint);
  void loadSlice();
  void moveSliceUp();
  void moveSliceDown();
  void moveSliceLeft();
  void moveSliceRight();
  void moveSliceUpLeft();
  void moveSliceDownRight();
  void showPreviousSlice();
  void showNextSlice();
  void notifySelection(const ZStackObjectSelector &selector);
  void notifyBodyMerged(QList<uint64_t> bodyLabelList);

private:
  Ui::FlyEmBodyMergeProjectDialog *ui;
  ZFlyEmBodyMergeProject *m_project;

//  ZFlyEmBookmarkListModel m_bookmarkList;
//  QGraphicsScene *m_sideViewScene;
//  ZDvidDialog *m_dvidDlg;
//  QMenu *m_mainMenu;
//  QAction *m_showBodyMaskAction;
};

#endif // FLYEMBODYMERGEPROJECTDIALOG_H
