#ifndef FLYEMBODYMERGEPROJECTDIALOG_H
#define FLYEMBODYMERGEPROJECTDIALOG_H

#include <QDialog>

#include "flyem/zflyembodymergeproject.h"

namespace Ui {
class FlyEmBodyMergeProjectDialog;
}

class FlyEmBodyMergeProjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyMergeProjectDialog(QWidget *parent = 0);
  ~FlyEmBodyMergeProjectDialog();

  void setPushButtonSlots();

public slots:


private:
  Ui::FlyEmBodyMergeProjectDialog *ui;
  ZFlyEmBodyMergeProject m_project;
//  ZFlyEmBookmarkListModel m_bookmarkList;
//  QGraphicsScene *m_sideViewScene;
//  ZDvidDialog *m_dvidDlg;
//  QMenu *m_mainMenu;
//  QAction *m_showBodyMaskAction;
};

#endif // FLYEMBODYMERGEPROJECTDIALOG_H
