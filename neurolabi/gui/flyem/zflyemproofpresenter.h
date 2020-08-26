#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include <memory>
#include <functional>

#include "mvc/zstackpresenter.h"
#include "dvid/zdvidsynapse.h"

class QKeyEvent;
class ZFlyEmBookmark;
class ZFlyEmProofDoc;
class ZFlyEmToDoDelegate;

class ZFlyEmProofPresenter : public ZStackPresenter
{
  Q_OBJECT

protected:
//  explicit ZFlyEmProofPresenter(ZStackFrame *parent = 0);
  explicit ZFlyEmProofPresenter(QWidget *parent = 0);
  ~ZFlyEmProofPresenter();

public:
  static ZFlyEmProofPresenter* Make(QWidget *parent);

  bool customKeyProcess(QKeyEvent *event) override;

  void toggleHighlightMode();
  bool isHighlight() const;
  void setHighlightMode(bool hl);
  bool isSplitOn() const;
  bool highTileContrast() const;
  bool smoothTransform() const;

  void setHighTileContrast(bool high);
  void setSmoothTransform(bool on);
  void showData(bool on);
  bool showingData() const;

  void enableSplit(neutu::EBodySplitMode mode);
  void disableSplit();
  void setSplitEnabled(bool s);

  bool processKeyPressEvent(QKeyEvent *event) override;
  bool processCustomOperator(
      const ZStackOperator &op, ZInteractionEvent *e = NULL) override;

  neutu::EBodySplitMode getSplitMode() const {
    return m_splitMode;
  }

  inline bool isSplitWindow() const {
    return m_splitMode != neutu::EBodySplitMode::NONE;
//    return m_splitWindowMode;
  }

  void setSplitMode(neutu::EBodySplitMode mode) {
    m_splitMode = mode;
  }

  void processRectRoiUpdate(ZRect2d *rect, bool appending) override;

  ZKeyOperationConfig* getKeyConfig() override;
  void configKeyMap() override;

//  void createBodyContextMenu();

  ZStackDocMenuFactory* getMenuFactory() override;

  ZFlyEmProofDoc* getCompleteDocument() const;

  void createSynapseContextMenu();
  QMenu* getSynapseContextMenu();

  QMenu* getContextMenu() override;

//  QAction* makeAction(ZActionFactory::EAction item);
  bool connectAction(QAction *action, ZActionFactory::EAction item) override;

  void setTodoDelegate(std::unique_ptr<ZFlyEmToDoDelegate> &&delegate);

  bool allowingBlinkingSegmentation() const;
  bool allowingBodySplit() const;

  void setBodyHittable(bool on);
//  void setLabelAlpha(int alpha) {
//    m_labelAlpha = alpha;
//  }

//  int getLabelAlpha() const {
//    return m_labelAlpha;
//  }

public:
  std::function<int()> getSegmentationAlpha = []() {
    return 75;
  };

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody(bool asking);
  void selectingBodyInRoi();
  void selectingBodyInRoi(bool appending);
  void runningSplit();
  void runningFullSplit();
  void runningLocalSplit();
  void goingToBody();
  void selectingBody();
  void goingToPosition();
  void goingToTBar();
//  void bookmarkAdded(ZFlyEmBookmark*);
  void annotatingBookmark(ZFlyEmBookmark*);
  void annotatingSynapse();
  void annotatingTodo();
  void mergingBody();
  void uploadingMerge();
  void goingToBodyBottom();
  void goingToBodyTop();
  void togglingSegmentation();
  void togglingData();
  void highlightModeChanged();
  void showingSupervoxelList();
  void togglingBodyColorMap();
  void refreshingData();
  void tipDetectRequested(ZIntPoint point, uint64_t bodyID);

public slots:
  void deleteSelectedSynapse();
  void verifySelectedSynapse();
  void unverifySelectedSynapse();
  void linkSelectedSynapse();
  void unlinkSelectedSynapse();
  void repairSelectedSynapse();
  void highlightPsd(bool on);
  void tryAddSynapseMode(ZDvidSynapse::EKind kind);
  void tryAddPreSynapseMode();
  void tryAddPostSynapseMode();
  void tryMoveSynapseMode();
  void tryAddTodoItem();
  void tryAddDoneItem();
  void tryAddToSplitItem();
  void tryAddToSupervoxelSplitItem();
  void tryAddToMergeItem();
  void tryAddTraceToSomaItem();
  void tryAddNoSomaItem();
  void tryAddDiagnosticItem();
  void tryAddSegmentationDiagnosticItem();
  void removeTodoItem();
  void checkTodoItem();
  void uncheckTodoItem();
  void setTodoItemToNormal();
  void setTodoItemIrrelevant();
  void setTodoItemToMerge();
  void setTodoItemToSplit();
  void setTodoItemToTraceToSoma();
  void setTodoItemToNoSoma();
  void selectBodyInRoi();
  void zoomInRectRoi();
  void refreshSegmentation();
  void refreshData();
  void trace();

  void tryAddTodoItem(const ZIntPoint &pt);
  void tryAddDoneItem(const ZIntPoint &pt);
  void tryAddToMergeItem(const ZIntPoint &pt);
  void tryAddToSplitItem(const ZIntPoint &pt);
  void tryAddToSupervoxelSplitItem(const ZIntPoint &pt);
  void tryAddTraceToSomaItem(const ZIntPoint &pt);
  void tryAddNoSomaItem(const ZIntPoint &pt);
  void tryAddDiagnosticItem(const ZIntPoint &pt);
  void tryAddSegmentationDiagnosticItem(const ZIntPoint &pt);
  void runTipDetection();

  void setCutPlaneAlongX();
  void setCutPlaneAlongY();
  void setCutPlaneAlongZ();
  //for testing
  void setCutPlaneArb();

  void setBodyColor();
  void resetBodyColor();

  void showSupervoxelList();

  void allowBlinkingSegmentation(bool on);
  void toggleSupervoxelView(bool on);
  void takeScreenshot();

  void updateActions();

protected:
  virtual void tryAddTodoItem(
      int x, int y, int z, bool checked, neutu::EToDoAction action,
      uint64_t bodyId);
  void tryAddTodoItem(
      const ZIntPoint &pt, bool checked, neutu::EToDoAction action,
      uint64_t bodyId);
  void tryAddTodoItem(
      const ZIntPoint &pt, bool checked, neutu::EToDoAction action);

  void copyLink(const QString &option) const override;

private:
//  void connectAction();
  void tryAddBookmarkMode();
//  void tryAddBookmarkMode(double x, double y);
//  void tryAddTodoItemMode(double x, double y);
  void addActiveDecorationAsBookmark();
  void init();
  void tryAddSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind,
                     bool tryingLink);
  void tryAddSynapse(const ZIntPoint &pt, bool tryingLink);
  void tryMoveSynapse(const ZIntPoint &pt);
//  void tryTodoItemMode();
//  bool updateActiveObjectForSynapseMove();
//  bool updateActiveObjectForSynapseMove(const ZPoint &currentPos);
//  void updateActiveObjectForSynapseAdd();
//  void updateActiveObjectForSynapseAdd(const ZPoint &currentDataPos);

  ZPoint getLastMouseReleasePosition(Qt::MouseButtons buttons) const;

private:
  bool m_isHightlightMode;
  neutu::EBodySplitMode m_splitMode;
//  bool m_splitWindowMode;
  bool m_highTileContrast;
  bool m_smoothTransform;
  bool m_showingData;
  bool m_blinkingSegmenationAllowed = true;
  bool m_isBodyHittable = true;

  std::unique_ptr<ZFlyEmToDoDelegate> m_todoDelegate;

  QMenu *m_synapseContextMenu;

  ZKeyOperationMap m_bookmarkKeyOperationMap;
};

#endif // ZFLYEMPROOFPRESENTER_H
