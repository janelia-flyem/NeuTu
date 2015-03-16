#ifndef FLYEMBODYMERGEPROJECTDIALOG_H
#define FLYEMBODYMERGEPROJECTDIALOG_H

#include <QDialog>
#include <QSet>

#include "flyem/zflyembodymergeproject.h"
#include "flyemprojectdialog.h"

class QModelIndex;
class ZDvidVersionModel;

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
  void dump(const QString &str, bool appending = false);
  void showInfo(const QString &str, bool appending = false);

  void updateDataFrame(ZStackDocReader &docReader, bool readyForPaint);
  void clear();

  void setupProgress();

  void updateInfo();

  inline ZFlyEmBodyMergeProject* getProject() {
    return m_project;
  }

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
  void changeDvidNode(const std::string &newUuid);
  void changeDvidNode(const QModelIndex &index);
  void lockNode();
  void createVersionBranch();

private:
    void connectSignalSlot();
    void connectProjectSignalSlot();
    void setPushButtonSlots();
    void updateVersionTree();
    ZDvidVersionModel* getVersionModel();
    QModelIndex getSelectedVersionIndex() const;
    void activateCurrentNode();

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
