#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <cstdint>

#include <QWidget>

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
  explicit FlyEmProofControlForm(QWidget *parent = Q_NULLPTR);
  ~FlyEmProofControlForm();

  ZFlyEmBookmarkView* getUserBookmarkView() const;
  ZFlyEmBookmarkView* getAssignedBookmarkView() const;

  void setMainMenu(QMenu *menu);

signals:
  void segmentVisibleChanged(bool visible);
  void mergingSelected();
  void dvidSetTriggered();
  void dvidSetTriggered(const QString &name);
  void splitTriggered(uint64_t bodyId);
  void splitTriggered();
  void labelSizeChanged(int width, int height);
  void showingFullSegmentation();
  void coarseBodyViewTriggered();
  void skeletonViewTriggered();
  void meshViewTriggered();
  void coarseMeshViewTriggered();
  void bodyViewTriggered();
  void savingMerge();
  void committingMerge();
  void zoomingTo(int x, int y, int z);
  void zoomingToAssigned(int x, int y, int z);
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
  void queryingBody();
  void exportingSelectedBodyLevel();
  void exportingGrayscale();
  void exportingSelectedBodyStack();
  void skeletonizingSelectedBody();
  void skeletonizingTopBody();
  void skeletonizingBodyList();
  void updatingMeshForSelectedBody();
  void showingInfo();
  void reportingBodyCorruption();
  void importingUserBookmark();
  void exportingUserBookmark();

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
//  void locateAssignedBookmark(const QModelIndex &index);
//  void locateUserBookmark(const QModelIndex &index);
  void locateBookmark(const ZFlyEmBookmark *bookmark);
  void locateAssignedBookmark(const ZFlyEmBookmark *bookmark);

  void changeColorMap(QAction *action);
  void enableNameColorMap(bool on);
//  void enableSequencerColorMap(bool on);
  void clearBodyMergeStage();
  void exportSelectedBody();
  void queryBody();
  void exportSelectedBodyLevel();
  void exportSelectedBodyStack();
  void skeletonizeSelectedBody();
  void skeletonizeTopBody();
  void skeletonizeBodyList();
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
