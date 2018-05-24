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
class ZFlyEmBookmarkView;
class ZColorLabel;
class ZFlyEmProofMvc;

namespace Ui {
class FlyEmProofControlForm;
}

class FlyEmProofControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmProofControlForm(QWidget *parent = 0);
  ~FlyEmProofControlForm();

  ZFlyEmBookmarkView* getUserBookmarkView() const;
  ZFlyEmBookmarkView* getAssignedBookmarkView() const;

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
  void meshViewTriggered();
  void bodyViewTriggered();
  void savingMerge();
  void committingMerge();
  void zoomingTo(int x, int y, int z);
  void locatingBody(uint64_t);
  void goingToBody();
  void selectingBody();
  void bookmarkChecked(QString, bool);
  void bookmarkChecked(ZFlyEmBookmark*);
  void removingBookmark(ZFlyEmBookmark*);
  void removingBookmark(QList<ZFlyEmBookmark*>);
  void userBookmarkChecked(ZFlyEmBookmark*);
  void changingColorMap(QString);
  void clearingBodyMergeStage();
  void exportingSelectedBody();
  void exportingSelectedBodyLevel();
  void exportingGrayscale();
  void exportingSelectedBodyStack();
  void skeletonizingSelectedBody();
  void updatingMeshForSelectedBody();
  void showingInfo();
  void reportingBodyCorruption();

public slots:
  void setInfo(const QString &info);
  void updateWidget(const ZDvidTarget &target);
  void setDvidInfo(const ZDvidTarget &target);
//  void updateBookmarkTable(ZFlyEmBodyMergeProject *project);
//  void clearBookmarkTable(ZFlyEmBodyMergeProject *project);
//  void updateUserBookmarkTable(ZStackDoc *doc);
//  void removeBookmarkFromTable(ZFlyEmBookmark *bookmark);
  void updateLatency(int t);
  void updateWidget(ZFlyEmProofMvc *mvc);

private slots:
  void setSegmentSize();
  void incSegmentSize();
  void decSegmentSize();
  void showFullSegmentation(bool on);
  void goToPosition();
  void goToBody();
  void selectBody();
  void locateAssignedBookmark(const QModelIndex &index);
  void locateUserBookmark(const QModelIndex &index);
  void locateBookmark(const ZFlyEmBookmark *bookmark);
  void changeColorMap(QAction *action);
  void enableNameColorMap(bool on);
//  void enableSequencerColorMap(bool on);
  void clearBodyMergeStage();
  void exportSelectedBody();
  void exportSelectedBodyLevel();
  void exportSelectedBodyStack();
  void skeletonizeSelectedBody();
  void updateMeshForSelectedBody();
  void exportGrayscale();

private:
  void createMenu();
  void createColorMenu();
  QSortFilterProxyModel *createSortingProxy(ZFlyEmBookmarkListModel *model);

private:
  Ui::FlyEmProofControlForm *ui;
  QMenu *m_mainMenu;
//  ZFlyEmBookmarkListModel *m_assignedBookmarkList;
//  ZFlyEmBookmarkListModel *m_userBookmarkList;

  QAction *m_nameColorAction;

//  QAction *m_sequencerColorAction;
//  QAction *m_protocolColorAction;

  ZColorLabel *m_latencyWidget;

//  QSortFilterProxyModel *m_bookmarkProxy;
//  QSortFilterProxyModel *m_userBookmarkProxy;

//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H
