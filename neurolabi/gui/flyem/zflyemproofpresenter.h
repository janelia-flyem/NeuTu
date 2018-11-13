#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include <memory>

#include "zstackpresenter.h"
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

  bool customKeyProcess(QKeyEvent *event);

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

  void enableSplit(flyem::EBodySplitMode mode);
  void disableSplit();
  void setSplitEnabled(bool s);

  bool processKeyPressEvent(QKeyEvent *event);
  bool processCustomOperator(
      const ZStackOperator &op, ZInteractionEvent *e = NULL);

  flyem::EBodySplitMode getSplitMode() const {
    return m_splitMode;
  }

  inline bool isSplitWindow() const {
    return m_splitMode != flyem::EBodySplitMode::NONE;
//    return m_splitWindowMode;
  }

  void setSplitMode(flyem::EBodySplitMode mode) {
    m_splitMode = mode;
  }

  void processRectRoiUpdate(ZRect2d *rect, bool appending);

  ZKeyOperationConfig* getKeyConfig();
  void configKeyMap();

//  void createBodyContextMenu();

  ZStackDocMenuFactory* getMenuFactory();

  ZFlyEmProofDoc* getCompleteDocument() const;

  void createSynapseContextMenu();
  QMenu* getSynapseContextMenu();

  QMenu* getContextMenu();

//  QAction* makeAction(ZActionFactory::EAction item);
  bool connectAction(QAction *action, ZActionFactory::EAction item);

  void setTodoDelegate(std::unique_ptr<ZFlyEmToDoDelegate> &&delegate);

  bool allowingBlinkingSegmentation() const;
//  void setLabelAlpha(int alpha) {
//    m_labelAlpha = alpha;
//  }

//  int getLabelAlpha() const {
//    return m_labelAlpha;
//  }

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
  void mergingBody();
  void uploadingMerge();
  void goingToBodyBottom();
  void goingToBodyTop();
  void togglingSegmentation();
  void togglingData();
  void highlightModeChanged();
  void showingSupervoxelList();

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
  void removeTodoItem();
  void checkTodoItem();
  void uncheckTodoItem();
  void setTodoItemToNormal();
  void setTodoItemIrrelevant();
  void setTodoItemToMerge();
  void setTodoItemToSplit();
  void selectBodyInRoi();
  void zoomInRectRoi();
  void refreshSegmentation();

  void tryAddTodoItem(const ZIntPoint &pt);
  void tryAddDoneItem(const ZIntPoint &pt);
  void tryAddToMergeItem(const ZIntPoint &pt);
  void tryAddToSplitItem(const ZIntPoint &pt);
  void tryAddToSupervoxelSplitItem(const ZIntPoint &pt);

  void showSupervoxelList();

  void allowBlinkingSegmentation(bool on);

protected:
  virtual void tryAddTodoItem(
      int x, int y, int z, bool checked, neutube::EToDoAction action,
      uint64_t bodyId);
  void tryAddTodoItem(
      const ZIntPoint &pt, bool checked, neutube::EToDoAction action,
      uint64_t bodyId);
  void tryAddTodoItem(
      const ZIntPoint &pt, bool checked, neutube::EToDoAction action);

private:
//  void connectAction();
  void tryAddBookmarkMode();
  void tryAddBookmarkMode(double x, double y);
  void tryAddTodoItemMode(double x, double y);
  void addActiveStrokeAsBookmark();
  void init();
  void tryAddSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind,
                     bool tryingLink);
  void tryAddSynapse(const ZIntPoint &pt, bool tryingLink);
  void tryMoveSynapse(const ZIntPoint &pt);
  void tryTodoItemMode();
  bool updateActiveObjectForSynapseMove();
  bool updateActiveObjectForSynapseMove(const ZPoint &currentPos);
  void updateActiveObjectForSynapseAdd();
  void updateActiveObjectForSynapseAdd(const ZPoint &currentPos);

private:
  bool m_isHightlightMode;
  flyem::EBodySplitMode m_splitMode;
//  bool m_splitWindowMode;
  bool m_highTileContrast;
  bool m_smoothTransform;
  bool m_showingData;
  bool m_blinkingSegmenationAllowed = true;

  std::unique_ptr<ZFlyEmToDoDelegate> m_todoDelegate;

  QMenu *m_synapseContextMenu;

  ZKeyOperationMap m_bookmarkKeyOperationMap;
};

#endif // ZFLYEMPROOFPRESENTER_H
