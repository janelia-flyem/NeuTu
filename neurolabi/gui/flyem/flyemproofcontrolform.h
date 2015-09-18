#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>
#include "tz_stdint.h"
#include "zflyembookmarklistmodel.h"

class QMenu;
class ZDvidTarget;
class ZFlyEmBodyMergeProject;
class ZStackDoc;
class QSortFilterProxyModel;

namespace Ui {
class FlyEmProofControlForm;
}

class FlyEmProofControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmProofControlForm(QWidget *parent = 0);
  ~FlyEmProofControlForm();

signals:
  void segmentVisibleChanged(bool visible);
  void mergingSelected();
  void dvidSetTriggered();
  void splitTriggered(uint64_t bodyId);
  void splitTriggered();
  void labelSizeChanged(int width, int height);
  void showingFullSegmentation();
  void coarseBodyViewTriggered();
  void skeletonViewTriggered();
  void bodyViewTriggered();
  void savingMerge();
  void committingMerge();
  void zoomingTo(int x, int y, int z);
  void locatingBody(uint64_t);
  void goingToBody();
  void selectingBody();
  void bookmarkChecked(QString, bool);
  void bookmarkChecked(ZFlyEmBookmark*);
  void userBookmarkChecked(ZFlyEmBookmark*);
  void changingColorMap(QString);

public slots:
  void setInfo(const QString &info);
  void setDvidInfo(const ZDvidTarget &target);
  void updateBookmarkTable(ZFlyEmBodyMergeProject *project);
  void clearBookmarkTable(ZFlyEmBodyMergeProject *project);
  void updateUserBookmarkTable(ZStackDoc *doc);

private slots:
  void setSegmentSize();
  void incSegmentSize();
  void decSegmentSize();
  void showFullSegmentation();
  void goToPosition();
  void goToBody();
  void selectBody();
  void locateAssignedBookmark(const QModelIndex &index);
  void locateUserBookmark(const QModelIndex &index);
  void changeColorMap(QAction *action);

private:
  void createMenu();
  QSortFilterProxyModel *createSortingProxy(ZFlyEmBookmarkListModel *model);

private:
  Ui::FlyEmProofControlForm *ui;
  QMenu *m_mainMenu;
  ZFlyEmBookmarkListModel m_bookmarkList;
  ZFlyEmBookmarkListModel m_userBookmarkList;

  QSortFilterProxyModel *m_bookmarkProxy;
  QSortFilterProxyModel *m_userBookmarkProxy;

//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H
