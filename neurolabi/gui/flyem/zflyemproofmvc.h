#ifndef ZFLYEMPROOFMVC_H
#define ZFLYEMPROOFMVC_H

#include <QString>
#include <QMetaType>
#include "zstackmvc.h"
#include "flyem/zflyembodysplitproject.h"
#include "flyem/zflyembodymergeproject.h"
#include "qthreadfuturemap.h"
#include "flyem/zflyembookmark.h"

class QWidget;
class ZFlyEmProofDoc;
class ZDvidTileEnsemble;
class ZDvidTarget;
class ZDvidDialog;
class ZFlyEmProofPresenter;
class ZFlyEmSupervisor;
class ZPaintLabelWidget;
class FlyEmBodyInfoDialog;

/*!
 * \brief The MVC class for flyem proofreading
 */
class ZFlyEmProofMvc : public ZStackMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofMvc(QWidget *parent = 0);
  ~ZFlyEmProofMvc();

  static ZFlyEmProofMvc* Make(
      QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc);
  static ZFlyEmProofMvc* Make(const ZDvidTarget &target);

  ZFlyEmProofDoc* getCompleteDocument() const;
  ZFlyEmProofPresenter* getCompletePresenter() const;

  template <typename T>
  void connectControlPanel(T *panel);

  template <typename T>
  void connectSplitControlPanel(T *panel);

  ZDvidTileEnsemble* getDvidTileEnsemble();

  void setDvidTarget(const ZDvidTarget &target);
  void setDvidTargetFromDialog();

  void clear();

  void exitCurrentDoc();

  void enableSplit();
  void disableSplit();

  void processViewChangeCustom(const ZStackViewParam &viewParam);

  ZFlyEmSupervisor* getSupervisor() const;

//  bool checkInBody(uint64_t bodyId);
  bool checkOutBody(uint64_t bodyId);

signals:
  void launchingSplit(const QString &message);
  void launchingSplit(uint64_t bodyId);
  void messageGenerated(const QString &message, bool appending = true);
  void errorGenerated(const QString &message, bool appending = true);
  void messageGenerated(const ZWidgetMessage &message);
  void splitBodyLoaded(uint64_t bodyId);
  void bookmarkUpdated(ZFlyEmBodyMergeProject *m_project);
  void bookmarkUpdated(ZFlyEmBodySplitProject *m_project);
  void bookmarkDeleted(ZFlyEmBodyMergeProject *m_project);
  void bookmarkDeleted(ZFlyEmBodySplitProject *m_project);
  void dvidTargetChanged(ZDvidTarget);
  void userBookmarkUpdated(ZStackDoc *doc);

public slots:
  void mergeSelected();
  void undo();
  void redo();

  void setSegmentationVisible(bool visible);
  void setDvidTarget();
  ZDvidTarget getDvidTarget() const;
  void launchSplit(uint64_t bodyId);
  void processMessageSlot(const QString &message);
  void notifySplitTriggered();
  void annotateBody();
  void checkInSelectedBody();
  void checkInSelectedBodyAdmin();
  void checkOutBody();
  bool checkInBody(uint64_t bodyId);
  bool checkInBodyWithMessage(uint64_t bodyId);
  void exitSplit();
  void switchSplitBody(uint64_t bodyId);
  void showBodyQuickView();
  void showSplitQuickView();
  void presentBodySplit(uint64_t bodyId);
  void updateBodySelection();
  void saveSeed();
  void saveMergeOperation();
  void commitMerge();
  void commitCurrentSplit();
  void locateBody(uint64_t bodyId);
  void selectBody(uint64_t bodyId);

  void showBody3d();
  void showSplit3d();
  void showCoarseBody3d();
  void showFineBody3d();

  void setDvidLabelSliceSize(int width, int height);
  void showFullSegmentation();

  void enhanceTileContrast(bool state);

  void zoomTo(const ZIntPoint &pt);
  void zoomTo(int x, int y, int z);
  void zoomTo(int x, int y, int z, int width);
  void goToBody();
  void selectBody();
  void processLabelSliceSelectionChange();

  void loadBookmark(const QString &filePath);
  void addSelectionAt(int x, int y, int z);
  void xorSelectionAt(int x, int y, int z);
  void deselectAllBody();
  void selectSeed();
  void selectAllSeed();
  void recoverSeed();
  void exportSeed();
  void importSeed();
  void runSplit();

  void loadSynapse();
  void showSynapseAnnotation(bool visible);
  void showBookmark(bool visible);
  void showSegmentation(bool visible);

  void loadBookmark();
  void openSequencer();

  void recordCheckedBookmark(const QString &key, bool checking);
  void recordBookmark(ZFlyEmBookmark *bookmark);
  void processSelectionChange(const ZStackObjectSelector &selector);

  void annotateBookmark(ZFlyEmBookmark *bookmark);

  void updateUserBookmarkTable();

  void processCheckedUserBookmark(ZFlyEmBookmark *bookmark);

//  void toggleEdgeMode(bool edgeOn);

protected:
  void customInit();
  void createPresenter();

private:
  void launchSplitFunc(uint64_t bodyId);
  uint64_t getMappedBodyId(uint64_t bodyId);
  std::set<uint64_t> getCurrentSelectedBodyId(NeuTube::EBodyLabelType type) const;
  void runSplitFunc();
  void notifyBookmarkUpdated();
  void notifyBookmarkDeleted();

  void syncDvidBookmark();
  void loadBookmarkFunc(const QString &filePath);

private:
  bool m_showSegmentation;
  ZFlyEmBodySplitProject m_splitProject;
  ZFlyEmBodyMergeProject m_mergeProject;
//  ZFlyEmBookmarkArray m_bookmarkArray;

  QThreadFutureMap m_futureMap;

  ZPaintLabelWidget *m_paintLabelWidget;

  ZDvidDialog *m_dvidDlg;
  FlyEmBodyInfoDialog *m_bodyInfoDlg;
  ZFlyEmSupervisor *m_supervisor;
};

template <typename T>
void ZFlyEmProofMvc::connectControlPanel(T *panel)
{
  connect(panel, SIGNAL(segmentVisibleChanged(bool)),
          this, SLOT(setSegmentationVisible(bool)));
  connect(panel, SIGNAL(mergingSelected()), this, SLOT(mergeSelected()));
  connect(panel, SIGNAL(edgeModeToggled(bool)),
          this, SLOT(toggleEdgeMode(bool)));
  connect(panel, SIGNAL(dvidSetTriggered()), this, SLOT(setDvidTarget()));
  connect(this, SIGNAL(dvidTargetChanged(ZDvidTarget)),
          panel, SLOT(setDvidInfo(ZDvidTarget)));
  connect(this, SIGNAL(launchingSplit(uint64_t)),
          panel, SIGNAL(splitTriggered(uint64_t)));
  connect(panel, SIGNAL(labelSizeChanged(int, int)),
          this, SLOT(setDvidLabelSliceSize(int, int)));
  connect(panel, SIGNAL(showingFullSegmentation()),
          this, SLOT(showFullSegmentation()));
  connect(panel, SIGNAL(coarseBodyViewTriggered()),
          this, SLOT(showCoarseBody3d()));
  connect(panel, SIGNAL(bodyViewTriggered()),
          this, SLOT(showFineBody3d()));
  connect(panel, SIGNAL(savingMerge()), this, SLOT(saveMergeOperation()));
  connect(panel, SIGNAL(committingMerge()), this, SLOT(commitMerge()));
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(locatingBody(uint64_t)),
          this, SLOT(locateBody(uint64_t)));
  connect(panel, SIGNAL(goingToBody()), this, SLOT(goToBody()));
  connect(panel, SIGNAL(selectingBody()), this, SLOT(selectBody()));
  connect(this, SIGNAL(bookmarkUpdated(ZFlyEmBodyMergeProject*)),
          panel, SLOT(updateBookmarkTable(ZFlyEmBodyMergeProject*)));
  connect(this, SIGNAL(bookmarkDeleted(ZFlyEmBodyMergeProject*)),
          panel, SLOT(clearBookmarkTable(ZFlyEmBodyMergeProject*)));
  connect(panel, SIGNAL(bookmarkChecked(QString, bool)),
          this, SLOT(recordCheckedBookmark(QString, bool)));
  connect(panel, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
          this, SLOT(recordBookmark(ZFlyEmBookmark*)));
  connect(this, SIGNAL(userBookmarkUpdated(ZStackDoc*)),
          panel, SLOT(updateUserBookmarkTable(ZStackDoc*)));
  connect(panel, SIGNAL(userBookmarkChecked(ZFlyEmBookmark*)),
          this, SLOT(processCheckedUserBookmark(ZFlyEmBookmark*)));
}

template <typename T>
void ZFlyEmProofMvc::connectSplitControlPanel(T *panel)
{
  connect(panel, SIGNAL(quickViewTriggered()), this, SLOT(showBodyQuickView()));
  connect(panel, SIGNAL(splitQuickViewTriggered()),
          this, SLOT(showSplitQuickView()));
  connect(panel, SIGNAL(bodyViewTriggered()), this, SLOT(showBody3d()));
  connect(panel, SIGNAL(splitViewTriggered()), this, SLOT(showSplit3d()));

  connect(panel, SIGNAL(exitingSplit()), this, SLOT(exitSplit()));
  connect(panel, SIGNAL(changingSplit(uint64_t)), this,
          SLOT(switchSplitBody(uint64_t)));
  connect(panel, SIGNAL(savingSeed()), this, SLOT(saveSeed()));
  connect(panel, SIGNAL(committingResult()), this, SLOT(commitCurrentSplit()));
  connect(panel, SIGNAL(loadingBookmark(QString)),
          this, SLOT(loadBookmark(QString)));
  connect(this, SIGNAL(bookmarkUpdated(ZFlyEmBodySplitProject*)),
          panel, SLOT(updateBookmarkTable(ZFlyEmBodySplitProject*)));
  connect(this, SIGNAL(bookmarkDeleted(ZFlyEmBodySplitProject*)),
          panel, SLOT(clearBookmarkTable(ZFlyEmBodySplitProject*)));
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(selectingSeed()), this, SLOT(selectSeed()));
  connect(panel, SIGNAL(selectingAllSeed()), this, SLOT(selectAllSeed()));
  connect(panel, SIGNAL(recoveringSeed()), this, SLOT(recoverSeed()));
  connect(panel, SIGNAL(exportingSeed()), this, SLOT(exportSeed()));
  connect(panel, SIGNAL(importingSeed()), this, SLOT(importSeed()));
  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          panel, SLOT(updateBodyWidget(uint64_t)));
  connect(panel, SIGNAL(loadingSynapse()), this, SLOT(loadSynapse()));
  connect(panel, SIGNAL(bookmarkChecked(QString, bool)),
          this, SLOT(recordCheckedBookmark(QString, bool)));
  connect(panel, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
          this, SLOT(recordBookmark(ZFlyEmBookmark*)));
}


#endif // ZFLYEMPROOFMVC_H
