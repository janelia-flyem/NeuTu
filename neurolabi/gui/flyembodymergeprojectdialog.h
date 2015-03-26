#ifndef FLYEMBODYMERGEPROJECTDIALOG_H
#define FLYEMBODYMERGEPROJECTDIALOG_H

#include <QDialog>
#include <QSet>

#include "flyem/zflyembodymergeproject.h"
#include "flyemprojectdialog.h"
#include "zmessageprocessor.h"

class QModelIndex;
class ZDvidVersionModel;
class ZMessage;
class ZMessageManager;

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

  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

  void enableMessageManager();


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

  ZMessageManager *m_messageManager;

//  ZFlyEmBookmarkListModel m_bookmarkList;
//  QGraphicsScene *m_sideViewScene;
//  ZDvidDialog *m_dvidDlg;
//  QMenu *m_mainMenu;
//  QAction *m_showBodyMaskAction;
};

#endif // FLYEMBODYMERGEPROJECTDIALOG_H
