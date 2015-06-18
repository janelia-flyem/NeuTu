#ifndef ZFLYEMPROOFMVC_H
#define ZFLYEMPROOFMVC_H

#include <QString>
#include <QMetaType>
#include "zstackmvc.h"
#include "flyem/zflyembodysplitproject.h"
#include "flyem/zflyembodymergeproject.h"
#include "qthreadfuturemap.h"

class QWidget;
class ZFlyEmProofDoc;
class ZDvidTileEnsemble;
class ZDvidTarget;
class ZDvidDialog;
class ZFlyEmProofPresenter;

class ZFlyEmProofMvc : public ZStackMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofMvc(QWidget *parent = 0);

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

  void clear();

  void enableSplit();
  void disableSplit();

  void processViewChangeCustom(const ZStackViewParam &viewParam);

signals:
  void launchingSplit(const QString &message);
  void launchingSplit(uint64_t bodyId);
  void messageGenerated(const QString &message, bool appending = true);
  void errorGenerated(const QString &message, bool appending = true);
  void messageGenerated(const ZWidgetMessage &message);
  void splitBodyLoaded(uint64_t bodyId);
  void bookmarkUpdated(ZFlyEmBodySplitProject *m_project);
  void dvidTargetChanged(ZDvidTarget);

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

  void showBody3d();
  void showSplit3d();
  void showCoarseBody3d();

  void setDvidLabelSliceSize(int width, int height);

  void zoomTo(const ZIntPoint &pt);
  void zoomTo(int x, int y, int z);
  void zoomTo(int x, int y, int z, int width);

  void loadBookmark(const QString &filePath);
  void addSelectionAt(int x, int y, int z);
  void xorSelectionAt(int x, int y, int z);
  void deselectAllBody();
  void selectSeed();
  void selectAllSeed();
  void recoverSeed();
  void runSplit();
//  void toggleEdgeMode(bool edgeOn);

protected:
  void customInit();
  void createPresenter();

private:
  void launchSplitFunc(uint64_t bodyId);
  uint64_t getMappedBodyId(uint64_t bodyId);
  std::set<uint64_t> getCurrentSelectedBodyId(NeuTube::EBodyLabelType type) const;
  void runSplitFunc();

private:
  bool m_showSegmentation;
  bool m_splitOn;
  ZFlyEmBodySplitProject m_splitProject;
  ZFlyEmBodyMergeProject m_mergeProject;

  QThreadFutureMap m_futureMap;

  ZDvidDialog *m_dvidDlg;
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
  connect(panel, SIGNAL(coarseBodyViewTriggered()),
          this, SLOT(showCoarseBody3d()));
  connect(panel, SIGNAL(savingMerge()), this, SLOT(saveMergeOperation()));
  connect(panel, SIGNAL(committingMerge()), this, SLOT(commitMerge()));
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(locatingBody(uint64_t)),
          this, SLOT(locateBody(uint64_t)));
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
  connect(panel, SIGNAL(zoomingTo(int, int, int)),
          this, SLOT(zoomTo(int, int, int)));
  connect(panel, SIGNAL(selectingSeed()), this, SLOT(selectSeed()));
  connect(panel, SIGNAL(selectingAllSeed()), this, SLOT(selectAllSeed()));
  connect(panel, SIGNAL(recoveringSeed()), this, SLOT(recoverSeed()));
  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          panel, SLOT(updateBodyWidget(uint64_t)));
}


#endif // ZFLYEMPROOFMVC_H
