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

  void updateDataFrame(ZStackDocReader &docReader);
  void clear();

  void setupProgress();

public slots:
  void test();
  void setDvidTarget();
  void consumeNewDoc(ZStackDocReader *docReader);
  void loadSlice();

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
