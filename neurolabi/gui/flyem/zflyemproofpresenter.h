#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include "zstackpresenter.h"
#include "dvid/zdvidsynapse.h"

class QKeyEvent;
class ZFlyEmBookmark;
class ZFlyEmProofDoc;

class ZFlyEmProofPresenter : public ZStackPresenter
{
  Q_OBJECT

protected:
//  explicit ZFlyEmProofPresenter(ZStackFrame *parent = 0);
  explicit ZFlyEmProofPresenter(QWidget *parent = 0);

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

  void enableSplit();
  void disableSplit();
  void setSplitEnabled(bool s);

  bool processKeyPressEvent(QKeyEvent *event);
  bool processCustomOperator(
      const ZStackOperator &op, ZInteractionEvent *e = NULL);

  inline bool isSplitWindow() const {
    return m_splitWindowMode;
  }

  void setSplitWindow(bool state) {
    m_splitWindowMode = state;
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

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody();
  void selectingBodyInRoi();
  void selectingBodyInRoi(bool appending);
  void runningSplit();
  void runningLocalSplit();
  void goingToBody();
  void selectingBody();
  void goingToTBar();
//  void bookmarkAdded(ZFlyEmBookmark*);
  void annotatingBookmark(ZFlyEmBookmark*);
  void annotatingSynapse();
  void mergingBody();
  void goingToBodyBottom();
  void goingToBodyTop();
  void togglingSegmentation();
  void togglingData();
  void highlightModeChanged();

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
  void removeTodoItem();
  void checkTodoItem();
  void uncheckTodoItem();
  void selectBodyInRoi();
  void zoomInRectRoi();

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
  void tryAddTodoItem(const ZIntPoint &pt);
  void tryAddDoneItem(const ZIntPoint &pt);
  bool updateActiveObjectForSynapseMove();
  bool updateActiveObjectForSynapseMove(const ZPoint &currentPos);
  void updateActiveObjectForSynapseAdd();
  void updateActiveObjectForSynapseAdd(const ZPoint &currentPos);

private:
  bool m_isHightlightMode;
  bool m_splitWindowMode;
  bool m_highTileContrast;
  bool m_smoothTransform;
  bool m_showingData;

  QMenu *m_synapseContextMenu;

  ZKeyOperationMap m_bookmarkKeyOperationMap;
};

#endif // ZFLYEMPROOFPRESENTER_H
