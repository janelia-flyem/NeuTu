#ifndef FLYEMSPLITCONTROLFORM_H
#define FLYEMSPLITCONTROLFORM_H

#include <QWidget>
//#include "zflyembodysplitproject.h"

#include "flyem/zflyembookmarklistmodel.h"

class QMenu;
class ZFlyEmBodySplitProject;
class ZFlyEmBookmarkView;
class ZStackDoc;

namespace Ui {
class FlyEmSplitControlForm;
}

class FlyEmSplitControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmSplitControlForm(QWidget *parent = 0);
  ~FlyEmSplitControlForm();

  ZFlyEmBookmarkView* getUserBookmarkView() const;
  ZFlyEmBookmarkView* getAssignedBookmarkView() const;

signals:
  void exitingSplit();
  void quickViewTriggered();
  void coarseBodyViewTriggered();
  void splitQuickViewTriggered();
  void bodyViewTriggered();
  void meshViewTriggered();
  void splitViewTriggered();
  void changingSplit(uint64_t);
  void savingSeed();
  void committingResult();
  void zoomingTo(int x, int y, int z);
  void loadingBookmark(QString);
  void savingTask();
  void loadingSplitResult();
  void uploadingSplitResult();
  void recoveringSeed();
  void exportingSeed();
  void importingSeed();
  void selectingSeed();
  void selectingAllSeed();
  void settingMainSeed();
  void loadingSynapse();
  void bookmarkChecked(QString key, bool);
  void bookmarkChecked(ZFlyEmBookmark*);
  void croppingCoarseBody3D();
  void showingBodyGrayscale();
  void loadingSplitTask();

public slots:
  void updateBodyWidget(uint64_t bodyId);
  void goToPosition();
  void recoverSeed();
  void exportSeed();
  void importSeed();
  void selectSeed();
  void selectAllSeed();
  void setMainSeed();
  void cropCoarseBody3D();
  void showBodyGrayscale();
//  void updateUserBookmarkTable(ZStackDoc *doc);

private slots:
  void slotTest();
  void setSplit(uint64_t bodyId);
  void changeSplit();
  void commitResult();
//  void updateBookmarkTable(ZFlyEmBodySplitProject *project);
//  void clearBookmarkTable(ZFlyEmBodySplitProject *project);
//  void locateBookmark(const QModelIndex &index);
  void locateBookmark(const ZFlyEmBookmark *bookmark);
  void loadBookmark();
  void checkCurrentBookmark();
  void uncheckCurrentBookmark();
  void checkCurrentBookmark(bool checking);
  void saveTask();
  void loadSplitResult();
  void uploadSplitResult();
  void loadSplitTask();

private:
  void setupWidgetBehavior();
  void createMenu();

private:
  Ui::FlyEmSplitControlForm *ui;
//  ZFlyEmBookmarkListModel m_assignedBookmarkList;
//  ZFlyEmBookmarkListModel m_userBookmarkList;
  uint64_t m_currentBodyId;
  //ZFlyEmBodySplitProject m_project;
  QMenu *m_mainMenu;
};

#endif // FLYEMSPLITCONTROLFORM_H
